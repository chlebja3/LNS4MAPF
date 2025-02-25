/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "ConstraintTable.h"

#include <limits.h>

#include <boost/functional/hash.hpp>
#include <iostream>

EdgeCollision::EdgeCollision(const int from, const int to, const int t) : from(from), to(to), t(t){};


bool EdgeCollision::operator==(const EdgeCollision& other) const
{
  return from == other.from && to == other.to && t == other.t;
}

size_t EdgeCollision::hash_edge_collision::operator()(const EdgeCollision& c) const
{
  std::size_t seed = 0;
  // combine hash of from, to and t
  boost::hash_combine(seed, c.from);
  boost::hash_combine(seed, c.to);
  boost::hash_combine(seed, c.t);
  return seed;
}

ConstraintTable::ConstraintTable(const Instance& instance) : instance(instance)
{
  // initialize the constraint vectors TODO static obstacles dont need constraint vectors
  const int num_constraints = instance.get_map_data().width * instance.get_map_data().height;
  constraints               = std::vector<std::list<TimeInterval>>(num_constraints, std::list<TimeInterval>());
}


void ConstraintTable::add_constraint(const TimePoint& timepoint)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");

  std::list<TimeInterval>& cur_TI_list = constraints[timepoint.location];

  // insert the time interval to the given list
  if (cur_TI_list.empty())
  {
    // insert to an empty list
    cur_TI_list.push_back(timepoint.interval);
    return;
  }
  else
  {
    bool inserted = false;
    // find where to insert the interval
    for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
    {
      // check that the intervals do not have any overlap

      assertm(!overlap(*it, timepoint.interval), "Cannot add overlapping constraints.");

      // insert before the first interval that starts later
      if (it->t_min > timepoint.interval.t_min)
      {
        // check that there is no overlap
        // assertm(timepoint.interval.t_max < it->t_min, "Cannot add overlapping constraints.");
        cur_TI_list.insert(it, timepoint.interval);
        inserted = true;
        break;
      }
    }

    // if no interval starts later, insert to the back of the list
    if (!inserted) cur_TI_list.push_back(timepoint.interval);
  }
}

void ConstraintTable::remove_constraint(const TimePoint& timepoint)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(timepoint.location), "Invalid location.");

  std::list<TimeInterval>& cur_TI_list = constraints[timepoint.location];

  // check that the list is not empty
  assertm(!cur_TI_list.empty(), "Trying to remove interval from an empty list.");

  // remove the interval from the list
  for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
  {
    if (*it == timepoint.interval)
    {
      cur_TI_list.erase(it);
      return;
    }
  }
  // if the interval was not found, throw an error
  throw std::runtime_error("Trying to remove non-existing interval from the constraint table.");
}

void ConstraintTable::add_constraints(const TimePointPath& path)
{
  for (int i = 0; i < (int)path.size(); i++)
  {
    // add the point constraint
    add_constraint(path[i]);

    // add the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      // the time when the edge is used is the start time of the later interval
      const EdgeCollision c(path[i - 1].location, path[i].location, path[i].interval.t_min);
      edge_constraints.insert(c);
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
      const EdgeCollision c(path[i - 1].location, path[i].location, path[i].interval.t_min);
      const int           erased = edge_constraints.erase(c);
      assertm(erased == 1, "Trying to erase an edge constraint that is not present.");
    }
  }
}

TimeInterval ConstraintTable::get_first_safe_interval(const int location)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(location), "Invalid location.");

  std::list<TimeInterval>& cur_TI_list = constraints[location];

  // if there are no constraints, return the whole interval
  if (cur_TI_list.empty()) return TimeInterval(0, INT_MAX);

  // check that time 0 is not obstructed
  assertm(cur_TI_list.begin()->t_min > 0, "Location obstructed from time 0.");

  // return the time until first constraint
  return TimeInterval(0, cur_TI_list.begin()->t_min - 1);
}

