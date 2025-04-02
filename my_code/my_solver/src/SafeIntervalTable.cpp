/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "SafeIntervalTable.h"

#include <boost/functional/hash.hpp>
#include <climits>
#include <iostream>

auto EdgeConstraint::operator==(const EdgeConstraint& other) const -> bool
{
  return from == other.from && to == other.to && t == other.t;
}

SafeIntervalTable::SafeIntervalTable(const Instance& instance_)
    : edge_constraint_table(instance_.get_num_cells()), instance(instance_), unlimited_safe_intervals(instance_.get_map_data().free_cells)
{
  const int num_cells = instance.get_map_data().width * instance.get_map_data().height;

  // initialize the safe interval vectors TODO free cells dont need safe interval
  safe_intervals = std::vector<std::list<TimeInterval>>(num_cells, std::list<TimeInterval>(1, TimeInterval(0, INT_MAX)));
}

void SafeIntervalTable::add_constraint(const TimePoint& timepoint)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");

  // update unlimited safe interval number
  if (timepoint.interval.t_max == INT_MAX)
  {
    unlimited_safe_intervals--;
    assertm(unlimited_safe_intervals >= 0, "Invalid unlimited safe interval count.");
  }
  else
  {
    // update latest constraint end
    latest_constraint_end = std::max(latest_constraint_end, timepoint.interval.t_max);
  }

  std::list<TimeInterval>& cur_TI_list = safe_intervals[timepoint.location];

  // modify intervals that have some intersections with the time range
  auto it = cur_TI_list.begin();
  while (it != cur_TI_list.end())
  {
    // skip irrelevant time intervals
    if (it->t_max < timepoint.interval.t_min)
    {
      it++;
      continue;
    }

    // check that there are still intervals that could be reduced
    assertm(it->t_min <= timepoint.interval.t_max, "Can not add an overlapping constraint.");

    // shorten intervals starting before the range
    if (it->t_min < timepoint.interval.t_min)
    {
      assertm(it->t_max >= timepoint.interval.t_max, "Can not add an overlapping constraint.");
      const int new_tmax = timepoint.interval.t_min - 1;

      // create new interval if there is still some time in the safe interval after the constraint
      if (it->t_max > timepoint.interval.t_max)
      {
        cur_TI_list.insert(std::next(it, 1), TimeInterval(timepoint.interval.t_max + 1, it->t_max));
        it->t_max = new_tmax;
        break;
      }
      const int tmax = it->t_max;

      it->t_max = new_tmax;

      // check whether there might be any other affected intervals
      if (tmax == timepoint.interval.t_max)
      {
        break;
      }
      it++;
    }
    // shorten intervals ending after the range
    else if (it->t_max > timepoint.interval.t_max)
    {
      assertm(it->t_min == timepoint.interval.t_min, "Can not add an overlapping constraint.");
      it->t_min = timepoint.interval.t_max + 1;
      break;
    }
    // delete intervals that are fully overlapped by the constraint
    else
    {
      const int tmax = it->t_max;
      it             = cur_TI_list.erase(it);

      // check whether there can be any other affected ontervals
      if (tmax == timepoint.interval.t_max)
      {
        break;
      }
    }
  }
}


void SafeIntervalTable::remove_constraint(const TimePoint& timepoint)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");

  // update unlimited safe interval number
  if (timepoint.interval.t_max == INT_MAX)
  {
    unlimited_safe_intervals++;
    assertm(unlimited_safe_intervals <= instance.get_map_data().free_cells, "Invalid unlimited safe interval count.");

    // update latest constraint end TODO (done in remove constraints)
  }

  std::list<TimeInterval>& cur_TI_list = safe_intervals[timepoint.location];

  // check whether there are any safe intervals
  if (cur_TI_list.empty())
  {
    cur_TI_list.push_back(timepoint.interval);
    return;
  }

  // firstly handle constraints after the last safe interval (there is no safe interval after the constraint)
  assertm(timepoint.interval.t_min != cur_TI_list.back().t_max, "Constraint start overlaps with last safe interval end.");

  if (timepoint.interval.t_min > cur_TI_list.back().t_max)
  {
    // if the last safe interval precedes directly, extend it
    if (cur_TI_list.back().t_max == timepoint.interval.t_min - 1)
    {
      cur_TI_list.back().t_max = timepoint.interval.t_max;
      return;
    }
    // otherwise add a new safe interval
    cur_TI_list.push_back(timepoint.interval);
    return;
  }

  // find interval after the constraint
  auto it = cur_TI_list.begin();
  while (it != cur_TI_list.end())
  {
    assertm(!overlap(*it, timepoint.interval), "Constraint interval can not have any overlap with safe interval.");

    // find the interval after the constraint
    if (it->t_min > timepoint.interval.t_max)
    {
      // check whether there was an interval before the constraint
      if (it != cur_TI_list.begin())
      {
        // check whether the previous timeinterval can be extended
        const auto prev = std::prev(it, 1);
        if (prev->t_max == timepoint.interval.t_min - 1)
        {
          // check whether prev should be merged with next interval
          if (timepoint.interval.t_max != INT_MAX && it->t_min == timepoint.interval.t_max + 1)
          {
            prev->t_max = it->t_max;
            it          = cur_TI_list.erase(it);
          }
          else  // extend prev
          {
            prev->t_max = timepoint.interval.t_max;
          }
          return;
        }
      }

      // previous can not be extended, check whether next can be extended
      if (timepoint.interval.t_max != INT_MAX && it->t_min == timepoint.interval.t_max + 1)
      {
        it->t_min = timepoint.interval.t_min;
        return;
      }

      // no interval can be extended, therefore insert a new interval
      cur_TI_list.insert(it, timepoint.interval);
      return;
    }
    it++;
  }
  throw std::runtime_error("Did not remove any constraint.");
}

