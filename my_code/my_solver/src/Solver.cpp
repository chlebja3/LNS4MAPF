/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "Solver.h"

#include <unordered_set>

bool Solution::is_valid(std::shared_ptr<Instance> instance) const
{
  // check that each agent has a path
  if (instance->num_of_agents != (int)paths.size()) return false;

  // check that each path starts and ends in the correct position and find the longest path
  int T_max = 0;
  for (int i = 0; i < (int)paths.size(); i++)
  {
    // check that no path has length 0
    if (paths[i].empty()) return false;

    // check that the path starts in the start position
    if (!(paths[i].front() == instance->get_start_positions()[i])) return false;

    // check that the path ends in the goal position
    if (!(paths[i].back() == instance->get_goal_positions()[i])) return false;

    // remember the length of the longest path
    if ((int)paths[i].size() > T_max) T_max = paths[i].size();
  }

  // now check that there are no collisions or invalid points
  for (int t = 0; t < T_max; t++)
  {
    // create reservation set for each time step to check for dynamic coliisions
    std::unordered_set<Point2d, Point2d::hash_point> occupied;

    for (int i = 0; i < (int)paths.size(); i++)
    {
      int       idx          = t;
      const int cur_path_len = (int)paths[i].size();

      // paths that are already finished at time t
      if (t > cur_path_len)
      {
        // stay at the last point
        idx = cur_path_len - 1;
      }

      // check the path does not leave the map
      if (!instance->get_map_data().is_in(paths[i][idx])) return false;

      // check collisions with static obstacles
      if (instance->get_map_data().index(paths[i][idx]) != 0) return false;

      // check collisions with dynamic obstacles
      if (occupied.find(paths[i][idx]) != occupied.end()) return false;

      // add the point to the reservation set
      occupied.insert(paths[i][idx]);
    }
  }
  return true;
}

Solver::Solver(const std::string& name, std::shared_ptr<Instance> instance) : name(name), instance(instance){};
