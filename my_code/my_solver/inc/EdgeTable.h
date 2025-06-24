/**
 * @file
 * @brief Manages all constraints and safe intervals and edge constraints.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 21-06-2025
 */

#pragma once
#include <magic_enum/magic_enum.hpp>

#include "Instance.h"

constexpr int NUM_DIRECTIONS = magic_enum::enum_count<Direction>() - 1;  // dont consider NONE direction

/**
 * @brief A class, which represents an edge constraint along with the number of agent, which causes it.
 * @deprecated This class is deprecated, as it is meant for EdgeTable implemented as a vector of hash tables.
 */
class EdgeConstraint
{
public:
  int from;      /**< The starting location of the edge. */
  int to;        /**< The ending location of the edge. */
  int t;         /**< The time when the agent traverses the edge. */
  int agent_num; /**< The number of the agent, which causes the constraint. */

  /**
   * @brief Constructs an edge constraint with the given parameters.
   *
   * @param from_ The starting location of the edge.
   * @param to_ The ending location of the edge.
   * @param t_ The time when the agent traverses the edge.
   * @param agent_num_ The number of the agent, which causes the constraint.
   */
  EdgeConstraint(int from_, int to_, int t_, int agent_num_) : from(from_), to(to_), t(t_), agent_num(agent_num_)
  {
  }

  /**
   * @brief Equality check operator for the edge constraint.
   *
   * @param other The other edge constraint to compare with.
   *
   * @return  True if the edge constraints are equal, false otherwise.
   */
  auto operator==(const EdgeConstraint& other) const -> bool;
};

/**
 * @brief Represents a table of all edge constraints for all agents.
 * @todo Test properly the best data structure for storing the constraints. Possibilities are vector of hashtables, vector of sets, vector
 * of vectors
 */
class EdgeTable
{
public:
  /**
   * @brief Constructs an edge constraint table.
   *
   * @param instance  The instance of the problem.
   */
  explicit EdgeTable(const Instance& instance);

  /**
   * @brief Adds an edge constraint to the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time when the agent traverses the edge.
   * @param agent_num The number of the agent, which causes the constraint.
   */
  void add(int loc1, int loc2, int time, int agent_num);

  /**
   * @brief Removes an edge constraint from the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time when the agent traverses the edge.
   */
  void remove(int loc1, int loc2, int time);
  // auto get(int loc1, int loc2, int time) const -> bool;

  /**
   * @brief Retrieves an edge constraint from the table.
   *
   * @param loc1 The starting location of the edge.
   * @param loc2 The ending location of the edge.
   * @param time The time when the agent traverses the edge.
   *
   * @return The number of the agent, which causes the constraint. -1 is returned if the constraint does not exist.
   */
  [[nodiscard]] auto get(int loc1, int loc2, int time) const -> int;

  void reset()
  {
    edge_constraints.clear();
    edge_constraints.resize(NUM_DIRECTIONS);
    const int num_free_cells = instance.get_num_free_cells();
    for (int i = 0; i < NUM_DIRECTIONS; i++)
    {
      edge_constraints[i].resize(num_free_cells);
    }
  }

private:
  // std::unordered_set<EdgeConstraint, EdgeConstraint::hash_edge_collision> edge_constraints;  // constraints for edges
  // std::vector<std::vector<std::unordered_set<int>>> edge_constraints; // unordered set implementation
  std::vector<std::vector<std::vector<std::pair<int, int>>>>
                  edge_constraints; /**< vector implementation @todo try set, or something with log complexity*/
  const Instance& instance;         /**< The instance of the problem. */
};

