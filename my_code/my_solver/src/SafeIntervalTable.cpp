/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "SafeIntervalTable.h"

#include <climits>
#include <iostream>

SafeIntervalTable::SafeIntervalTable(const Instance& instance_) : instance(instance_)
{
  const int num_free_cells = instance.get_num_free_cells();

  // initialize the safe interval vectors, cells with static obstacles dont need safe interval
  safe_intervals = std::vector<std::list<TimeIntervalNode>>(
      num_free_cells, std::list<TimeIntervalNode>(
                          1, TimeIntervalNode(TimeInterval(0, INT_MAX), -1, -1, TIME_INTERVAL_NODE_type::safe_interval,
                                              std::nullopt, std::nullopt, std::nullopt)));
  // set the right location
  for (int i = 0; i < instance.get_num_free_cells(); i++)
  {
    safe_intervals[i].begin()->location = instance.free_location_to_location(i);
  }
}

auto SafeIntervalTable::add_constraint(const TimePoint& timepoint, int agent_num, ConstraintTable& constraint_table,
                                       TimeIntervalNode_it& prev_in_path) -> TimeIntervalNode_it
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");

  int free_location = instance.location_to_free_location(timepoint.location);
  std::list<TimeIntervalNode>& cur_TI_list = safe_intervals[free_location];
  TimeIntervalNode_it constraint_node_ptr = std::nullopt;

  // modify intervals that have some intersections with the time range
  auto it = cur_TI_list.begin();
  while (it != cur_TI_list.end())
  {
    // skip irrelevant time intervals
    if (it->interval.t_max < timepoint.interval.t_min)
    {
      it++;
      continue;
    }

    // check that there are still intervals that could be reduced
    assertm(it->interval.t_min <= timepoint.interval.t_max, "Can not add an overlapping constraint.");

    // shorten intervals starting before the range
    // prev interval stays the same, next becomes the new constraint
    if (it->interval.t_min < timepoint.interval.t_min)
    {
      assertm(it->interval.t_max >= timepoint.interval.t_max, "Can not add an overlapping constraint.");
      const int new_tmax = timepoint.interval.t_min - 1;

      // create new interval if there is still some time in the safe interval after the constraint
      if (it->interval.t_max > timepoint.interval.t_max)
      {
        constraint_node_ptr = constraint_table.insert_constraint(timepoint, agent_num, std::nullopt, it->next_interval);
        auto it_new = cur_TI_list.insert(
            std::next(it, 1), TimeIntervalNode(TimeInterval(timepoint.interval.t_max + 1, it->interval.t_max), NO_AGENT,
                                               timepoint.location, TIME_INTERVAL_NODE_type::safe_interval,
                                               constraint_node_ptr, it->next_interval));
        // change next interval of the constraint to the new safe interval
        constraint_node_ptr.value()->next_interval = {it_new};
        // change prev interval of the constraint to the old safe interval
        constraint_node_ptr.value()->prev_interval = {it};

        // change previous interval of the next interval to the new safe interval
        if (it_new->next_interval.has_value())
        {
          it_new->next_interval.value()->prev_interval = {it_new};
        }
      }
      it->interval.t_max = new_tmax;
      if (!constraint_node_ptr.has_value())  // constraint node was not yet inserted to the ConstraintTable
      {
        constraint_node_ptr = constraint_table.insert_constraint(timepoint, agent_num, std::nullopt, it->next_interval);
        constraint_node_ptr.value()->prev_interval = {it};
        if (it->next_interval.has_value())
        {
          it->next_interval.value()->prev_interval = constraint_node_ptr;
        }
      }
      assertm(constraint_node_ptr != std::nullopt, "Constraint node pointer can not be a null pointer.");
      it->next_interval = constraint_node_ptr;
      constraint_node_ptr.value()->prev_in_path = prev_in_path;
      return constraint_node_ptr;
    }
    // shorten intervals ending after the range
    if (it->interval.t_max > timepoint.interval.t_max)
    {
      assertm(it->interval.t_min == timepoint.interval.t_min, "Can not add an overlapping constraint.");
      constraint_node_ptr =
          constraint_table.insert_constraint(timepoint, agent_num, it->prev_interval, it->next_interval);
      // rewire prev
      if (it->prev_interval.has_value())
      {
        it->prev_interval.value()->next_interval = constraint_node_ptr;
      }
      it->interval.t_min = timepoint.interval.t_max + 1;
      // rewire next
      it->prev_interval = constraint_node_ptr;
      assertm(constraint_node_ptr.has_value(), "Constraint pointer must have value.");
      constraint_node_ptr.value()->next_interval = {it};
      constraint_node_ptr.value()->prev_in_path = prev_in_path;
      return constraint_node_ptr;
    }

    // delete intervals that are fully overlapped by the constraint
    assertm(it->interval == timepoint.interval, "The intervals should be the same.");
    constraint_node_ptr =
        constraint_table.insert_constraint(timepoint, agent_num, it->prev_interval, it->next_interval);
    // rewire prev
    if (it->prev_interval.has_value())
    {
      it->prev_interval.value()->next_interval = constraint_node_ptr;
    }
    // rewire next
    if (it->next_interval.has_value())
    {
      it->next_interval.value()->prev_interval = constraint_node_ptr;
    }

    it = cur_TI_list.erase(it);
    constraint_node_ptr.value()->prev_in_path = prev_in_path;
    return constraint_node_ptr;
  }
  throw std::runtime_error("Did not add any constraint.");
}

