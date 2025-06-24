/**
 * @file
 * @brief Contains classes relevant to the SafeIntervalTable, which stores the safe intervals for SIPP.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 14-02-2025
 */

#pragma once
#include <climits>
#include <iostream>
#include <list>
#include <memory>
#include <unordered_set>
#include <vector>

#include "ConstraintTable.h"
#include "Instance.h"
#include "magic_enum/magic_enum.hpp"
#include "utils.h"

/**
 * @brief If this is defined, the solver is compiled with parallelized versions of the SafeIntervalTable.
 */
#define SIT_PARALLELIZATION

/**
 * @brief The SafeIntervalTable class stores the safe intervals for each location in the grid.
 */
class SafeIntervalTable
{
public:
  /**
   * @brief Constructs a SafeIntervalTable object.
   *
   * @param instance_ The instance of the problem.
   */
  explicit SafeIntervalTable(const Instance& instance_);

#ifdef SIT_PARALLELIZATION
  /**
   * @brief Adds constraints to the safe interval table in parallel.
   *
   * @param path The path to add constraints for.
   * @warning This function is not properly tested and has known issues.
   */
  void add_constraints_parallel(const TimePointPath& path);

  /**
   * @brief Removes constraints from the safe interval table in parallel.
   *
   * @param path The path to remove constraints for.
   * @warning This function is not properly tested and has known issues.
   */
  void remove_constraints_parallel(const TimePointPath& path);
#endif

  void add_constraint(const TimePoint& timepoint, int agent_num, ConstraintTable& constraint_table);

  void remove_constraint(const TimePoint& timepoint, int agent_num, ConstraintTable& constraint_table);


  /**
   * @brief Gets the safe intervals for a given location and a time interval.
   *
   * @param location The location to get the safe intervals for.
   * @param time_interval The time interal, the location can be entered. All returned intervals must have nonzero intersection with it.
   *
   * @return A pair of iterators representing the range of safe intervals for the given location and time interval.
   * @warning The returned safe intervals must be checked for edge collisions.
   */
  [[nodiscard]] auto get_safe_intervals(int location, const TimeInterval& time_interval) const
      -> std::pair<std::list<TimeIntervalNode>::const_iterator, std::list<TimeIntervalNode>::const_iterator>;

  /**
   * @brief Gets the first safe interval for a given location.
   *
   * @param location The location to get the first safe interval for.
   *
   * @return  An iterator to the first safe interval for the given location.
   */
  [[nodiscard]] auto get_first_safe_interval(int location) const -> std::list<TimeIntervalNode>::const_iterator;

  /**
   * @brief Get the minimal reach time of a location.
   *
   * @param goal The goal location to get the minimal reach time for.
   *
   * @return  The minimal reach time of the goal location.
   */
  [[nodiscard]] auto get_min_reach_time(int goal) const -> int
  {
    assertm(instance.get_map_data().is_in(goal), "Invalid goal location.");
    return safe_intervals[instance.location_to_free_location(goal)].back().interval.t_min;
  }

  /**
   * @brief Gets the last safe interval for a given location.
   *
   * @param location The location to get the last safe interval for.
   *
   * @return A reference to the last safe interval for the given location.
   */
  [[nodiscard]] auto get_last_safe_interval(int location) const -> const TimeInterval&
  {
    assertm(instance.get_map_data().is_in(location), "Invalid location");
    return safe_intervals[instance.location_to_free_location(location)].back().interval;
  }

  /**
   * @brief Resets the safe interval table.
   */
  void reset()
  {
    // reset safe intervals
    int num_free_cells = static_cast<int>(safe_intervals.size());
    safe_intervals     = std::vector<std::list<TimeIntervalNode>>(
        num_free_cells,
        std::list<TimeIntervalNode>(
            1, TimeIntervalNode(TimeInterval(0, INT_MAX), -1, TIME_INTERVAL_NODE_type::safe_interval, std::nullopt, std::nullopt)));


    // reset latest constraint end
    // latest_constraint_end         = 0;
    // latest_constraint_end_updated = true;

    // reset unlimited safe intervals
    // unlimited_safe_intervals = instance.get_map_data().get_num_free_cells();

    // reset max path len estimate
    // max_path_len_estimate = INT_MAX;
  }

private:
  std::vector<std::list<TimeIntervalNode>> safe_intervals; /**< safe intervals for each position */
  const Instance&                          instance;       /**< The instance of the problem. */
  // int                                  max_path_len_estimate = INT_MAX; /**< The estimate of the maximum path length. */
  // int unlimited_safe_intervals; /**< The number of unlimited safe intervals used for the max_path_len_estimate calculation. */
  // int  latest_constraint_end         = 0;    /**< The end of the latest constraint. */
  // bool latest_constraint_end_updated = true; /**< Indicates if the latest constraint end has been updated. */
};
