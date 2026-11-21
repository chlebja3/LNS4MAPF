/*
 * Author: Jan Chleboun
 * Date: 21-06-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <EdgeTable.h>

EdgeTable::EdgeTable(const Instance& instance_) : instance(instance_)
{
  // initialize the edge constraints
  edge_constraints.resize(NUM_DIRECTIONS);
  const int num_free_cells = instance.get_num_free_cells();
  for (int i = 0; i < NUM_DIRECTIONS; i++)
  {
    edge_constraints[i].resize(num_free_cells);
  }
}

void EdgeTable::add(int from, int to, int time, int agent_num)
{
  assertm(from != to, "Invalid edge constraint.");
  const int dir = static_cast<int>(find_direction(from, to)) - 1;
  assertm(dir >= 0 && dir < NUM_DIRECTIONS, "Invalid direction.");
  // assertm(edge_constraints[dir][loc2].find(time) == edge_constraints[dir][loc2].end(), "Edge constraint already exists.");
  int to_free = instance.location_to_free_location(to);
  assertm(std::find_if(edge_constraints[dir][to_free].begin(), edge_constraints[dir][to_free].end(),
                       [time](const std::pair<int, int>& p) { return p.first == time; }) == edge_constraints[dir][to_free].end(),
          "Edge constraint already exists.");
  // edge_constraints[dir][loc2].insert(time);     // unordered set implementation
  edge_constraints[dir][to_free].push_back({time, agent_num});  // vector implementation
}

void EdgeTable::remove(int from, int to, int time)
{
  const int dir = static_cast<int>(find_direction(from, to)) - 1;
  assertm(dir >= 0 && dir < NUM_DIRECTIONS, "Invalid direction.");
  int to_free = instance.location_to_free_location(to);
// int erased = edge_constraints[dir][loc2].erase(time);
// assertm(erased == 1, "Trying to erase an edge constraint that is not present.");
#ifndef NDEBUG
  int constraints_before = edge_constraints[dir][to_free].size();
  // std::cout << "Constraints before: " << constraints_before << std::endl;
#endif
  edge_constraints[dir][to_free].erase(std::remove_if(edge_constraints[dir][to_free].begin(), edge_constraints[dir][to_free].end(),
                                                        [time](const std::pair<int, int>& p) { return p.first == time; }),
                                         edge_constraints[dir][to_free].end());
  // std::cout << "Constraints after: " << edge_constraints[dir][loc2].size() << std::endl;
  assertm(constraints_before - edge_constraints[dir][to_free].size() == 1, "Should erase exactly one edge constraint.");
}


auto EdgeTable::get(int from, int to, int time) const -> int
{
  const int dir = static_cast<int>(find_direction(from, to)) - 1;  // dont consider NONE direction
  assertm(dir >= 0 && dir < NUM_DIRECTIONS, "Invalid direction.");

  int to_free = instance.location_to_free_location(to);
  // return edge_constraints[dir][loc2].find(time) != edge_constraints[dir][loc2].end(); // unordered set implementation
  auto it = std::find_if(edge_constraints[dir][to_free].begin(), edge_constraints[dir][to_free].end(),
                         [time](const std::pair<int, int>& p) { return p.first == time; });
  // check whether found
  if (it == edge_constraints[dir][to_free].end())
  {
    return -1;
  }
  return it->second;
}
