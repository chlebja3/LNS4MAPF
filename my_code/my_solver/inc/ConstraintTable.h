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
  int from;
  int to;
  int t;

  EdgeCollision(const int from, const int to, const int t);

  bool operator==(const EdgeCollision& other) const;

  struct hash_edge_collision
  {
    size_t operator()(const EdgeCollision& c) const;
  };
};

class ConstraintTable
{
public:
  ConstraintTable(const Instance& instance);

  void                      add_constraint(const TimePoint& timepoint);
  void                      add_constraints(const TimePointPath& path);
  void                      remove_constraint(const TimePoint& timepoint);
  void                      remove_constraints(const TimePointPath& path);
  TimeInterval              get_first_safe_interval(const int position);
  TimeInterval              get_safe_interval(const int position, const int from, const int time);
  std::vector<TimeInterval> get_safe_intervals(const int position, const int from, const TimeInterval& time_interval);
  int                       get_last_dynamic_constraint(void);

private:
  std::vector<std::list<TimeInterval>>                                  constraints;       // constraints for each position
  std::unordered_set<EdgeCollision, EdgeCollision::hash_edge_collision> edge_constraints;  // constraints for edges
  const Instance&                                                       instance;
};
