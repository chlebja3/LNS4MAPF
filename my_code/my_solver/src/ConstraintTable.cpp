/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "ConstraintTable.h"

#include <climits>
#include <iostream>


//
ConstraintTable::ConstraintTable(const Instance& instance_) : instance(instance_)
{
  // initialize the constraint vectors, static obstacles dont need constraint vectors
  const int num_free_cells = instance.get_num_free_cells();
  constraints              = std::vector<std::list<TimeIntervalNode>>(num_free_cells, std::list<TimeIntervalNode>());
  agents_counts            = std::vector<std::unordered_map<int, int>>(num_free_cells, std::unordered_map<int, int>());
}

template <typename ListIterator>
auto find_previous_timeintervalnode(ListIterator begin, ListIterator current, const TimeInterval& target) -> TimeIntervalNode_it
{
  if (current != begin)
  {
    auto prev = std::prev(current);
    if (prev->interval.t_max + 1 == target.t_min)
    {
      return TimeIntervalNode_it(prev);
    }
  }
  return std::nullopt;
}

template <typename ListIterator>
auto find_next_timeintervalnode(ListIterator end, ListIterator current, const TimeInterval& target) -> TimeIntervalNode_it
{
  if (current != std::prev(end))
  {
    auto next = std::next(current);
    assertm(target.t_max < INT_MAX, "There should not be a constraint after a constraint ending in infinity.");
    if (target.t_max + 1 == next->interval.t_min)
    {
      return TimeIntervalNode_it(next);
    }
  }
  return std::nullopt;
}

void ConstraintTable::add_constraint(const TimePoint& timepoint, int agent_num)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");
  assertm(agent_num >= 0 && agent_num < instance.get_num_of_agents(), "Invalid agent number.");
  int                          free_location = instance.location_to_free_location(timepoint.location);
  std::list<TimeIntervalNode>& cur_TI_list   = constraints[free_location];

  bool inserted = false;
  // find where to insert the interval
  for (auto it = cur_TI_list.begin(); it != cur_TI_list.end(); it++)
  {
    // check that the intervals do not have any overlap

    assertm(!overlap(it->interval, timepoint.interval), "Cannot add overlapping constraints.");

    // insert before the first interval that starts later
    if (it->interval.t_min > timepoint.interval.t_min)
    {
      TimeIntervalNode_it prev_ptr = find_previous_timeintervalnode(cur_TI_list.begin(), it, timepoint.interval);
      TimeIntervalNode_it next_ptr = find_next_timeintervalnode(cur_TI_list.begin(), it, timepoint.interval);
      cur_TI_list.insert(it, {timepoint.interval, agent_num, TIME_INTERVAL_NODE_type::constraint, prev_ptr, next_ptr});
      inserted = true;
      break;
    }
  }

  // if no interval starts later, insert to the back of the list
  if (!inserted)
  {
    TimeIntervalNode_it prev_ptr = std::nullopt;
    if (!cur_TI_list.empty())
    {
      prev_ptr = std::prev(cur_TI_list.end());
    }
    cur_TI_list.emplace_back(timepoint.interval, agent_num, TIME_INTERVAL_NODE_type::constraint, prev_ptr, std::nullopt);
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

auto ConstraintTable::remove_constraint(const TimePoint& timepoint, int agent_num) -> std::pair<TimeIntervalNode_it, TimeIntervalNode_it>
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");
  int free_location = instance.location_to_free_location(timepoint.location);

  std::list<TimeIntervalNode>& cur_TI_list = constraints[free_location];

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
  for (auto it = cur_TI_list.begin(); it != cur_TI_list.end(); it++)
  {
    if (it->interval == timepoint.interval)
    {
      // the interval pointers will be rewired by the caller (safe interval table)
      TimeIntervalNode_it prev = it->prev_interval;
      TimeIntervalNode_it next = it->next_interval;
      cur_TI_list.erase(it);
      return {prev, next};
    }
  }
  // if the interval was not found, throw an error
  throw std::runtime_error("Trying to remove non-existing interval from the constraint table.");
}

auto ConstraintTable::get_blocking_agent(int loc, int time) const -> int
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(loc), "Invalid location.");

  // check the time is valid
  assertm(time >= 0, "Invalid time.");

  // retrieve the constraint list
  int                                loc_free    = instance.location_to_free_location(loc);
  const std::list<TimeIntervalNode>& cur_TI_list = constraints[loc_free];

  // if there are no constraints, return -1 (no agent blocking the square)
  if (cur_TI_list.empty())
  {
    return -1;
  };

  // find the constraint
  for (const auto& it : cur_TI_list)
  {
    const TimeInterval& constraint_tp = it.interval;
    if (constraint_tp.t_min <= time && time <= constraint_tp.t_max)
    {
      // return the agent that created the constraint
      return it.agent_num;
    }
    // skip constraints that are too late
    if (constraint_tp.t_min > time)
    {
      break;
    }
  }

  return -1;
}

