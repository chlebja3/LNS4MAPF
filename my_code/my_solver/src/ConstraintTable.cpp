/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "ConstraintTable.h"

#include <boost/functional/hash.hpp>
#include <climits>
#include <iostream>

constexpr int NUM_DIRECTIONS = magic_enum::enum_count<Direction>() - 1;  // dont consider NONE direction
                                                                         //
ConstraintTable::ConstraintTable(const Instance& instance_) : edge_constraint_table(instance_.get_num_cells()), instance(instance_)
{
  // initialize the constraint vectors TODO static obstacles dont need constraint vectors
  const int num_cells = instance.get_num_cells();
  constraints         = std::vector<std::list<std::pair<TimeInterval, int>>>(num_cells, std::list<std::pair<TimeInterval, int>>());
}

void ConstraintTable::add_constraint(const TimePoint& timepoint, int agent_num)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");

  std::list<std::pair<TimeInterval, int>>& cur_TI_list = constraints[timepoint.location];

  bool inserted = false;
  // find where to insert the interval
  for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
  {
    // check that the intervals do not have any overlap

    assertm(!overlap((*it).first, timepoint.interval), "Cannot add overlapping constraints.");

    // insert before the first interval that starts later
    if (it->first.t_min > timepoint.interval.t_min)
    {
      // check that there is no overlap
      // assertm(timepoint.interval.t_max < it->t_min, "Cannot add overlapping constraints.");
      cur_TI_list.insert(it, {timepoint.interval, agent_num});
      inserted = true;
      break;
    }
  }

  // if no interval starts later, insert to the back of the list
  if (!inserted)
  {
    cur_TI_list.emplace_back(timepoint.interval, agent_num);
  }
}

void ConstraintTable::remove_constraint(const TimePoint& timepoint)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");

  std::list<std::pair<TimeInterval, int>>& cur_TI_list = constraints[timepoint.location];

  // check that the list is not empty
  assertm(!cur_TI_list.empty(), "Trying to remove interval from an empty list.");

  // remove the interval from the list
  for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
  {
    if ((*it).first == timepoint.interval)
    {
      cur_TI_list.erase(it);
      return;
    }
  }
  // if the interval was not found, throw an error
  throw std::runtime_error("Trying to remove non-existing interval from the constraint table.");
}


void ConstraintTable::add_constraints(const TimePointPath& path, int agent_num)
{
  for (int i = 0; i < (int)path.size(); i++)
  {
    // add the point constraint
    add_constraint(path[i], agent_num);

    // add the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      // the time when the edge is used is the start time of the later interval
      edge_constraint_table.add(path[i - 1].location, path[i].location, path[i].interval.t_min, agent_num);
    }
  }
}

void ConstraintTable::remove_constraints(const TimePointPath& path)
{
  for (int i = 0; i < (int)path.size(); i++)
  {
    // remove the point constraint
    remove_constraint(path[i]);

    // remove the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      edge_constraint_table.remove(path[i - 1].location, path[i].location, path[i].interval.t_min);
    }
  }
}

auto ConstraintTable::get_blocking_agent(int from, int to, int time) const -> int
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(from) && instance.get_map_data().is_in(to), "Invalid location.");

  // check the time is valid
  assertm(time >= 0, "Invalid time.");

  const std::list<std::pair<TimeInterval, int>>& cur_TI_list = constraints[to];

  // if there are no constraints, return -1 (no agent blocking the square)
  if (cur_TI_list.empty())
  {
    return -1;
  };

  // find the constraint
  for (const auto& it : cur_TI_list)
  {
    const TimeInterval& constraint_tp = it.first;
    if (constraint_tp.t_min <= time && constraint_tp.t_max >= time)
    {
      // return the agent that created the constraint
      return it.second;
    }
  }

  // find edge constraint
  if (from != to)
  {
    // we want to look for opposite edge, because there can not be any edge ending in to location without location constraint
    return edge_constraint_table.get(to, from, time);
  }
  return -1;
}

EdgeConstraintTableWithAgentNums::EdgeConstraintTableWithAgentNums(int num_locations)
{
  // initialize the edge constraints
  edge_constraints.resize(NUM_DIRECTIONS);
  for (int i = 0; i < NUM_DIRECTIONS; i++)
  {
    edge_constraints[i].resize(num_locations);
  }
}

void EdgeConstraintTableWithAgentNums::add(int loc1, int loc2, int time, int agent_num)
{
  assertm(loc1 != loc2, "Invalid edge constraint.");
  const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;
  assertm(dir >= 0 && dir <= NUM_DIRECTIONS, "Invalid direction.");
  // assertm(edge_constraints[dir][loc2].find(time) == edge_constraints[dir][loc2].end(), "Edge constraint already exists.");
  assertm(std::find_if(edge_constraints[dir][loc2].begin(), edge_constraints[dir][loc2].end(),
                       [time](const std::pair<int, int>& p) { return p.first == time; }) == edge_constraints[dir][loc2].end(),
          "Edge constraint already exists.");
  // edge_constraints[dir][loc2].insert(time);     // unordered set implementation
  edge_constraints[dir][loc2].push_back({time, agent_num});  // vector implementation
}

void EdgeConstraintTableWithAgentNums::remove(int loc1, int loc2, int time)
{
  const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;
  assertm(dir >= 0 && dir <= NUM_DIRECTIONS, "Invalid direction.");

// int erased = edge_constraints[dir][loc2].erase(time);
// assertm(erased == 1, "Trying to erase an edge constraint that is not present.");
#ifndef NDEBUG
  int constraints_before = edge_constraints[dir][loc2].size();
  // std::cout << "Constraints before: " << constraints_before << std::endl;
#endif
  edge_constraints[dir][loc2].erase(std::remove_if(edge_constraints[dir][loc2].begin(), edge_constraints[dir][loc2].end(),
                                                   [time](const std::pair<int, int>& p) { return p.first == time; }),
                                    edge_constraints[dir][loc2].end());
  // std::cout << "Constraints after: " << edge_constraints[dir][loc2].size() << std::endl;
  assertm(constraints_before - edge_constraints[dir][loc2].size() == 1, "Should erase exactly one edge constraint.");
}


auto EdgeConstraintTableWithAgentNums::get(int loc1, int loc2, int time) const -> int
{
  const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;  // dont consider NONE direction
  assertm(dir >= 0 && dir < NUM_DIRECTIONS, "Invalid direction.");

  // return edge_constraints[dir][loc2].find(time) != edge_constraints[dir][loc2].end(); // unordered set implementation
  auto it = std::find_if(edge_constraints[dir][loc2].begin(), edge_constraints[dir][loc2].end(),
                         [time](const std::pair<int, int>& p) { return p.first == time; });
  // check whether found
  if (it == edge_constraints[dir][loc2].end())
  {
    return -1;
  }
  return it->second;
}