void SafeIntervalTable::remove_constraint(const TimePoint& timepoint, int agent_num, ConstraintTable& constraint_table)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");
  // std::cout << "Removing, loc: " << timepoint.location << ", " << timepoint.interval << std::endl;

  int free_location = instance.location_to_free_location(timepoint.location);
  std::list<TimeIntervalNode>& cur_TI_list = safe_intervals[free_location];

  // check whether there are any safe intervals
  if (cur_TI_list.empty())
  {
    auto [prev, next] = constraint_table.remove_constraint(timepoint, agent_num);
    cur_TI_list.emplace_back(timepoint.interval, agent_num, timepoint.location, TIME_INTERVAL_NODE_type::safe_interval,
                             prev, next);
    // rewire
    if (prev.has_value())
    {
      prev.value()->next_interval = {cur_TI_list.begin()};
    }
    if (next.has_value())
    {
      next.value()->prev_interval = {cur_TI_list.begin()};
    }
    return;
  }

  // firstly handle constraints after the last safe interval (there is no safe interval after the
  // constraint)
  assertm(timepoint.interval.t_min != cur_TI_list.back().interval.t_max,
          "Constraint start overlaps with last safe interval end.");

  if (timepoint.interval.t_min > cur_TI_list.back().interval.t_max)
  {
    // if the last safe interval precedes directly, extend it
    if (cur_TI_list.back().interval.t_max == timepoint.interval.t_min - 1)
    {
      cur_TI_list.back().interval.t_max = timepoint.interval.t_max;
      auto to_del = cur_TI_list.back().next_interval;
      assertm(to_del.has_value(), "Constraint we are deleting must have value.");
      cur_TI_list.back().next_interval = to_del.value()->next_interval;
      // rewire
      if (cur_TI_list.back().next_interval.has_value())
      {
        cur_TI_list.back().next_interval.value()->prev_interval = {std::prev(cur_TI_list.end())};
      }
      constraint_table.delete_constraint(timepoint.location, to_del);
      return;
    }
    // otherwise add a new safe interval
    auto [prev, next] =
        constraint_table.remove_constraint_after(timepoint, agent_num, cur_TI_list.back().next_interval);
    cur_TI_list.emplace_back(timepoint.interval, agent_num, timepoint.location, TIME_INTERVAL_NODE_type::safe_interval,
                             prev, next);
    // rewire
    if (prev.has_value())
    {
      prev.value()->next_interval = {std::prev(cur_TI_list.end())};
    }
    if (next.has_value())
    {
      next.value()->prev_interval = {std::prev(cur_TI_list.end())};
    }
    return;
  }

  // find interval after the constraint
  auto it = cur_TI_list.begin();
  while (it != cur_TI_list.end())
  {
    assertm(!overlap(it->interval, timepoint.interval),
            "Constraint interval can not have any overlap with safe interval.");
    // std::cout << it->interval << std::endl;

    // find the interval after the constraint
    if (it->interval.t_min > timepoint.interval.t_max)
    {
      // check whether there was an interval before the constraint
      if (it != cur_TI_list.begin())
      {
        // check whether the previous timeinterval can be extended
        const auto prev = std::prev(it, 1);
        if (prev->interval.t_max == timepoint.interval.t_min - 1)
        {
          // check whether prev should be merged with next interval
          if (timepoint.interval.t_max != INT_MAX && it->interval.t_min == timepoint.interval.t_max + 1)
          {
            prev->interval.t_max = it->interval.t_max;
            assertm(prev->next_interval == it->prev_interval, "The pointers to the constraint should be the same.");
            // std::cout << "prev: " << prev->interval << ", const: " << prev->next_interval.value()->interval
            //           << ", next: " << it->interval << std::endl;
            constraint_table.delete_constraint(timepoint.location, prev->next_interval);
            prev->next_interval = it->next_interval;
            if (it->next_interval.has_value())
            {
              it->next_interval.value()->prev_interval = {prev};
            }
            it = cur_TI_list.erase(it);
          }
          else  // extend prev
          {
            auto start = prev->prev_interval;
            if (!start.has_value())
            {
              start = constraint_table.get_first_constraint(timepoint.location);
            }
            auto [prev_interval, next_interval] = constraint_table.remove_constraint_after(timepoint, agent_num, start);
            // rewire
            prev->next_interval = next_interval;
            if (next_interval.has_value())
            {
              next_interval.value()->prev_interval = {prev};
            }
            prev->interval.t_max = timepoint.interval.t_max;
          }
          return;
        }
      }

      // previous can not be extended, check whether next can be extended
      if (timepoint.interval.t_max != INT_MAX && it->interval.t_min == timepoint.interval.t_max + 1)
      {
        it->interval.t_min = timepoint.interval.t_min;
        assertm(it->prev_interval.has_value() && it->prev_interval.value()->type == TIME_INTERVAL_NODE_type::constraint,
                "Previous interval must be the constraint.");
        // rewire prev interval of it
        auto constr = it->prev_interval.value();
        it->prev_interval = constr->prev_interval;
        constraint_table.delete_constraint(timepoint.location, constr);
        // rewire next interval of prev interval
        if (it->prev_interval.has_value())
        {
          it->prev_interval.value()->next_interval = {it};
        }
        return;
      }

      // no interval can be extended, therefore insert a new interval
      TimeIntervalNode_it curr;

      if (it != cur_TI_list.begin())
      {
        curr = std::prev(it)->next_interval;
      }
      else  // there is no prev, use the first constraint
      {
        curr = constraint_table.get_first_constraint(timepoint.location);
      }
      // std::cout << "Searching constraint from: " << curr.value()->interval << std::endl;
      auto [prev_interval, next_interval] = constraint_table.remove_constraint_after(timepoint, agent_num, curr);
      // std::cout << "prev: " << prev_interval.value()->interval << ", next: " << next_interval.value()->interval
      //           << std::endl;
      auto new_it = cur_TI_list.insert(
          it, TimeIntervalNode(timepoint.interval, NO_AGENT, timepoint.location, TIME_INTERVAL_NODE_type::safe_interval,
                               prev_interval, next_interval));
      // rewire prev
      if (prev_interval.has_value())
      {
        prev_interval.value()->next_interval = {new_it};
      }
      if (next_interval.has_value())
      {
        next_interval.value()->prev_interval = {new_it};
      }
      return;
    }
    it++;
  }
  throw std::runtime_error("Did not remove any constraint.");
}

