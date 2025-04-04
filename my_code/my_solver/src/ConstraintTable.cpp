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
ConstraintTable::ConstraintTable(const Instance& instance_)
    : edge_constraint_table(instance_), locks(instance_.get_num_free_cells()), instance(instance_)
{
  // initialize the constraint vectors, static obstacles dont need constraint vectors
  const int num_free_cells = instance.get_num_free_cells();
  constraints   = std::vector<std::list<std::pair<TimeInterval, int>>>(num_free_cells, std::list<std::pair<TimeInterval, int>>());
  agents_counts = std::vector<std::unordered_map<int, int>>(num_free_cells, std::unordered_map<int, int>());
}

void ConstraintTable::add_constraint(const TimePoint& timepoint, int agent_num)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");
  int                                      free_location = instance.location_to_free_location(timepoint.location);
  std::list<std::pair<TimeInterval, int>>& cur_TI_list   = constraints[free_location];

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

  // add to the set of visiting agents
  // if the agent is not yet in the set, add it with count 1
  if (agents_counts[free_location].find(agent_num) == agents_counts[free_location].end())
  {
    agents_counts[free_location][agent_num] = 1;
  }
  else
  {
    // else increase the count
    agents_counts[free_location][agent_num] += 1;
  }
}

void ConstraintTable::remove_constraint(const TimePoint& timepoint, int agent_num)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");
  int free_location = instance.location_to_free_location(timepoint.location);


  std::list<std::pair<TimeInterval, int>>& cur_TI_list = constraints[free_location];

  // check that the list is not empty
  assertm(!cur_TI_list.empty(), "Trying to remove interval from an empty list.");

  assertm(agents_counts[free_location].find(agent_num) != agents_counts[free_location].end(),
          "Removing agent that is not in the agents counts");
  agents_counts[free_location][agent_num] -= 1;
  // if the count drops to 0, erase the agent number from keys
  if (agents_counts[free_location][agent_num] == 0)
  {
    agents_counts[free_location].erase(agent_num);
  }

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
    // std::cout << "Adding: " << path[i].location << ", " << path[i].interval << std::endl;
    add_constraint(path[i], agent_num);

    // add the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      // the time when the edge is used is the start time of the later interval
      edge_constraint_table.add(path[i - 1].location, path[i].location, path[i].interval.t_min, agent_num);
    }
  }
}


void ConstraintTable::add_constraints_parallel(const TimePointPath& path, int agent_num)
{
#pragma omp parallel for
  for (int i = 0; i < (int)path.size(); i++)
  {
    // lock the location
    omp_set_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);

    // add the point constraint
    add_constraint(path[i], agent_num);

    // add the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      // the time when the edge is used is the start time of the later interval
      edge_constraint_table.add(path[i - 1].location, path[i].location, path[i].interval.t_min, agent_num);
    }
    // release the lock
    omp_unset_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);
  }
}

void ConstraintTable::remove_constraints(const TimePointPath& path, int agent_num)
{
  for (int i = 0; i < (int)path.size(); i++)
  {
    // remove the point constraint
    // std::cout << "Removing: " << path[i].location << ", " << path[i].interval << std::endl;
    remove_constraint(path[i], agent_num);

    // remove the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      edge_constraint_table.remove(path[i - 1].location, path[i].location, path[i].interval.t_min);
    }
  }
}

void ConstraintTable::remove_constraints_parallel(const TimePointPath& path, int agent_num)
{
#pragma omp parallel for
  for (int i = 0; i < (int)path.size(); i++)
  {
    // lock the location
    omp_set_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);

    // remove the point constraint
    remove_constraint(path[i], agent_num);

    // remove the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      edge_constraint_table.remove(path[i - 1].location, path[i].location, path[i].interval.t_min);
    }

    // unlock the location
    omp_unset_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);
  }
}

auto ConstraintTable::get_blocking_agent(int from, int to, int time) const -> int
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(from) && instance.get_map_data().is_in(to), "Invalid location.");

  // check the time is valid
  assertm(time >= 0, "Invalid time.");

  int to_free = instance.location_to_free_location(to);

  const std::list<std::pair<TimeInterval, int>>& cur_TI_list = constraints[to_free];

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


void ConstraintTable::build_sequential(const std::vector<TimePointPath>& paths)
{
  assertm(static_cast<int>(paths.size()) > 0, "No paths to add.");
  for (int i = 0; i < static_cast<int>(paths.size()); i++)
  {
    add_constraints(paths[i], i);
  }
}

void ConstraintTable::build_parallel(const std::vector<TimePointPath>& paths)
{
#pragma omp parallel for
  for (int agent_num = 0; agent_num < static_cast<int>(paths.size()); agent_num++)
  {
    const TimePointPath& path = paths[agent_num];
    for (int i = 0; i < (int)path.size(); i++)
    {
      // add the point constraint
      omp_set_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);
      add_constraint(path[i], agent_num);
      // add the edge constraint
      if (i > 0)  // skip the first point as there is no edge
      {
        // the time when the edge is used is the start time of the later interval
        edge_constraint_table.add(path[i - 1].location, path[i].location, path[i].interval.t_min, agent_num);
      }
      omp_unset_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);
    }
  }
}

