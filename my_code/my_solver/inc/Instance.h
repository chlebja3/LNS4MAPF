/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: A class that represents an instance of the MAPF problem and related functions.
 */

#pragma once
#include <string>
#include <vector>

#include "utils.h"

class Instance
{
public:
  int num_of_agents;

  // constructor
  Instance() = default;
  Instance(std::string map_fname_, std::string scene_fname_, int num_of_agents_ = 0);

  void               initialize(const std::string& map_fname, const std::string& scene_fname, int num_of_agents = 0);
  void               initialize();
  [[nodiscard]] auto is_initialized() const -> bool;

  // function to print the map
  void print_map() const;

  // function to print the agents starts and goal positions
  void print_agents() const;

  // function to obtain the map data
  [[nodiscard]] auto get_map_data() const -> const Map&;

  // functions to obtain the agent starts, goals and optimal paths
  [[nodiscard]] auto get_start_positions() const -> const std::vector<Point2d>&;
  [[nodiscard]] auto get_start_locations() const -> const std::vector<int>&;
  [[nodiscard]] auto get_goal_positions() const -> const std::vector<Point2d>&;
  [[nodiscard]] auto get_goal_locations() const -> const std::vector<int>&;
  [[nodiscard]] auto get_optimal_paths() const -> const std::vector<Path>&;

  // function to calculate the optimal paths ignoring the other agents
  void calculate_optimal_paths();
  void calculate_optimal_paths_parallel();

  void calculate_heuristics();
  void precompute_neighbors();
  //[[nodiscard]] auto get_heuristic_manhattan(int agent_num, int loc) const -> int;
  [[nodiscard]] inline auto get_heuristic_manhattan(const int agent_num, const int loc) const -> int
  {

    assertm(agent_num >= 0 && agent_num < (int)heuristic_manhattan.size(), "Agent number out of range of the heuristic vector.");
    assertm(map_data.is_in(loc), "Trying to index a point that is not in the map.");
    return heuristic_manhattan[agent_num][loc];
  };

  [[nodiscard]] inline auto get_heuristic_distance(const int agent_num, const int loc) const -> int
  {

    assertm(agent_num >= 0 && agent_num < (int)heuristic_manhattan.size(), "Agent number out of range of the heuristic vector.");
    assertm(map_data.is_in(loc), "Trying to index a point that is not in the map.");
    return heuristic_distance[agent_num][loc];
  };

  [[nodiscard]] auto get_heuristic_euclidean(int agent_num, int loc) const -> double;
  //[[nodiscard]] auto get_neighbor_locations(int loc) const -> const std::vector<int>&;
  [[nodiscard]] inline auto get_neighbor_locations(int loc) const -> const std::vector<int>&
  {
    assertm(map_data.is_in(loc), "Trying to get neighbors of invalid location.");
    assertm(loc < (int)neighbors.size(), "Neighbors are not precomputed for this location.");
    return neighbors[loc];
  }

  [[nodiscard]] auto location_to_position(int location) const -> Point2d;
  [[nodiscard]] auto position_to_location(const Point2d& position) const -> int;
  [[nodiscard]] auto path_to_pointpath(const Path& path) const -> PointPath;


private:
  /**
   * @brief Load the map from a file specified during Instance initialization.
   *
   * @return Success of the operation
   */
  void load_map();

  // function to load a scene
  void load_scene();

  void calculate_distance_heuristic();

  bool                          initialized = false;
  std::string                   map_fname;
  std::string                   scene_fname;
  std::string                   type;
  Map                           map_data;
  std::vector<Point2d>          start_positions;
  std::vector<int>              start_locations;
  std::vector<Point2d>          goal_positions;
  std::vector<int>              goal_locations;
  std::vector<Path>             optimal_paths;
  std::vector<std::vector<int>> neighbors;

  // heuristics
  std::vector<std::vector<int>>    heuristic_manhattan;
  std::vector<std::vector<double>> heuristic_euclidean;
  std::vector<std::vector<int>>    heuristic_distance;
};

