/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "LNS.h"

#include <numeric>
#include <random>

LNS::LNS(std::shared_ptr<Instance> instance) : Solver("LNS", instance)
{
  // SIPP's lifetime can not be longer than LNS's lifetime, because then map_data might be deleted before SIPP
  planner = std::make_unique<SIPP>(instance->get_map_data());
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
  const int agent_num = instance->num_of_agents;

  // create a sequence from 0 to agent_num
  std::vector<int> priorities(agent_num);
  std::iota(priorities.begin(), priorities.end(), 1);

  // shuffle the sequence to get random priorities
  std::random_shuffle(priorities.begin(), priorities.end());

  // initialize path table

  // plan path for each agent according to its priority
  for (int i = 0; i < agent_num; i++)
  {
    // get agent with priority i
    int agent_id = priorities[i];

    // plan path for agent
  }
}