void ConstraintTable::build_parallel_2(const std::vector<TimePointPath>& paths)
{
  // calculate how many constraints to add
  int total_work = 0;
  for (const auto& path : paths)
  {
    total_work += static_cast<int>(path.size());
  }
  assertm(total_work > 0, "No constraints to add.");

  std::vector<const TimePoint*> timepoints(total_work);
  std::vector<int>              agent_nums(total_work);
  std::vector<int>              prev_location(total_work, -1);
  int                           idx = 0;
  for (int i = 0; i < static_cast<int>(paths.size()); i++)
  {
    const auto& path = paths[i];
    for (int j = 0; j < (int)path.size(); j++)
    {
      timepoints[idx] = &path[j];
      agent_nums[idx] = i;
      if (j > 0)
      {
        prev_location[idx] = path[j - 1].location;
      }
      idx++;
    }
  }

#pragma omp parallel for
  for (int i = 0; i < total_work; i++)
  {
    // add the point constraint
    omp_set_lock(&locks.locks[instance.location_to_free_location(timepoints[i]->location)]);
    add_constraint(*timepoints[i], agent_nums[i]);
    // add the edge constraint
    if (prev_location[i] != -1)  // skip the first point as there is no edge
    {
      // the time when the edge is used is the start time of the later interval
      edge_constraint_table.add(prev_location[i], timepoints[i]->location, timepoints[i]->interval.t_min, agent_nums[i]);
    }
    omp_unset_lock(&locks.locks[instance.location_to_free_location(timepoints[i]->location)]);
  }
}

EdgeConstraintTableWithAgentNums::EdgeConstraintTableWithAgentNums(const Instance& instance_) : instance(instance_)
{
  // initialize the edge constraints
  edge_constraints.resize(NUM_DIRECTIONS);
  for (int i = 0; i < NUM_DIRECTIONS; i++)
  {
    edge_constraints[i].resize(instance.get_num_free_cells());
  }
}

void EdgeConstraintTableWithAgentNums::add(int loc1, int loc2, int time, int agent_num)
{
  assertm(loc1 != loc2, "Invalid edge constraint.");
  const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;
  assertm(dir >= 0 && dir <= NUM_DIRECTIONS, "Invalid direction.");
  // assertm(edge_constraints[dir][loc2].find(time) == edge_constraints[dir][loc2].end(), "Edge constraint already exists.");
  int loc2_free = instance.location_to_free_location(loc2);
  assertm(std::find_if(edge_constraints[dir][loc2_free].begin(), edge_constraints[dir][loc2_free].end(),
                       [time](const std::pair<int, int>& p) { return p.first == time; }) == edge_constraints[dir][loc2_free].end(),
          "Edge constraint already exists.");
  // edge_constraints[dir][loc2].insert(time);     // unordered set implementation
  edge_constraints[dir][loc2_free].push_back({time, agent_num});  // vector implementation
}

void EdgeConstraintTableWithAgentNums::remove(int loc1, int loc2, int time)
{
  const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;
  assertm(dir >= 0 && dir <= NUM_DIRECTIONS, "Invalid direction.");
  int loc2_free = instance.location_to_free_location(loc2);
// int erased = edge_constraints[dir][loc2].erase(time);
// assertm(erased == 1, "Trying to erase an edge constraint that is not present.");
#ifndef NDEBUG
  int constraints_before = edge_constraints[dir][loc2_free].size();
  // std::cout << "Constraints before: " << constraints_before << std::endl;
#endif
  edge_constraints[dir][loc2_free].erase(std::remove_if(edge_constraints[dir][loc2_free].begin(), edge_constraints[dir][loc2_free].end(),
                                                        [time](const std::pair<int, int>& p) { return p.first == time; }),
                                         edge_constraints[dir][loc2_free].end());
  // std::cout << "Constraints after: " << edge_constraints[dir][loc2].size() << std::endl;
  assertm(constraints_before - edge_constraints[dir][loc2_free].size() == 1, "Should erase exactly one edge constraint.");
}


auto EdgeConstraintTableWithAgentNums::get(int loc1, int loc2, int time) const -> int
{
  const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;  // dont consider NONE direction
  assertm(dir >= 0 && dir < NUM_DIRECTIONS, "Invalid direction.");

  int loc2_free = instance.location_to_free_location(loc2);
  // return edge_constraints[dir][loc2].find(time) != edge_constraints[dir][loc2].end(); // unordered set implementation
  auto it = std::find_if(edge_constraints[dir][loc2_free].begin(), edge_constraints[dir][loc2_free].end(),
                         [time](const std::pair<int, int>& p) { return p.first == time; });
  // check whether found
  if (it == edge_constraints[dir][loc2_free].end())
  {
    return -1;
  }
  return it->second;
}


omp_locks::omp_locks(int num_cells)
{
  locks.resize(num_cells, omp_lock_t());
  // initialize the locks
  for (int i = 0; i < num_cells; i++)
  {
    omp_init_lock(&locks[i]);
  }
}

omp_locks::~omp_locks()
{
  // destroy the locks
  for (auto& lock : locks)
  {
    omp_destroy_lock(&lock);
  }
}
