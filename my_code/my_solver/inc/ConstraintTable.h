/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include "utils.h"

class ConstraintTable
{
public:
  TimeInterval              get_safe_interval(const Point2d& position, const int time);
  std::vector<TimeInterval> get_safe_intervals(const Point2d& position, const TimeInterval& time_interval);
};
