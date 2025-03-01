/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "LNS.h"

#include <chrono>
#include <numeric>
#include <random>

#include "SIPP.h"

using dsec = std::chrono::duration<double>;

LNS::LNS(const Instance& instance_, const bool is_visualisation_) : Solver("LNS", instance_), is_visualisation(is_visualisation_)
{
  // SIPP's lifetime can not be longer than LNS's lifetime, because then map_data might be deleted before SIPP
  planner = std::make_unique<SIPP>(instance, is_visualisation);
}

void LNS::solve()
{
  // start measuring time
  std::chrono::high_resolution_clock::time_point start_find_initial_solution = std::chrono::high_resolution_clock::now();

  // calculate initial solution
  bool found = find_initial_solution();

  double initial_solution_time = ((dsec)(std::chrono::high_resolution_clock::now() - start_find_initial_solution)).count();
  std::cout << "Initial solution found in " << initial_solution_time << " second." << std::endl;
  // check whether solution valid

  // improve the solution
}

auto LNS::find_initial_solution() -> bool
{
  // create initial solution
  solution = PrioritizedPlanning();
  return solution.feasible;
}

auto LNS::PrioritizedPlanning() -> Solution
{
  Solution sol;
  sol.paths.resize(instance.num_of_agents, Path());
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
    TimePointPath tp_path = planner->plan(agent_id);

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

    // update the constraint table
    planner->constraint_table.add_constraints(tp_path);

    // convert to Path and add to the solution TODO not needed
    sol.paths[agent_id] = timepointpath_to_path(tp_path);
  }
  return sol;
}

auto LNS::get_num_of_generated_nodes() const -> int
{
  return planner->generated_all_iter;
}
