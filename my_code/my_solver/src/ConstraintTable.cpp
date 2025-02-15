/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "ConstraintTable.h"

ConstraintTable::ConstraintTable(std::shared_ptr<Instance> instance) : instance(instance){};


void ConstraintTable::add_constraint(const TimePoint& timepoint)
{
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
        assertm(timepoint.interval.t_max < it->t_min, "Cannot add overlapping constraints");
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

TimeInterval ConstraintTable::get_safe_interval(const Point2d& position, const int time)
{
}

std::vector<TimeInterval> get_safe_intervals(const Point2d& position, const TimeInterval& time_interval)
{
}
