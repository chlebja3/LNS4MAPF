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

  // new implementation
  void add_constraint(const TimePoint& timepoint);
  void add_constraints(const TimePointPath& path);
  void remove_constraint(const TimePoint& timepoint);
  void remove_constraints(const TimePointPath& path);
  auto get_safe_intervals(int location, const TimeInterval& time_interval) const
      -> std::pair<std::list<TimeInterval>::const_iterator, std::list<TimeInterval>::const_iterator>;
  auto get_first_safe_interval(int location) const -> const std::list<TimeInterval>::const_iterator;
  auto get_max_path_len_estimate() const -> int;
  void update_max_path_len_estimate();

  // old implementation
  void add_constraint_old(const TimePoint& timepoint);
  void add_constraints_old(const TimePointPath& path);
  void remove_constraint_old(const TimePoint& timepoint);
  void remove_constraints_old(const TimePointPath& path);
  auto get_first_safe_interval_old(int location) const -> TimeInterval;
  auto get_safe_interval_old(int location, int from, int time) const -> TimeInterval;
  auto get_safe_intervals_old(int location, int from, const TimeInterval& time_interval) const -> std::vector<TimeInterval>;
  auto get_last_dynamic_constraint_old() const -> int;

private:
  std::vector<std::list<TimeInterval>>                                  constraints;       // constraints for each position
  std::vector<std::list<TimeInterval>>                                  safe_intervals;    // safe intervals for each position
  std::unordered_set<EdgeCollision, EdgeCollision::hash_edge_collision> edge_constraints;  // constraints for edges
  const Instance&                                                       instance;
  int                                                                   max_path_len_estimate;
};
