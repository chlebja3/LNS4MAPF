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
namespace old
{
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
  void add(int loc1, int loc2, int time);
  void remove(int loc1, int loc2, int time);

  [[nodiscard]] inline auto get(int loc1, int loc2, int time) const -> bool
  {
    return edge_constraints.find(EdgeConstraint(loc1, loc2, time)) != edge_constraints.end();
  }

private:
  std::unordered_set<EdgeConstraint, EdgeConstraint::hash_edge_collision> edge_constraints;  // constraints for edges
};

class ConstraintTable
{
public:
  explicit ConstraintTable(const Instance& instance_);
  old::EdgeConstraintTable edge_constraint_table;

  void add_constraint(const TimePoint& timepoint);
  void add_constraints(const TimePointPath& path);
  void remove_constraint(const TimePoint& timepoint);
  void remove_constraints(const TimePointPath& path);
  auto get_first_safe_interval(int location) const -> TimeInterval;
  auto get_safe_interval(int location, int from, int time) const -> TimeInterval;
  auto get_safe_intervals(int location, int from, const TimeInterval& time_interval) const -> std::vector<TimeInterval>;
  auto get_last_dynamic_constraint() const -> int;

private:
  std::vector<std::list<TimeInterval>> constraints;     // constraints for each position
  std::vector<std::list<TimeInterval>> safe_intervals;  // safe intervals for each position
  const Instance&                      instance;
};
}  // namespace old