auto SafeIntervalTable::get_first_safe_interval(int location) const -> TimeIntervalNode_const_it
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(location), "Invalid location.");

  int free_location = instance.location_to_free_location(location);
  const std::list<TimeIntervalNode>& cur_TI_list = safe_intervals[free_location];

  // there is no safe interval
  if (cur_TI_list.empty())
  {
    return std::nullopt;
  }

  return {cur_TI_list.cbegin()};
}

auto SafeIntervalTable::get_safe_intervals(int location, const TimeInterval& time_interval) const
    -> std::pair<std::list<TimeIntervalNode>::const_iterator, std::list<TimeIntervalNode>::const_iterator>
{
  // this implementation does not check edge collisions, so they must be handled elsewhere.
  assertm(instance.get_map_data().is_in(location), "Invalid location.");
  int free_location = instance.location_to_free_location(location);
  assertm(free_location < static_cast<int>(safe_intervals.size()), "Safe intervals not precomputed for this location.");

  // iterate over the list to find the first and last safe interval
  auto start = safe_intervals[free_location].cend();
  auto end = safe_intervals[free_location].cend();
  for (auto it = safe_intervals[free_location].cbegin(); it != safe_intervals[free_location].cend(); it++)
  {
    // std::cout << "Interval: " << it->t_min << " " << it->t_max << std::endl;
    // check whether the interval is inside the given time range
    if (it->interval.t_max >= time_interval.t_min && it->interval.t_min <= time_interval.t_max)
    {
      // remember the first interval in the range
      if (start == safe_intervals[free_location].end())
      {
        start = it;
      }
    }
    // if the interval is not inside the time range, check whether already found some intervals
    else if (start != safe_intervals[free_location].cend())
    {
      // if already found some intervals, return the found safe intervals
      end = it;
      return std::make_pair(start, end);
    }
  }
  return std::make_pair(start, end);
}

