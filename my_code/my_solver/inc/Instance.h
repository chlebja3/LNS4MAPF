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
  Instance(void) = default;
  Instance(const std::string& map_fname, const std::string& scene_fname, int num_of_agents = 0);

  void initialize(const std::string& map_fname, const std::string& scene_fname, int num_of_agents = 0);
  void initialize(void);
  bool is_initialized(void) const;

  // function to print the map
  void print_map(void) const;

  // function to print the agents starts and goal positions
  void print_agents(void) const;

  // function to obtain the map data
  const Map& get_map_data(void) const;

  // functions to obtain the agent starts, goals and optimal paths
  const std::vector<Point2d>& get_start_positions(void) const;
  const std::vector<Point2d>& get_goal_positions(void) const;
  const std::vector<Path>&    get_optimal_paths(void) const;

  // function to calculate the optimal paths ignoring the other agents
  void calculate_optimal_paths(void);
  void calculate_optimal_paths_parallel(void);

  void   calculate_heuristics(void);
  int    get_heuristic_manhattan(const int agent_num, const Point2d& pos) const;
  double get_heuristic_euclidean(const int agent_num, const Point2d& pos) const;


private:
  /**
   * @brief Load the map from a file specified during Instance initialization.
   *
   * @return Success of the operation
   */
  void load_map(void);

  // function to load a scene
  void load_scene(void);

  bool                 initialized = false;
  std::string          map_fname;
  std::string          scene_fname;
  std::string          type;
  Map                  map_data;
  std::vector<Point2d> start_positions;
  std::vector<Point2d> goal_positions;
  std::vector<Path>    optimal_paths;

  // heuristics
  std::vector<std::vector<int>>    heuristic_manhattan;
  std::vector<std::vector<double>> heuristic_euclidean;
};