void SafeIntervalTable::add_constraints(const TimePointPath& path)
{
  for (int i = 0; i < (int)path.size(); i++)
  {
    // add the point constraint
    add_constraint(path[i]);

    // add the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      // the time when the edge is used is the start time of the later interval
      edge_constraint_table.add(path[i - 1].location, path[i].location, path[i].interval.t_min);
    }
  }
}

void SafeIntervalTable::remove_constraints(const TimePointPath& path)
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

  // update the latest constraint estimate
  update_latest_constraint_end_estimate();
}

auto SafeIntervalTable::get_first_safe_interval(int location) const -> std::list<TimeInterval>::const_iterator
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(location), "Invalid location.");

  const std::list<TimeInterval>& cur_TI_list = safe_intervals[location];

  // there is no safe interval
  if (cur_TI_list.empty())
  {
    return cur_TI_list.cend();
  }

  return cur_TI_list.cbegin();
}

auto SafeIntervalTable::get_safe_intervals(int location, const TimeInterval& time_interval) const
    -> std::pair<std::list<TimeInterval>::const_iterator, std::list<TimeInterval>::const_iterator>
{
  // this implementation does not check edge collisions, so they must be handled elsewhere.
  assertm(instance.get_map_data().is_in(location), "Invalid location.");
  assertm(location < (int)safe_intervals.size(), "Safe intervals not precomputed for this location.");

  // iterate over the list to find the first and last safe interval
  auto start = safe_intervals[location].cend();
  auto end   = safe_intervals[location].cend();
  for (auto it = safe_intervals[location].cbegin(); it != safe_intervals[location].cend(); it++)
  {
    // std::cout << "Interval: " << it->t_min << " " << it->t_max << std::endl;
    // check whether the interval is inside the given time range
    if (it->t_max >= time_interval.t_min && it->t_min <= time_interval.t_max)
    {
      // remember the first interval in the range
      if (start == safe_intervals[location].end())
      {
        start = it;
      }
    }
    // if the interval is not inside the time range, check whether already found some intervals
    else if (start != safe_intervals[location].cend())
    {
      // if already found some intervals, return the found safe intervals
      end = it;
      return std::make_pair(start, end);
    }
  }
  return std::make_pair(start, end);
}

void SafeIntervalTable::update_latest_constraint_end_estimate()
{
  latest_constraint_end = 0;
  for (const auto& safe_interval_list : safe_intervals)
  {
    const auto last = safe_interval_list.back();

    // safe interval reaches the end
    if (last.t_max == INT_MAX)
    {
      latest_constraint_end = std::max(latest_constraint_end, last.t_min - 1);
    }
  }
}

[[nodiscard]] auto SafeIntervalTable::get_max_path_len_estimate() const -> int
{
  if (unlimited_safe_intervals == 0)
  {
    return INT_MAX;
  }
  // check for overflow
  if (latest_constraint_end <= INT_MAX - unlimited_safe_intervals)
  {
    return latest_constraint_end + unlimited_safe_intervals;
  }
  return INT_MAX;
}


auto SafeIntervalTable::get_min_reach_time(int goal) const -> int
{
  return safe_intervals[goal].back().t_min;
}

EdgeConstraintTable::EdgeConstraintTable(int num_locations)
{
  // initialize the edge constraints
  edge_constraints.resize(NUM_DIRECTIONS);
  for (int i = 0; i < NUM_DIRECTIONS; i++)
  {
    edge_constraints[i].resize(num_locations);
  }
}

void EdgeConstraintTable::add(int loc1, int loc2, int time)
{
  assertm(loc1 != loc2, "Invalid edge constraint.");
  const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;
  assertm(dir >= 0 && dir <= NUM_DIRECTIONS, "Invalid direction.");
  // assertm(edge_constraints[dir][loc2].find(time) == edge_constraints[dir][loc2].end(), "Edge constraint already exists.");
  assertm(std::find(edge_constraints[dir][loc2].begin(), edge_constraints[dir][loc2].end(), time) == edge_constraints[dir][loc2].end(),
          "Edge constraint already exists.");
  // edge_constraints[dir][loc2].insert(time);     // unordered set implementation
  edge_constraints[dir][loc2].push_back(time);  // vector implementation
}

void EdgeConstraintTable::remove(int loc1, int loc2, int time)
{
  const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;
  assertm(dir >= 0 && dir <= NUM_DIRECTIONS, "Invalid direction.");

// int erased = edge_constraints[dir][loc2].erase(time);
// assertm(erased == 1, "Trying to erase an edge constraint that is not present.");
#ifndef NDEBUG
  int constraints_before = edge_constraints[dir][loc2].size();
  // std::cout << "Constraints before: " << constraints_before << std::endl;
#endif
  edge_constraints[dir][loc2].erase(std::remove(edge_constraints[dir][loc2].begin(), edge_constraints[dir][loc2].end(), time),
                                    edge_constraints[dir][loc2].end());
  // std::cout << "Constraints after: " << edge_constraints[dir][loc2].size() << std::endl;
  assertm(constraints_before - edge_constraints[dir][loc2].size() == 1, "Should erase exactly one edge constraint.");
}

