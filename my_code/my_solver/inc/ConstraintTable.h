/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <list>
#include <memory>
#include <vector>

#include "Instance.h"
#include "utils.h"

class ConstraintTable
{
public:
  ConstraintTable(std::shared_ptr<Instance> instance);
  void                      add_constraint(const TimePoint& timepoint);
  void                      add_constraints(const TimePointPath& path);
  void                      remove_constraint(const TimePoint& timepoint);
  void                      remove_constraints(const TimePointPath& path);
  TimeInterval              get_safe_interval(const Point2d& position, const int time);
  std::vector<TimeInterval> get_safe_intervals(const Point2d& position, const TimeInterval& time_interval);

private:
  // structure to store the constraints for each position
  std::vector<std::list<TimeInterval>> constraints;
  std::shared_ptr<Instance>            instance;
};
