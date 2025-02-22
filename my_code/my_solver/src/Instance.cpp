/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "Instance.h"

#include <bits/stdc++.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Astar.h"

Instance::Instance(const std::string& map_fname, const std::string& scene_fname, int num_of_agents)
    : num_of_agents(num_of_agents), map_fname(map_fname), scene_fname(scene_fname)
{
  initialize();
}

void Instance::initialize(const std::string& map_fname_, const std::string& scene_fname_, int num_of_agents_)
{
  map_fname     = map_fname_;
  scene_fname   = scene_fname_;
  num_of_agents = num_of_agents_;
  initialize();
}

void Instance::initialize()
{
  load_map();
  load_scene();
  calculate_heuristics();
  initialized = true;
}

bool Instance::is_initialized(void) const
{
  return initialized;
}

void Instance::load_map(void)
{
  // reset the map data
  Map new_map;
  int num_of_cols, num_of_rows = 0;
  // open the map file
  std::ifstream map_file(map_fname.c_str());

  // check if the open was successful
  if (!map_file.is_open()) throw std::runtime_error("Cannot open file '" + map_fname + "'");

  // read the file line by line
  std::string line;
  while (getline(map_file, line))
  {
    // split the line
    std::vector<std::string> split_line;
    boost::split(split_line, line, boost::is_any_of(" "));

    // filter out invalid lines
    if (split_line.size() == 0) continue;

    if (boost::iequals(split_line[0], "type"))
      type = split_line[1];                            // read type
    else if (boost::iequals(split_line[0], "height"))  // read height
    {
      try
      {
        num_of_rows = stoi(split_line[1]);
      }
      catch (const std::exception& e)
      {
        throw std::runtime_error("Unable to read the number of rows.");
      }
    }
    else if (boost::iequals(split_line[0], "width"))  // read width
    {
      try
      {
        num_of_cols = stoi(split_line[1]);
      }
      catch (const std::exception& e)
      {
        throw std::runtime_error("Unable to read the number of rows.");
      }
    }
    else if (boost::iequals(split_line[0], "map"))  // read the map
    {
      while (getline(map_file, line))
      {
        for (int i = 0; i < (int)line.size(); i++)
        {
          // treat . as free cell
          if (line[i] == '.')
          {
            new_map.data.push_back(0);
            new_map.free_cells++;
          }
          // treat @, T as obstacle
          else if (line[i] == '@' || line[i] == 'T')
          {
            new_map.data.push_back(1);
          }
          // report all other symbols as unknown
          else
          {
            throw std::runtime_error("Unknown symbol in map description.");
          }
        }
        // check whether the number of cells in this row is consistent with num_of_columns
        if (new_map.data.size() % num_of_cols != 0)
        {
          throw std::runtime_error("Inconsistent number of symbols in a row in the map description.");
        }
      }
      // check whether the number of lines is consistent with num_of_rows
      if ((int)new_map.data.size() != num_of_cols * num_of_rows)
      {
        throw std::runtime_error("Inconsistent number of rows in the map description.");
      }
    }
  }

  if (num_of_cols <= 0) throw std::runtime_error("Invalid number of columns.");
  if (num_of_rows <= 0) throw std::runtime_error("Invalid number of rows.");

  // close the map file
  map_file.close();

  // assign the size of the map
  new_map.height = num_of_rows;
  new_map.width  = num_of_cols;

  map_data = std::move(new_map);
}

void Instance::load_scene(void)
{
  // open the scene file
  std::ifstream scene_file(scene_fname.c_str());

  // check if the open was successful
  if (!scene_file.is_open()) throw std::runtime_error("Unable to open scene file.");

  std::string line;

  // read the version
  if (!getline(scene_file, line) || line != "version 1") throw std::invalid_argument("Invalid version of scene file.");

  int loaded_agents = 0;
  start_positions.reserve(num_of_agents);
  goal_positions.reserve(num_of_agents);
  // read the file line by line until enough agents were loaded
  while (loaded_agents < num_of_agents && getline(scene_file, line))
  {
    // split the line - the format is 'bucket, map, map_width, map_height, start_x, start_y, goal_x, goal_y, optimal_length'
    std::vector<std::string> split_line;
    boost::split(split_line, line, boost::is_any_of("\t "));

    // filter out invalid lines
    if (split_line.size() != 9)
    {
      std::cout << "Invalid line in scen file: " << line << " length: " << split_line.size() << std::endl;
      continue;
    }

    // read the start and goal positions
    try
    {
      const int start_x = stoi(split_line[4]);
      const int start_y = stoi(split_line[5]);
      const int goal_x  = stoi(split_line[6]);
      const int goal_y  = stoi(split_line[7]);

      // store the positions in the list
      start_positions.emplace_back(start_x, start_y);
      goal_positions.emplace_back(goal_x, goal_y);
      loaded_agents++;
    }
    catch (const std::exception& e)
    {
      throw std::runtime_error("Could not read start and goal positions: '" + (std::string)e.what() + "'");
    }
  }

  // cloase the map file
  scene_file.close();

  // check whether enough agents were loaded
  if (loaded_agents != num_of_agents)
  {
    std::cout << "WARNING: Unable to load " << num_of_agents << " agents, as only " << loaded_agents
              << " agents are available in the scene file." << std::endl;
    num_of_agents = loaded_agents;
  }
}