auto ConstraintTable::get_blocking_agents(int location, int time_min) const -> std::vector<int>
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(location), "Invalid location.");

  // check the time is valid
  assertm(time_min >= 0, "Invalid time.");

  // retrieve the constraint list
  int                                to_free     = instance.location_to_free_location(location);
  const std::list<TimeIntervalNode>& cur_TI_list = constraints[to_free];

  assertm(!cur_TI_list.empty(), "Trying to get blocking agents from an empty list.");
  assertm(cur_TI_list.back().interval.t_max == INT_MAX, "Last interval should be infinite.");

  std::unordered_set<int> blocking_agents     = {};
  std::vector<int>        blocking_agents_vec = {};

  // check how many intervals are present
  if (cur_TI_list.size() < 2)
  {
    return {};
  }

  // iterate over the constraints from end to beginning (skip the last interval, as agent can not block itself)
  for (auto it = std::next(cur_TI_list.rbegin()); it != cur_TI_list.rend(); it++)
  {
    // std::cout << "Interval: " << it->first << ", agent: " << it->second << std::endl;
    if (it->interval.t_max < time_min)
    {
      // there can not be any more blocking agents
      break;
    }

    // check whether the agent was not already added
    if (blocking_agents.find(it->agent_num) == blocking_agents.end())
    {
      // add the agent to the set
      blocking_agents.insert(it->agent_num);
      blocking_agents_vec.push_back(it->agent_num);
    }
  }
  return blocking_agents_vec;
}

auto ConstraintTable::insert_constraint(const TimePoint& timepoint, int agent_num, TimeIntervalNode_it prev, TimeIntervalNode_it next)
    -> TimeIntervalNode_it
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");
  assertm(agent_num >= 0 && agent_num < instance.get_num_of_agents(), "Invalid agent number.");
  int                          free_location = instance.location_to_free_location(timepoint.location);
  std::list<TimeIntervalNode>& cur_TI_list   = constraints[free_location];

  // try to insert after prev
  if (prev.has_value())
  {
    assertm(prev.value()->type == TIME_INTERVAL_NODE_type::constraint, "Previous time interval node must be a constraint.");
    assertm(prev.value()->interval.t_max + 1 == timepoint.interval.t_min,
            "Previous constraint must end just before the new constraint starts.");
    return cur_TI_list.insert(std::next(prev.value()),
                              TimeIntervalNode(timepoint.interval, agent_num, TIME_INTERVAL_NODE_type::constraint, prev, next));
  }
  // try to insert before next
  if (next.has_value())
  {
    assertm(next.value()->type == TIME_INTERVAL_NODE_type::constraint, "Next time interval node must be a constraint.");
    assertm(timepoint.interval.t_max + 1 == next.value()->interval.t_min, "Next constraint must start just after the new constraint ends.");
    return cur_TI_list.insert(next.value(),
                              TimeIntervalNode(timepoint.interval, agent_num, TIME_INTERVAL_NODE_type::constraint, prev, next));
  }

  // the current time interval list is empty
  assertm(timepoint.interval.t_min == 0 && timepoint.interval.t_max == INT_MAX,
          "If there is no prev and no next, the interval must be infinite, because there can not be any safe intervals.");
  assertm(cur_TI_list.empty(), "If prev and next are empty, cur TI list must be also.");
  return cur_TI_list.insert(cur_TI_list.end(),
                            TimeIntervalNode(timepoint.interval, agent_num, TIME_INTERVAL_NODE_type::constraint, prev, next));
}


void ConstraintTable::delete_constraint(int location, TimeIntervalNode_it node)
{
  assertm(node.has_value(), "Can not delete a node without a value.");
  assertm(node.value()->type == TIME_INTERVAL_NODE_type::constraint, "Deleting constraint, but type is safe interval.");
  assertm(instance.get_map_data().is_in(location), "Invalid location.");

  int                          free_location = instance.location_to_free_location(location);
  std::list<TimeIntervalNode>& cur_TI_list   = constraints[free_location];
  // rewire
  if (node.value()->prev_interval.has_value())
  {
    node.value()->prev_interval.value()->next_interval = node.value()->next_interval;
  }
  if (node.value()->next_interval.has_value())
  {
    node.value()->next_interval.value()->prev_interval = node.value()->prev_interval;
  }
  cur_TI_list.erase(node.value());
}

auto ConstraintTable::remove_constraint_after(const TimePoint& timepoint, int agent_num, TimeIntervalNode_it start)
    -> std::pair<TimeIntervalNode_it, TimeIntervalNode_it>
{
  assertm(start.has_value(), "Start node must have a value.");
  assertm(start.value()->type == TIME_INTERVAL_NODE_type::constraint, "Start node type must be constraint.");
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");
  int                          free_location = instance.location_to_free_location(timepoint.location);
  std::list<TimeIntervalNode>& cur_TI_list   = constraints[free_location];

  // go through the list until the constraint is found
  for (auto it = start.value(); it != cur_TI_list.end(); std::next(it))
  {
    if (it->interval == timepoint.interval)
    {
      assertm(agent_num == it->agent_num, "The agent num must be the same.");
      TimeIntervalNode_it prev = it->prev_interval;
      TimeIntervalNode_it next = it->next_interval;
      // rewire
      if (prev.has_value())
      {
        prev.value()->next_interval = next;
      }
      if (next.has_value())
      {
        next.value()->prev_interval = prev;
      }

      // remove it from the list
      cur_TI_list.erase(it);
      return std::make_pair(prev, next);
    }
    assertm(it->interval.t_max < timepoint.interval.t_min, "The constraint is not present in the list.");
  }
  throw std::runtime_error("Could not find the constraint in the constraint list.");
}