// not used
TimeInterval ConstraintTable::get_safe_interval(const int location, const int from, const int time)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(location), "Invalid location.");

  std::list<TimeInterval>& cur_TI_list = constraints[location];

  // if there are no constraints, return the whole interval
  if (cur_TI_list.empty()) return TimeInterval(time, INT_MAX);

  // find the closest safe interval

  // in this varaible there will be stored the earliest unobstructed time after 'time'
  int earliest_unobstructed_time = time;

  for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
  {
    // there is no location constraint in the requested time
    if (earliest_unobstructed_time < it->t_min)
    {
      // edge collision might happen only if the cell was occupied in last time step
      if (it == cur_TI_list.cbegin() || std::prev(it)->t_max != earliest_unobstructed_time - 1)
        return TimeInterval(earliest_unobstructed_time, it->t_min - 1);

      // if there is no edge collision, return the interval
      const EdgeCollision c(location, from, earliest_unobstructed_time);
      if (edge_constraints.find(c) == edge_constraints.end()) return TimeInterval(earliest_unobstructed_time, it->t_min - 1);

      // if there is an edge collision, increase earliest unobstructed time by one and then check, whether there is time before the next
      // point constraint
      earliest_unobstructed_time++;
      if (earliest_unobstructed_time < it->t_min) return TimeInterval(earliest_unobstructed_time, it->t_min - 1);
    }

    // there is a point constraint, increase the earliest unobstructed time estimate to the end of the constraint + 1
    if (it->t_max >= earliest_unobstructed_time)
    {
      // check whether the constraint is forever
      if (it->t_max == INT_MAX) return TimeInterval(-1, -1);
      earliest_unobstructed_time = it->t_max + 1;
    }
  }

  // check that there is no edge collision after the end of the last interval
  if (cur_TI_list.back().t_max + 1 == earliest_unobstructed_time)
  {
    // if there is an edge collision, increase the earliest unobstructed time by one
    const EdgeCollision c(location, from, earliest_unobstructed_time);
    if (edge_constraints.find(c) != edge_constraints.end()) earliest_unobstructed_time++;
  }


  // if there is no constraint after time, return the whole interval
  return TimeInterval(earliest_unobstructed_time, INT_MAX);
}

std::vector<TimeInterval> ConstraintTable::get_safe_intervals(const int location, const int from, const TimeInterval& time_interval)
{
  // check the location is valid
  assertm(instance.get_map_data().is_in(location), "Invalid location.");

  // check the time interval is valid
  assertm(time_interval.t_min >= 0 && time_interval.t_max >= time_interval.t_min, "Invalid time interval.");

  std::list<TimeInterval>& cur_TI_list = constraints[location];

  // if there are no constraints, return the whole interval
  if (cur_TI_list.empty()) return {TimeInterval(time_interval.t_min, INT_MAX)};

  // find the closest safe interval

  // in this varaible there will be stored the earliest unobstructed time after 'time'
  int earliest_unobstructed_time = time_interval.t_min;

  // in this vector, the possible safe intervals will be stored
  std::vector<TimeInterval> intervals;

  for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
  {
    // there is some time between earliest unobstracted time and next obstacle
    if (earliest_unobstructed_time < it->t_min)
    {
      // edge collision might happen only if the cell was occupied in last time step
      if (it == cur_TI_list.cbegin() || std::prev(it)->t_max != earliest_unobstructed_time - 1)
      {
        // add the interval to the safe intervals
        intervals.emplace_back(earliest_unobstructed_time, it->t_min - 1);
      }
      else
      {
        // check for edge collision
        const EdgeCollision c(location, from, earliest_unobstructed_time);
        if (edge_constraints.find(c) == edge_constraints.end())  // if there is no edge collision, add the interval to the safe intervals
          intervals.emplace_back(earliest_unobstructed_time, it->t_min - 1);
        else
        {
          // if there is an edge collision, then the location the agent is coming from is gonna be obstructed in the next time step, so the
          // interval should end
          assertm(earliest_unobstructed_time == time_interval.t_max, "Invalid time interval, the from location will be in edge collision.");
          return intervals;
        }
      }

      // check whether there might be any remaining safe interval in the given time interval
      if (it->t_max == INT_MAX || it->t_max + 1 > time_interval.t_max) return intervals;

      // shift the earliest unobstructed time and continue with the next interval
      earliest_unobstructed_time = it->t_max + 1;
      continue;
    }

    if (it->t_max >= earliest_unobstructed_time)
    {
      // if the constraint obstructs the location forever, there can not be any more safe intervals
      if (it->t_max == INT_MAX) return intervals;

      // there is a constraint, increase the earliest unobstructed time estimate
      earliest_unobstructed_time = it->t_max + 1;
    }

    // check whether there might be any remaining safe interval in the given time interval
    if (earliest_unobstructed_time > time_interval.t_max) return intervals;
  }

  // check that there is no edge collision after the end of the last interval
  if (cur_TI_list.back().t_max + 1 == earliest_unobstructed_time)
  {
    // if there is an edge collision, increase the earliest unobstructed time by one
    const EdgeCollision c(location, from, earliest_unobstructed_time);
    if (edge_constraints.find(c) != edge_constraints.end())
    {
      // check whether the interval is valid
      assertm(earliest_unobstructed_time == time_interval.t_max, "Invalid time interval, the from location will be in edge collision.");

      // there can not be any more valid safe intervals after edge conflict
      return intervals;
    }
  }

  // if there is no constraint after the time_interval end, add the interval from earliest_unobstructed_time to infinity.
  intervals.emplace_back(earliest_unobstructed_time, INT_MAX);
  return intervals;
}

int ConstraintTable::get_last_dynamic_constraint(void)
{
  int last_dynamic_constraint = 0;
  for (int i = 0; i < (int)constraints.size(); i++)
  {
    if (constraints[i].size() > 0) last_dynamic_constraint = std::max(last_dynamic_constraint, constraints[i].back().t_min);
  }
  return last_dynamic_constraint;
}
