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
#include "magic_enum/magic_enum.hpp"
#include "utils.h"

constexpr int NUM_DIRECTIONS = magic_enum::enum_count<Direction>() - 1;  // dont consider NONE direction

class EdgeConstraint
{
public:
  int from;
  int to;
  int t;

  EdgeConstraint(int from_, int to_, int t_) : from(from_), to(to_), t(t_)
  {
  }

  auto operator==(const EdgeConstraint& other) const -> bool;
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
    const int dir = static_cast<int>(find_direction(loc1, loc2)) - 1;  // dont consider NONE direction
    assertm(dir >= 0 && dir < NUM_DIRECTIONS, "Invalid direction.");

    // return edge_constraints[dir][loc2].find(time) != edge_constraints[dir][loc2].end(); // unordered set implementation
    return std::find(edge_constraints[dir][loc2].begin(), edge_constraints[dir][loc2].end(), time) !=
           edge_constraints[dir][loc2].end();  // vector implementation
  }

private:
  // std::unordered_set<EdgeConstraint, EdgeConstraint::hash_edge_collision> edge_constraints;  // constraints for edges
  // std::vector<std::vector<std::unordered_set<int>>> edge_constraints; // unordered set implementation
  std::vector<std::vector<std::vector<int>>> edge_constraints;  // vector implementation
};

class SafeIntervalTable
{
public:
  explicit SafeIntervalTable(const Instance& instance_);
  EdgeConstraintTable edge_constraint_table;

  // new implementation
  void               add_constraint(const TimePoint& timepoint);
  void               add_constraints(const TimePointPath& path);
  void               remove_constraint(const TimePoint& timepoint);
  void               remove_constraints(const TimePointPath& path);
  [[nodiscard]] auto get_safe_intervals(int location, const TimeInterval& time_interval) const
      -> std::pair<std::list<TimeInterval>::const_iterator, std::list<TimeInterval>::const_iterator>;
  [[nodiscard]] auto get_first_safe_interval(int location) const -> std::list<TimeInterval>::const_iterator;
  [[nodiscard]] auto get_max_path_len_estimate() const -> int;
  void               update_latest_constraint_end_estimate();
  [[nodiscard]] auto get_min_reach_time(int goal) const -> int;
  [[nodiscard]] auto get_last_safe_interval(int location) const -> const TimeInterval&
  {
    return safe_intervals[location].back();
  }

private:
  std::vector<std::list<TimeInterval>> safe_intervals;  // safe intervals for each position
  const Instance&                      instance;
  int                                  max_path_len_estimate = INT_MAX;
  int                                  unlimited_safe_intervals;
  int                                  latest_constraint_end = 0;
};