[[nodiscard]] auto SafeIntervalTable::get_first_safe_interval_after_time(int location,
                                                                         const TimeInterval& time_interval) const
    -> std::list<TimeIntervalNode>::const_iterator
{
  assertm(instance.get_map_data().is_in(location), "Invalid location.");
  int free_location = instance.location_to_free_location(location);
  assertm(free_location < static_cast<int>(safe_intervals.size()), "Safe intervals not precomputed for this location.");

  // iterate over the list to find the first and last safe interval
  for (auto it = safe_intervals[free_location].cbegin(); it != safe_intervals[free_location].cend(); it++)
  {
    // check whether the interval is inside the given time range
    if (it->interval.t_max >= time_interval.t_min && it->interval.t_min <= time_interval.t_max)
    {
      return it;
    }
  }
  // if nothing found, return the end of the list
  return safe_intervals[free_location].cend();
}

/*
void SafeIntervalTable::update_latest_constraint_end_estimate()
{
  latest_constraint_end = 0;
  // TODO iterate only for last intervals
  for (const auto& safe_interval_list : safe_intervals)
  {
    const auto last = safe_interval_list.back();

    // safe interval reaches the end
    if (last.t_max == INT_MAX)
    {
      latest_constraint_end = std::max(latest_constraint_end, last.t_min - 1);
    }
  }

  latest_constraint_end_updated = true;
}

[[nodiscard]] auto SafeIntervalTable::get_max_path_len_estimate() -> int
{
  if (unlimited_safe_intervals == 0)
  {
    return INT_MAX;
  }

  if (!latest_constraint_end_updated)
  {
    update_latest_constraint_end_estimate();
  }
  // check for overflow
  if (latest_constraint_end <= INT_MAX - unlimited_safe_intervals)
  {
    return latest_constraint_end + unlimited_safe_intervals;
  }
  return INT_MAX;
} */
