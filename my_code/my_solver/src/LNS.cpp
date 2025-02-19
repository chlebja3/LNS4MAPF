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

LNS::LNS(const Instance& instance) : Solver("LNS", instance)
{
  // SIPP's lifetime can not be longer than LNS's lifetime, because then map_data might be deleted before SIPP
  planner = std::make_unique<SIPP>(instance);
};

void LNS::solve()
{
  // calculate initial solution
  find_initial_solution();

  // improve the solution
}

void LNS::find_initial_solution()
{
  // create initial solution
  solution = PrioritizedPlanning();
}

Solution LNS::PrioritizedPlanning()
{
  Solution sol;
  sol.paths.resize(instance.num_of_agents, Path());
  const int agent_num = instance.num_of_agents;

  // create a sequence from 0 to agent_num
  std::vector<int> priorities(agent_num);
  std::iota(priorities.begin(), priorities.end(), 0);

  /*// print priorities
  std::cout << "Priorities: ";
  for (int i = 0; i < agent_num; i++)
  {
    std::cout << priorities[i] << " ";
  }
  */

  // shuffle the sequence to get random priorities
  std::random_shuffle(priorities.begin(), priorities.end());

  // plan path for each agent according to its priority
  for (int i = 0; i < agent_num; i++)
  {
    // get agent with priority i
    int agent_id = priorities[i];
    // std::cout << "planning agent: " << agent_id << std::endl;

    // plan path for agent
    TimePointPath tp_path = planner->plan(agent_id);

    // print the path
    // std::cout << "Path for agent " << agent_id << ": " << std::endl;
    // for (const auto& tp : tp_path)
    // {
    //   std::cout << "{" << tp.position << ", " << tp.interval << "}"
    //             << " ";
    // }
    // std::cout << std::endl;

    // check validity
    if (tp_path.empty())
    {
      std::cout << "Unable to find initial solution by prioritized planning. " << std::endl;
      return sol;
    }

    // update the constraint table
    planner->constraint_table.add_constraints(tp_path);

    // convert to Path and add to the solution
    sol.paths[agent_id] = timepointpath_to_path(tp_path);
  }
  return sol;
}

int LNS::get_num_of_generated_nodes(void) const
{
  return planner->generated_nodes;
}
