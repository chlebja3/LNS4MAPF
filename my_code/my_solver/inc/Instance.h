/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#pragma once
#include <string>
#include <vector>
#include "utils.h"

class Map
{
public:
  int                  width  = 0;
  int                  height = 0;
  std::vector<uint8_t> data;

  void print() const;
};

class Instance
{
public:
  int num_of_cols;
  int num_of_rows;
  int num_of_agents;
  int map_size;

  // constructor
  Instance(const std::string& map_fname, const std::string& scene_fname, int num_of_agents = 0, int num_of_rows = 0, int num_of_cols = 0);

  // function to load a map
  bool load_map();

  // function to load a scene
  bool load_scene();

  // function to print the map
  void print_map() const;

  // function to print the agents starts and goal positions
  void print_agents() const;

  // function to obtain the map data
  const Map& get_map_data() const;

  // functions to obtain the agent starts and goals
  std::vector<std::tuple<int, int>> get_start_positions() const;
  std::vector<std::tuple<int, int>> get_goal_positions() const;

  // function to calculate the optimal paths ignoring the other agents
  void calculate_optimal_paths();

private:
  std::string                       map_fname;
  std::string                       scene_fname;
  std::string                       type;
  Map                               map_data;
  std::vector<std::tuple<int, int>> start_positions;
  std::vector<std::tuple<int, int>> goal_positions;
  std::vector<Path>                 optimal_paths;
};

