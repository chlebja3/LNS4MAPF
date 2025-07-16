/**
 * @file
 * @brief Contains the Constraint table and related classes.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 14-02-2025
 */

#pragma once

#include <climits>
#include <iostream>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Instance.h"
#include "utils.h"

/**
 * @brief A class representing a constraint table, which stores all dynamic constraints.
 */
class ConstraintTable
{
public:
  /**
   * @brief Constructs a constraint table for the given instance.
   *
   * @param instance_ The instance of the problem.
   */
  explicit ConstraintTable(const Instance& instance_);

  /**
   * @brief Adds a constraint to the constraint table for the given timepoint and agent number.
   *
   * @param timepoint The timepoint to add the constraint for.
   * @param agent_num The number of the agent to add the constraint for.
   */
  void add_constraint(const TimePoint& timepoint, int agent_num);

  auto insert_constraint(const TimePoint& timepoint, int agent_num, TimeIntervalNode_it prev, TimeIntervalNode_it next)
      -> TimeIntervalNode_it;

  auto remove_constraint(const TimePoint& timepoint, int agent_num)
      -> std::pair<TimeIntervalNode_it, TimeIntervalNode_it>;

  auto remove_constraint_after(const TimePoint& timepoint, int agent_num, TimeIntervalNode_it start)
      -> std::pair<TimeIntervalNode_it, TimeIntervalNode_it>;

  void delete_constraint(int location, TimeIntervalNode_it node);

  [[nodiscard]] auto get_blocking_agent(int loc, int time) const -> int;

  /**
   * @brief Finds agents that block a certaion location at a certain time and later.
   *
   * @param location The location to check for blocking agents.
   * @param time_min The minimum time to check for blocking agents.
   *
   * @return  A vector of integers representing the agents that block the location sorted in
   * descending order from the latest to the earliest.
   */
  [[nodiscard]] auto get_blocking_agents(int location, int time_min) const -> std::vector<int>;

  /**
   * @brief Returns agents, that enter a given location, along with how many they enter it.
   *
   * @param location The location to check for agents.
   *
   * @return A map of integers representing the agents that enter the location and how many times
   * they enter it.
   */
  [[nodiscard]] auto get_agents_counts(int location) const -> const std::unordered_map<int, int>&
  {
    assertm(instance.get_map_data().is_in(location), "Invalid location.");
    return agents_counts[instance.location_to_free_location(location)];
  }

  /**
   * @brief Returns agents, that enter a given free location, along with how many they enter it.
   *
   * @param free_location The free location to check for agents.
   *
   * @return A map of integers representing the agents that enter the free location and how many
   * times they enter it.
   */
  [[nodiscard]] auto get_agents_counts_free(int free_location) const -> const std::unordered_map<int, int>&
  {
    assertm(instance.get_map_data().is_in(instance.free_location_to_location(free_location)), "Invalid location.");
    return agents_counts[free_location];
  }

  void increase_agents_counts(int free_location, int agent_num)
  {
    // std::cout << "free location: " << free_location << ", adding agent: " << agent_num << std::endl;
    if (agents_counts[free_location].find(agent_num) == agents_counts[free_location].end())
    {
      agents_counts[free_location][agent_num] = 1;
    }
    else
    {
      // else increase the count
      agents_counts[free_location][agent_num] += 1;
    }
  }

  void decrease_agents_counts(int free_location, int agent_num)
  {
    // std::cout << "free location: " << free_location << ", removing agent: " << agent_num << std::endl;
    assertm(agents_counts[free_location].find(agent_num) != agents_counts[free_location].end(),
            "Removing agent that is not in the agents counts");
    agents_counts[free_location][agent_num] -= 1;
    // if the count drops to 0, erase the agent number from keys
    if (agents_counts[free_location][agent_num] == 0)
    {
      agents_counts[free_location].erase(agent_num);
    }
  }

  /**
   * @brief Returns the last constraint start time for a given location.
   *
   * @param location The location to check for the last constraint start time.
   *
   * @return The last constraint start time for the given location.
   */
  [[nodiscard]] auto get_last_constraint_start(int location) const -> int
  {
    return constraints[instance.location_to_free_location(location)].back().interval.t_min;
  }

  void reset()
  {
    const int num_free_cells = instance.get_num_free_cells();
    constraints.clear();
    constraints = std::vector<std::list<TimeIntervalNode>>(num_free_cells, std::list<TimeIntervalNode>());
    agents_counts = std::vector<std::unordered_map<int, int>>(num_free_cells, std::unordered_map<int, int>());
  }

  [[nodiscard]] auto get_first_constraint(int location) -> TimeIntervalNode_it;

private:
  std::vector<std::list<TimeIntervalNode>> constraints;    /**< constraints for each location */
  const Instance& instance;                                /**< The instance of the problem. */
  std::vector<std::unordered_map<int, int>> agents_counts; /**< unordered map of times the agents visit each location */
};
