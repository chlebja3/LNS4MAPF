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


LNS::LNS(const Instance& instance_, bool is_visualisation_, double w_)
    : Solver("LNS", instance_),
      is_visualisation(is_visualisation_),
      w(w_),
      destroy_operator(Operator([this](Solution& sol) { destroy_random(sol); })),
      repair_operator(Operator([this](Solution& sol) { repair_default(sol); }))
{
  // SIPP's lifetime can not be longer than LNS's lifetime, because then map_data might be deleted before SIPP
  planner = std::make_unique<SIPP>(instance, is_visualisation);
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
  while (iteration_num < max_iter && curr_time < time_limit)
  {
    iteration_num++;

    // copy solution
    Solution perturbed_sol = solution;

    // destroy operator
    destroy_operator.apply(perturbed_sol);

    // repair operator
    repair_operator.apply(perturbed_sol);

    // accept only feasible solutions
    if (!perturbed_sol.feasible)
    {
      continue;
    }

    // accept only improving solutions
    perturbed_sol.calculate_cost(instance);
    if (perturbed_sol.sum_of_delays >= solution.sum_of_delays)
    {
      continue;
    }

    // remember the perturbed solution
    solution = std::move(perturbed_sol);
  }
}

auto LNS::find_initial_solution() -> bool
{
  // create initial solution
  solution = PrioritizedPlanning();
  if (solution.feasible)
  {
    solution.calculate_cost(instance);
  }
  std::cout << "Solution has sum of costs: " << solution.sum_of_costs << ", sol has delay: " << solution.sum_of_delays << std::endl;
  return solution.feasible;
}

auto LNS::PrioritizedPlanning() -> Solution
{
  Solution sol;
  sol.paths.resize(instance.num_of_agents, TimePointPath());
  const int agent_num = instance.num_of_agents;

  // create a sequence from 0 to agent_num
  std::vector<int> priorities(agent_num);
  std::iota(priorities.begin(), priorities.end(), 0);

  // shuffle the sequence to get random priorities
  std::random_shuffle(priorities.begin(), priorities.end());

  // initialize iter info
  if (is_visualisation)
  {
    sipp_info.clear();
    sipp_info.resize(agent_num);
  }

  // plan path for each agent according to its priority
  for (int i = 0; i < agent_num; i++)
  {
    // get agent with priority i
    int agent_id = priorities[i];
    // std::cout << "planning agent: " << agent_id << std::endl;

    // plan path for agent
    TimePointPath tp_path;
    if (w == 1.0)
    {
      // optimal sipp
      tp_path = planner->plan(agent_id);
      // tp_path = planner->plan_mapflns_heuristic(agent_id);
    }
    else if (w > 1.0)
    {
      // suboptimal sipp
      tp_path = planner->plan_suboptimal(agent_id, w);
    }
    else
    {
      throw std::runtime_error("SIPP suboptimality factor must be >= 1");
    }

    // get the visualization info
    if (is_visualisation)
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

auto LNS::get_num_of_generated_nodes() const -> int
{
  return planner->generated_all_iter;
}

auto LNS::get_num_of_expanded_nodes() const -> int
{
  return planner->expanded_all_iter;
}

void LNS::destroy_random(Solution& sol) const
{
  // TODO optimize
  constexpr int    neighborhood_size = 4;
  std::vector<int> path_idcs(sol.paths.size());
  std::iota(path_idcs.begin(), path_idcs.end(), 0);
  std::random_shuffle(path_idcs.begin(), path_idcs.end());
  assertm((int)path_idcs.size() > neighborhood_size, "Not enough paths for the destroy operator.");
  sol.destroyed_paths =
      std::vector(std::make_move_iterator(path_idcs.begin()), std::make_move_iterator(path_idcs.begin() + neighborhood_size));
  sol.feasible = false;
}

void LNS::repair_default(Solution& sol) const
{
  assertm(!sol.feasible, "Can not repair a feasible solution");
  assertm((int)sol.destroyed_paths.size() > 0, "There are no destroyed paths in the solution to be repaired.");
  // remove the destroyed paths
  for (auto& it : sol.destroyed_paths)
  {
    planner->constraint_table.remove_constraints(sol.paths[it]);
    sol.paths[it].clear();
  }

  // replan
  for (auto& it : sol.destroyed_paths)
  {
    TimePointPath tp_path = planner->plan(it);
    planner->constraint_table.add_constraints(tp_path);
    sol.paths[it] = tp_path;
  }
}
