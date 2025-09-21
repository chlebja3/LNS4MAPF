/**
 * @file
 * @brief Manages all constraints and safe intervals and edge constraints.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 21-06-2025
 */

#pragma once
#include "EdgeTable.h"
#include "SafeIntervalTable.h"

class ConstraintManager
{
public:
  explicit ConstraintManager(const Instance& instance_)
      : safe_interval_table(instance_),
        constraint_table(instance_),
        edge_table(instance_),
        instance(instance_)
#ifdef CM_PARALLELIZATION
        ,
        locks(instance_.get_num_free_cells())
#endif
  {
  }

  void add_trajectory(const TimePointPath& path, int agent_num);
  void remove_trajectory(const TimePointPath& path, int agent_num);
  void build_sequential(const std::vector<TimePointPath>& paths);

  void reset()
  {
    safe_interval_table.reset();
    constraint_table.reset();
    edge_table.reset();
  }

  // Constraint Table functionalities
  [[nodiscard]] auto get_blocking_agent(int from, int to, int time) const -> std::pair<int, int>;

  [[nodiscard]] auto get_blocking_agents(int location, int time_min) const -> std::vector<int>
  {
    return constraint_table.get_blocking_agents(location, time_min);
  }

  [[nodiscard]] auto get_agents_counts(int location) const -> const std::unordered_map<int, int>&
  {
    return constraint_table.get_agents_counts(location);
  }

  [[nodiscard]] auto get_all_intervals(int location, const TimeInterval& time_interval) const
      -> std::pair<std::list<TimeIntervalNode>::const_iterator, std::list<TimeIntervalNode>::const_iterator>;

  // Safe Interval Table functionalities
  [[nodiscard]] auto get_safe_intervals(int location, const TimeInterval& time_interval) const
      -> std::pair<std::list<TimeIntervalNode>::const_iterator, std::list<TimeIntervalNode>::const_iterator>
  {
    return safe_interval_table.get_safe_intervals(location, time_interval);
  }

  [[nodiscard]] auto get_first_safe_interval(int location) const -> TimeIntervalNode_const_it
  {
    return safe_interval_table.get_first_safe_interval(location);
  }

  [[nodiscard]] auto get_first_time_interval(int location) -> TimeIntervalNode_const_it;

  [[nodiscard]] auto get_min_reach_time(int goal) const -> int
  {
    return safe_interval_table.get_min_reach_time(goal);
  }

  [[nodiscard]] auto get_agents_counts_free(int free_location) const -> const std::unordered_map<int, int>&
  {
    return constraint_table.get_agents_counts_free(free_location);
  }

  [[nodiscard]] auto get_last_constraint_start(int location) const -> int
  {
    return constraint_table.get_last_constraint_start(location);
  }

  [[nodiscard]] auto get_max_path_len_estimate() const -> int
  {
    return INT_MAX;
  }

  // Edge Table functionalities
  [[nodiscard]] auto check_edge_collision(int from, int to, int time) const -> bool
  {
    // swap to and from, because we need the opposite edge from the edge table
    return edge_table.get(to, from, time) != -1;
  }

  [[nodiscard]] auto get_edge_collision(int from, int to, int time) const -> int
  {
    // swap to and from, because we need the opposite edge from the edge table
    return edge_table.get(to, from, time);
  }

  auto is_valid() -> bool;

  inline auto is_end_of_list(int location, std::list<TimeIntervalNode>::const_iterator it) const -> bool
  {
    return safe_interval_table.is_end_of_list(location, it);
  }

private:
  SafeIntervalTable safe_interval_table;
  ConstraintTable constraint_table;
  EdgeTable edge_table;
  const Instance& instance;
#ifdef CM_PARALLELIZATION
  omp_locks locks; /**< The locks used for parallelization. */
#endif
};
