/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "Solver.h"

bool Solution::is_valid(std::shared_ptr<Instance> instance) const
{
  // check that each agent has a path
  if (instance->num_of_agents != (int)paths.size()) return false;

  // check that each path starts and ends in the correct position and find the longest path
  int T_max = 0;
  for (int i = 0; i < (int)paths.size(); i++)
  {
    // remember the length of the longest path
    if ((int)paths[i].size() > T_max) T_max = paths[i].size();

    // check that the path starts in the start position
    if (!(paths[i].front() == instance->get_start_positions()[i])) return false;

    // check that the path ends in the goal position
    if (!(paths[i].back() == instance->get_goal_positions()[i])) return false;
  }

  // now check that there are no collisions or invalid points
  for (int t = 0; t < T_max; t++)
  {
    for (int i = 0; i < (int)paths.size(); i++)
    {
      // no need to check paths that are already finished at time t
      if (t > (int)paths[i].size()) continue;

      // check the path does not leave the map
      if (!instance->get_map_data().is_in(paths[i][t])) return false;

      // check collisions with static obstacles
      if (instance->get_map_data().index(paths[i][t]) != 0) return false;

      // TODO check collisions with dynamic obstacles
    }
  }
  return true;
}

Solver::Solver(const std::string& name, std::shared_ptr<Instance> instance) : name(name), instance(instance){};
