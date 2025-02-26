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

class EdgeConstraint
{
public:
  int from;
  int to;
  int t;

  EdgeConstraint(int from, int to, int t);

  auto operator==(const EdgeConstraint& other) const -> bool;

  struct hash_edge_collision
  {
    auto operator()(const EdgeConstraint& c) const -> size_t;
  };
};

class EdgeConstraintTable
{
public:
  void add(int loc1, int loc2, int time);
  void remove(int loc1, int loc2, int time);
  auto get(int loc1, int loc2, int time) const -> bool;

private:
  std::unordered_set<EdgeConstraint, EdgeConstraint::hash_edge_collision> edge_constraints;  // constraints for edges
};

class ConstraintTable
{
public:
  explicit ConstraintTable(const Instance& instance);
  EdgeConstraintTable edge_constraint_table;

  // new implementation
  void               add_constraint(const TimePoint& timepoint);
  void               add_constraints(const TimePointPath& path);
  void               remove_constraint(const TimePoint& timepoint);
  void               remove_constraints(const TimePointPath& path);
  [[nodiscard]] auto get_safe_intervals(int location, const TimeInterval& time_interval) const
      -> std::pair<std::list<TimeInterval>::const_iterator, std::list<TimeInterval>::const_iterator>;
  [[nodiscard]] auto get_first_safe_interval(int location) const -> const std::list<TimeInterval>::const_iterator;
  [[nodiscard]] auto get_max_path_len_estimate() const -> int;
  void               update_max_path_len_estimate();

  // old implementation
  void               add_constraint_old(const TimePoint& timepoint);
  void               add_constraints_old(const TimePointPath& path);
  void               remove_constraint_old(const TimePoint& timepoint);
  void               remove_constraints_old(const TimePointPath& path);
  [[nodiscard]] auto get_first_safe_interval_old(int location) const -> TimeInterval;
  [[nodiscard]] auto get_safe_interval_old(int location, int from, int time) const -> TimeInterval;
  [[nodiscard]] auto get_safe_intervals_old(int location, int from, const TimeInterval& time_interval) const -> std::vector<TimeInterval>;
  [[nodiscard]] auto get_last_dynamic_constraint_old() const -> int;

private:
  std::vector<std::list<TimeInterval>> constraints;     // constraints for each position
  std::vector<std::list<TimeInterval>> safe_intervals;  // safe intervals for each position
  const Instance&                      instance;
  int                                  max_path_len_estimate;
};
