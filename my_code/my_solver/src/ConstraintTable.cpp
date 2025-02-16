/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "ConstraintTable.h"

#include <limits.h>

#include <iostream>

ConstraintTable::ConstraintTable(std::shared_ptr<Instance> instance) : instance(instance)
{
  std::cout << "Initialized Constraint table." << std::endl;
};


void ConstraintTable::add_constraint(const TimePoint& timepoint)
{
  std::cout << "Trying to add a constraint.";
  // check the position is valid
  assertm(instance->get_map_data().is_in(timepoint.position), "Invalid position.");

  // find the index of the position
  int idx = instance->get_map_data().position_to_index(timepoint.position);

  // check, that the index is valid
  assertm(idx >= 0 && idx < instance->get_map_data().width * instance->get_map_data().height, "Invalid index.");

  std::list<TimeInterval>& cur_TI_list = constraints[idx];

  // insert the time interval to the given list
  if (cur_TI_list.empty())
  {
    // insert to an empty list
    cur_TI_list.push_back(timepoint.interval);
  }
  else
  {
    bool inserted = false;
    // find where to insert the interval
    for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
    {

      // insert before the first interval that starts later
      if (it->t_min > timepoint.interval.t_min)
      {
        // check that there is no overlap
        assertm(timepoint.interval.t_max < it->t_min, "Cannot add overlapping constraints.");
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
  // check the position is valid
  assertm(instance->get_map_data().is_in(timepoint.position), "Invalid position.");

  // find the index of the position
  int idx = instance->get_map_data().position_to_index(timepoint.position);

  // check, that the index is valid
  assertm(idx >= 0 && idx < instance->get_map_data().width * instance->get_map_data().height, "Invalid index.");

  std::list<TimeInterval>& cur_TI_list = constraints[idx];

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
  for (auto const& constraint : path) add_constraint(constraint);
}

void ConstraintTable::remove_constraints(const TimePointPath& path)
{
  for (auto const& constraint : path) remove_constraint(constraint);
}

TimeInterval ConstraintTable::get_first_safe_interval(const Point2d& position)
{
  // check the position is valid
  assertm(instance->get_map_data().is_in(position), "Invalid position.");

  // find the index of the position
  int idx = instance->get_map_data().position_to_index(position);

  // check, that the index is valid
  assertm(idx >= 0 && idx < instance->get_map_data().width * instance->get_map_data().height, "Invalid index.");

  std::list<TimeInterval>& cur_TI_list = constraints[idx];

  // if there are no constraints, return the whole interval
  if (cur_TI_list.empty()) return TimeInterval(0, INT_MAX);

  // check that time 0 is not obstructed
  assertm(cur_TI_list.begin()->t_min > 0, "Location obstructed from time 0.");

  // return the time until first constraint
  return TimeInterval(0, cur_TI_list.begin()->t_min - 1);
}

TimeInterval ConstraintTable::get_safe_interval(const Point2d& position, const Point2d& from, const int time)
{
  // check the position is valid
  assertm(instance->get_map_data().is_in(position), "Invalid position.");

  // find the index of the position
  int idx = instance->get_map_data().position_to_index(position);

  // check, that the index is valid
  assertm(idx >= 0 && idx < instance->get_map_data().width * instance->get_map_data().height, "Invalid index.");

  std::list<TimeInterval>& cur_TI_list = constraints[idx];

  // if there are no constraints, return the whole interval
  if (cur_TI_list.empty()) return TimeInterval(time, INT_MAX);

  // find the closest safe interval

  // in this varaible there will be stored the earliest unobstructed time after 'time'
  int earliest_unobstructed_time = time;

  for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
  {
    // there is no constraint in the requested time
    if (it->t_min > earliest_unobstructed_time)
    {
      return TimeInterval(earliest_unobstructed_time, it->t_min - 1);
    }

    // there is a constraint, increase the earliest unobstructed time estimate
    if (it->t_max >= earliest_unobstructed_time)
    {
      earliest_unobstructed_time = it->t_max + 1;
    }
  }
  // TODO incorporate also edge obstacles to this

  // if there is no constraint after time, return the whole interval
  return TimeInterval(earliest_unobstructed_time, INT_MAX);
}

std::vector<TimeInterval> ConstraintTable::get_safe_intervals(const Point2d& position, const Point2d& from,
                                                              const TimeInterval& time_interval)
{
  // check the position is valid
  assertm(instance->get_map_data().is_in(position), "Invalid position.");

  // find the index of the position
  int idx = instance->get_map_data().position_to_index(position);

  // check, that the index is valid
  assertm(idx >= 0 && idx < instance->get_map_data().width * instance->get_map_data().height, "Invalid index.");

  std::list<TimeInterval>& cur_TI_list = constraints[idx];

  // if there are no constraints, return the whole interval
  if (cur_TI_list.empty()) return {TimeInterval(time_interval.t_min, INT_MAX)};

  // find the closest safe interval

  // in this varaible there will be stored the earliest unobstructed time after 'time'
  int earliest_unobstructed_time = time_interval.t_min;

  // in this vector, the possible safe intervals will be stored
  std::vector<TimeInterval> intervals;

  for (auto it = cur_TI_list.cbegin(); it != cur_TI_list.cend(); it++)
  {
    // there is some time between earliest unobstracted and next obstacle
    if (it->t_min > earliest_unobstructed_time)
    {
      // add the interval to the safe intervals
      intervals.emplace_back(earliest_unobstructed_time, it->t_min - 1);

      // check whether there might be any remaining safe interval in the given time interval
      if (it->t_max + 1 > time_interval.t_max) return intervals;

      // shift the earliest unobstructed time and continue with the next interval
      earliest_unobstructed_time = it->t_max + 1;
      continue;
    }

    // there is a constraint, increase the earliest unobstructed time estimate
    if (it->t_max >= earliest_unobstructed_time)
    {
      earliest_unobstructed_time = it->t_max + 1;
    }

    // check whether there might be any remaining safe interval in the given time interval
    if (earliest_unobstructed_time > time_interval.t_max) return intervals;
  }
  // TODO incorporate also edge obstacles to this

  // if there is no constraint after the time_interval end, add the interval from earliest_unobstructed_time to infinity.
  intervals.emplace_back(earliest_unobstructed_time, INT_MAX);
  return intervals;
}
