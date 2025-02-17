/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <iostream>
#include <list>
#include <memory>
#include <unordered_set>
#include <vector>

#include "Instance.h"
#include "utils.h"

class EdgeCollision
{
public:
  Point2d from;
  Point2d to;
  int     t;

  EdgeCollision(const Point2d& from, const Point2d& to, const int t);

  bool operator==(const EdgeCollision& other) const;

  struct hash_edge_collision
  {
    size_t operator()(const EdgeCollision& c) const;
  };
};

class ConstraintTable
{
public:
  ConstraintTable(std::shared_ptr<Instance> instance);

  void                      add_constraint(const TimePoint& timepoint);
  void                      add_constraints(const TimePointPath& path);
  void                      remove_constraint(const TimePoint& timepoint);
  void                      remove_constraints(const TimePointPath& path);
  TimeInterval              get_first_safe_interval(const Point2d& position);
  TimeInterval              get_safe_interval(const Point2d& position, const Point2d& from, const int time);
  std::vector<TimeInterval> get_safe_intervals(const Point2d& position, const Point2d& from, const TimeInterval& time_interval);
  int                       get_last_constraint_end(void);

private:
  std::vector<std::list<TimeInterval>>                                  constraints;       // constraints for each position
  std::unordered_set<EdgeCollision, EdgeCollision::hash_edge_collision> edge_constraints;  // constraints for edges
  std::shared_ptr<Instance>                                             instance;
};
