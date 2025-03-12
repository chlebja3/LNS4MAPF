/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "LNS.h"

#include <numeric>
#include <random>

#include "SIPP.h"
#include "utils.h"


LNS::LNS(const Instance& instance_, std::mt19937& rnd_generator_, SharedData* shared_data_, const LNS_settings& settings_)
    : Solver("LNS", instance_, rnd_generator_),
      shared_data(shared_data_),
      settings(settings_),
      destroy_operator(Operator([this](Solution& sol) { destroy_random(sol); })),
      repair_operator(Operator([this](Solution& sol) { repair_default(sol); }))
{
  // SIPP's lifetime can not be longer than LNS's lifetime, because then map_data might be deleted before SIPP
  planner = std::make_unique<SIPP>(instance, rnd_generator, settings.sipp_settings);
}

void LNS::solve()
{
  // start measuring time
  Time::time_point start_find_initial_solution = Time::now();

  // calculate initial solution
  bool found = find_initial_solution();
  assertm(found, "Could not find the initial solution");
  assertm(solution.is_valid(instance), "Found invalid solution");

  initial_solution_time = ((dsec)(Time::now() - start_find_initial_solution)).count();
  std::cout << "Initial solution found in " << initial_solution_time << " second." << std::endl;

  curr_time = initial_solution_time;

  // improve the solution
  while (iteration_num < settings.max_iter && curr_time < settings.time_limit)
  {
    iteration_num++;

    // copy solution
    Solution perturbed_sol = solution;

    // destroy operator
    destroy_operator.apply(perturbed_sol);

    // repair operator
    repair_operator.apply(perturbed_sol);


    // accept only feasible solutions
    bool accepted    = false;
    int  improvement = 0;
    if (!perturbed_sol.feasible)
    {
      discard_solution(perturbed_sol, solution);
      std::cout << "Discarded infeasible solution." << std::endl;
    }
    else
    {
      // accept only improving solutions
      perturbed_sol.calculate_cost(instance);
      improvement = solution.sum_of_delays - perturbed_sol.sum_of_delays;

      if (improvement <= 0)
      {
        discard_solution(perturbed_sol, solution);
        // std::cout << "Discarded worse solution, difference: " << perturbed_sol.sum_of_costs - solution.sum_of_costs << std::endl;
      }
      else
      {
        accepted = true;
        // remember the perturbed solution
        std::cout << "Solution improved by: " << solution.sum_of_delays - perturbed_sol.sum_of_delays << std::endl;
        solution = std::move(perturbed_sol);
      }
    }

    // construct LNS iteration info and add to the shared data
    if (settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      // get the pointer to the original perturbed solution (which might have been moved)
      Solution* sol = &solution;
      if (!accepted)
      {
        sol = &perturbed_sol;
      }
      LNSIterationInfo lns_info(iteration_num, accepted, improvement, sipp_info, *sol);
      sipp_info.clear();
      shared_data->update_lns_info(lns_info);
    }
  }
}

auto LNS::find_initial_solution() -> bool
{
  // create initial solution
  solution = PrioritizedPlanning(SIPP_implementation::SIPP_mine);
  if (solution.feasible)
  {
    solution.calculate_cost(instance);
  }

  // construct LNS iteration info and add to the shared data
  if (settings.sipp_settings.info_type == INFO_type::visualisation)
  {
    LNSIterationInfo lns_info(0, solution.feasible, 0, sipp_info, solution);
    sipp_info.clear();
    shared_data->update_lns_info(lns_info);
  }

  std::cout << "Solution has sum of costs: " << solution.sum_of_costs << ", sol has delay: " << solution.sum_of_delays << std::endl;
  return solution.feasible;
}

