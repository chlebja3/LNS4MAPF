/*
 * Author: Jan Chleboun
 * Date: 14-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <climits>
#include <iostream>
#include <list>
#include <memory>
#include <unordered_set>
#include <vector>

#include "Instance.h"
#include "utils.h"

#define NUM_DIRECTIONS 4

class EdgeConstraint
{
public:
  int from;
  int to;
  int t;

  EdgeConstraint(int from_, int to_, int t_);

  auto operator==(const EdgeConstraint& other) const -> bool;

  struct hash_edge_collision
  {
    auto operator()(const EdgeConstraint& c) const -> size_t;
  };
};

class EdgeConstraintTable
{
public:
  explicit EdgeConstraintTable(int num_locations);
  void add(int loc1, int loc2, int time);
  void remove(int loc1, int loc2, int time);
  // auto get(int loc1, int loc2, int time) const -> bool;

  [[nodiscard]] inline auto get(int loc1, int loc2, int time) const -> bool
  {
    const int dir = find_direction(loc1, loc2) - 1;
    assertm(dir >= 0 && dir <= NUM_DIRECTIONS, "Invalid direction.");

    return edge_constraints[dir][loc2].find(time) != edge_constraints[loc1][loc2].end();
  }

private:
  // std::unordered_set<EdgeConstraint, EdgeConstraint::hash_edge_collision> edge_constraints;  // constraints for edges
  std::vector<std::vector<std::unordered_set<int>>> edge_constraints;
};

class ConstraintTable
{
public:
  explicit ConstraintTable(const Instance& instance_);
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
  void               update_latest_constraint_end_estimate();
  auto               get_min_reach_time(int goal) const -> int;

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
  std::vector<std::list<TimeInterval>> constraints;     // constraints for each position
  std::vector<std::list<TimeInterval>> safe_intervals;  // safe intervals for each position
  const Instance&                      instance;
  int                                  max_path_len_estimate = INT_MAX;
  int                                  unlimited_safe_intervals;
  int                                  latest_constraint_end = 0;
};
