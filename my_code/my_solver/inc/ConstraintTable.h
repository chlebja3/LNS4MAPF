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

  EdgeCollision(int from, int to, int t);

  auto operator==(const EdgeCollision& other) const -> bool;

  struct hash_edge_collision
  {
    auto operator()(const EdgeCollision& c) const -> size_t;
  };
};

class ConstraintTable
{
public:
  explicit ConstraintTable(const Instance& instance);

  void add_constraint(const TimePoint& timepoint);
  void add_constraints(const TimePointPath& path);
  void add_constraint_safe_intervals(const TimePoint& timepoint);
  void remove_constraint(const TimePoint& timepoint);
  void remove_constraint_safe_intervals(const TimePoint& timepoint);
  void remove_constraints(const TimePointPath& path);
  auto get_first_safe_interval(int location) const -> TimeInterval;
  auto calculate_safe_interval(int location, int from, int time) const -> TimeInterval;
  auto calculate_safe_intervals(int location, int from, const TimeInterval& time_interval) const -> std::vector<TimeInterval>;
  auto get_safe_intervals(int location, int from, const TimeInterval& time_interval) const
      -> std::pair<std::list<TimeInterval>::const_iterator, std::list<TimeInterval>::const_iterator>;
  auto get_last_dynamic_constraint() const -> int;

private:
  std::vector<std::list<TimeInterval>>                                  constraints;       // constraints for each position
  std::vector<std::list<TimeInterval>>                                  safe_intervals;    // safe intervals for each position
  std::unordered_set<EdgeCollision, EdgeCollision::hash_edge_collision> edge_constraints;  // constraints for edges
  const Instance&                                                       instance;
};