void Instance::print_agents(void) const
{
  for (int i = 0; i < (int)start_positions.size(); i++)
  {
    std::cout << "Agent " << i << ": \t start (" << start_positions[i].x << "," << start_positions[i].y << ") \t"
              << "goal (" << goal_positions[i].x << "," << goal_positions[i].y << ")" << std::endl;
  }
}

void Instance::print_map(void) const
{
  map_data.print();
}

const Map& Instance::get_map_data(void) const
{
  return map_data;  // should copy the vector
}

const std::vector<Point2d>& Instance::get_start_positions(void) const
{
  return start_positions;  // should copy the vector
}

const std::vector<Point2d>& Instance::get_goal_positions(void) const
{
  return goal_positions;  // should copy the vector
}

void Instance::calculate_optimal_paths(void)
{
  optimal_paths.clear();
  optimal_paths.reserve(num_of_agents);
  // calculate the optimal paths one by one
  for (int i = 0; i < num_of_agents; i++)
  {
    // find the optimal path for each agent not considering other agents
    Path p = ASTAR::find_path(i, *this);
    optimal_paths.push_back(p);
  }
}

void Instance::calculate_optimal_paths_parallel(void)
{
  optimal_paths.clear();
  optimal_paths.reserve(num_of_agents);
  // calculate the optimal paths one by one

  // prepare the vector with future paths
  std::vector<std::future<Path>> futures;

  // calculate asynchronously the paths
  for (int i = 0; i < num_of_agents; i++)
  {
    futures.push_back(std::async(std::launch::async, [this, i]() { return ASTAR::find_path(i, *this); }));
  }

  // wait for the paths and save them
  for (int i = 0; i < num_of_agents; i++)
  {
    optimal_paths.push_back(futures[i].get());
  }
}

const std::vector<Path>& Instance::get_optimal_paths(void) const
{
  return optimal_paths;
}


void Instance::calculate_heuristics(void)
{

  // check map validity
  assertm(map_data.width * map_data.height > 0, "Invalid map.");
  assertm((int)map_data.data.size() == map_data.width * map_data.height, "Map size invalid");
  assertm(map_data.free_cells > 0 && map_data.free_cells <= (int)map_data.data.size(), "Invalid free cell count.");

  // preallocate vectors for the heuristics
  const std::vector<int>    manhattan_initializer(map_data.width * map_data.height, 0);
  const std::vector<double> euclidean_initializer(map_data.width * map_data.height, 0.0);
  heuristic_manhattan.resize(num_of_agents, manhattan_initializer);
  heuristic_euclidean.resize(num_of_agents, euclidean_initializer);


  // fill the vectors
  for (int i = 0; i < num_of_agents; i++)  // iterate over agents
  {
    const Point2d& goal = goal_positions[i];
    for (int y = 0; y < map_data.height; y++)  // iterate over all positions
    {
      for (int x = 0; x < map_data.width; x++)
      {
        heuristic_manhattan[i][y * map_data.width + x] = abs(goal.x - x) + abs(goal.y - y);
        heuristic_euclidean[i][y * map_data.width + x] = std::hypot(goal.x - x, goal.y - y);
      }
    }
  }
};

int Instance::get_heuristic_manhattan(const int agent_num, const Point2d& pos) const
{

  assertm(agent_num >= 0 && agent_num < (int)heuristic_manhattan.size(), "Agent number out of range of the heuristic vector.");
  assertm(map_data.is_in(pos), "Trying to index a point that is not in the map.");
  return heuristic_manhattan[agent_num][pos.y * map_data.width + pos.x];
};
double Instance::get_heuristic_euclidean(const int agent_num, const Point2d& pos) const
{
  assertm(agent_num >= 0 && agent_num < (int)heuristic_euclidean.size(), "Agent number out of range of the heuristic vector.");
  assertm(map_data.is_in(pos), "Trying to index a point that is not in the map.");
  return heuristic_euclidean[agent_num][pos.y * map_data.width + pos.x];
};