auto LNS::PrioritizedPlanning(SIPP_implementation sipp_implementation) -> Solution
{
  Solution sol;
  sol.paths.resize(instance.num_of_agents, TimePointPath());
  const int agent_num = instance.num_of_agents;

  // create a sequence from 0 to agent_num
  std::vector<int> priorities(agent_num);
  std::iota(priorities.begin(), priorities.end(), 0);

  // shuffle the sequence to get random priorities
  std::shuffle(priorities.begin(), priorities.end(), rnd_generator);

  // initialize iter info
  if (settings.sipp_settings.info_type == INFO_type::visualisation)
  {
    sipp_info.clear();
    sipp_info.resize(agent_num);
  }

  // plan path for each agent according to its priority
  for (int i = 0; i < agent_num; i++)
  {
    // get agent with priority i
    int agent_id = priorities[i];

    // plan path for agent
    TimePointPath tp_path;
    if (sipp_implementation == SIPP_implementation::SIPP_suboptimal)
    {
      assertm(settings.sipp_settings.w >= 1.0, "SIPP suboptimality factor must be 1.0 or more.");
      // suboptimal sipp
      tp_path = planner->plan_suboptimal(agent_id, settings.sipp_settings.w);
    }
    else
    {
      assertm(settings.sipp_settings.w == 1.0, "Suboptimality factor must be 1 for optimal algorithms.");
      // optimal sipp
      if (sipp_implementation == SIPP_implementation::SIPP_mine)
      {
        tp_path = planner->plan(agent_id);
      }
      else if (sipp_implementation == SIPP_implementation::SIPP_mapf_lns)
      {
        tp_path = planner->plan_mapflns_heuristic(agent_id);
      }
      else
      {
        throw std::runtime_error("Not implemented yet");
      }
    }

    // get the visualization info
    if (settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      sipp_info[agent_id] = std::move(planner->iter_info);
      planner->iter_info.clear();
    }

    // check validity
    if (tp_path.empty())
    {
      std::cout << "Unable to find initial solution by prioritized planning. " << std::endl;
      sol.feasible = false;
      return sol;
    }

    // check validity
    assertm(instance.check_timepointpath_validity(tp_path), "SIPP planned an invalid timepointpath.");

    // update the constraint table
    planner->constraint_table.add_constraints(tp_path);

    // add to the solution
    sol.paths[agent_id] = tp_path;
  }
  return sol;
}

void LNS::destroy_random(Solution& sol) const
{
  // TODO optimize
  constexpr int    neighborhood_size = 10;
  std::vector<int> path_idcs(sol.paths.size());
  std::iota(path_idcs.begin(), path_idcs.end(), 0);
  std::shuffle(path_idcs.begin(), path_idcs.end(), rnd_generator);
  assertm((int)path_idcs.size() > neighborhood_size, "Not enough paths for the destroy operator.");
  sol.destroyed_paths =
      std::vector(std::make_move_iterator(path_idcs.begin()), std::make_move_iterator(path_idcs.begin() + neighborhood_size));
  assertm(sol.destroyed_paths.size() > 0, "No paths were destroyed.");
  sol.feasible = false;
}

void LNS::repair_default(Solution& sol) const
{
  assertm(!sol.feasible, "Can not repair a feasible solution");
  assertm((int)sol.destroyed_paths.size() > 0, "There are no destroyed paths in the solution to be repaired.");

  if (settings.sipp_settings.info_type == INFO_type::visualisation)
  {
    sipp_info.clear();
    sipp_info.resize(sol.destroyed_paths.size());
  }

  // remove the destroyed paths
  for (auto& it : sol.destroyed_paths)
  {
    // std::cout << "Removing path: " << it << std::endl;
    planner->constraint_table.remove_constraints(sol.paths[it]);
    sol.paths[it].clear();
  }
  sol.feasible = true;

  // replan
  for (int i = 0; i < static_cast<int>(sol.destroyed_paths.size()); i++)
  {
    int           path_idx = sol.destroyed_paths[i];
    TimePointPath tp_path  = planner->plan(path_idx);
    if (tp_path.empty())
    {
      sol.feasible = false;
      return;
    }

    // get the visualization info
    if (settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      sipp_info[i] = std::move(planner->iter_info);
      planner->iter_info.clear();
    }

    planner->constraint_table.add_constraints(tp_path);
    sol.paths[path_idx] = tp_path;
  }
}

void LNS::discard_solution(const Solution& sol, const Solution& prev_sol) const
{
  for (const auto& it : sol.destroyed_paths)
  {
    // remove the destroyed paths
    if (!sol.paths[it].empty())
    {
      planner->constraint_table.remove_constraints(sol.paths[it]);
    }
  }

  // restore the old paths
  for (const auto& it : sol.destroyed_paths)
  {
    planner->constraint_table.add_constraints(prev_sol.paths[it]);
  }
}
