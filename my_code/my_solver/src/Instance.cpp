#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>
#include <stdexcept>
#include <tuple>

#include "Instance.h"

Instance::Instance(const std::string& map_fname, const std::string& scene_fname, int num_of_agents, int num_of_rows, int num_of_cols):
	num_of_cols(num_of_cols), num_of_rows(num_of_rows), num_of_agents(num_of_agents), map_fname(map_fname), scene_fname(scene_fname)
{
  load_map();
  load_scene();
  std::cout << "Successfully initialized instance." << std::endl;
}

bool Instance::load_map()
{
  // open the map file
  std::ifstream map_file(map_fname.c_str());

  // check if the open was successful
	if (!map_file.is_open())
  {
		return false;
  }

  // read the file line by line
  std::string line;
  while (getline(map_file, line)) 
  {
    // split the line
    std::vector<std::string> split_line;
    boost::split(split_line, line, boost::is_any_of(" "));

    // filter out invalid lines
    if (split_line.size() == 0) continue;

    if (boost::iequals(split_line[0], "type")) type = split_line[1]; // read type
    else if (boost::iequals(split_line[0], "height")) // read height
    {
      try
      {
        num_of_rows = stoi(split_line[1]);
      }
      catch (std::exception& e)
      {
        std::cout << "Could not read height: '" << e.what() << "'" << std::endl;
      }
    }
    else if (boost::iequals(split_line[0], "width")) // read width
    {
      try
      {
        num_of_cols = stoi(split_line[1]);
      }
      catch (std::exception& e)
      {
        std::cout << "Could not read width: '" << e.what() << "'" << std::endl;
      }
    }
    else if (boost::iequals(split_line[0], "map")) // read the map
    {
      while (getline(map_file, line)) 
      {
        for (int i = 0; i < (int) line.size(); i++)
        {
          // treat . as free cell
          if (line[i] == '.')
          {
            map_data.push_back(0);
          }
          // treat @, T as obstacle
          else if (line[i] == '@' || line[i] == 'T')
          {
            map_data.push_back(1);
          }
          // report all other symbols as unknown
          else
          {
            throw std::runtime_error("Unknown symbol in map description.");
          }
        }
        // check whether the number of cells in this row is consistent with num_of_columns 
        if (map_data.size() % num_of_cols != 0)
        {
          throw std::runtime_error("Inconsistent number of symbols in a row in the map description.");
        }
      }
      // check whether the number of lines is consistent with num_of_rows
      if ((int) map_data.size() != num_of_cols * num_of_rows)
      {
        throw std::runtime_error("Inconsistent number of rows in the map description.");
      }
    }
  }

  // cloase the map file
  map_file.close();

	return true;
}

bool Instance::load_scene()
{
  // open the scene file
  std::ifstream scene_file(scene_fname.c_str());

  // check if the open was successful
	if (!scene_file.is_open())
  {
		return false;
  }

  std::string line;

  // read the version
  if (!getline(scene_file, line) || line != "version 1") return false;

  int loaded_agents = 0;
  // read the file line by line until enough agents were loaded
  while (loaded_agents < num_of_agents && getline(scene_file, line)) 
  {
    // split the line - the format is 'bucket, map, map_width, map_height, start_x, start_y, goal_x, goal_y, optimal_length'
    std::vector<std::string> split_line;
    boost::split(split_line, line, boost::is_any_of("\t"));

    // filter out invalid lines
    if (split_line.size() != 9) continue;

    // read the start and goal positions
    try
    {
      int start_x = stoi(split_line[4]);
      int start_y = stoi(split_line[5]);
      int goal_x = stoi(split_line[6]);
      int goal_y = stoi(split_line[7]);

      // store the positions in the list
      start_positions.push_back(std::make_tuple(start_x, start_y));
      goal_positions.push_back(std::make_tuple(goal_x, goal_y));
      loaded_agents++;
    }
    catch (std::exception& e)
    {
      std::cout << "Could not read start and goal positions: '" << e.what() << "'" << std::endl;
    }
  }

  // cloase the map file
  scene_file.close();

  // check whether enough agents were loaded
  if (loaded_agents != num_of_agents)
  {
    std::cout << "WARNING: Unable to load " << num_of_agents << " agents, as only " << loaded_agents << " agents are available in the scene file." << std::endl;
    num_of_agents = loaded_agents;
  }

	return true;
}

void Instance::print_map()
{
  for (int i = 0; i < (int) map_data.size(); i++)
  {
    std::cout << (int) map_data[i];
    if (i % num_of_cols == num_of_cols - 1)
    {
      std::cout << std::endl;
    }
    else std::cout << " ";
  }
}

void Instance::print_agents()
{
  for (int i = 0; i < (int) start_positions.size(); i++)
  {
    std::cout << "Agent " << i << ": \t start (" << std::get<0>(start_positions[i]) << "," << std::get<1>(start_positions[i]) << ") \t"  
      << "goal (" << std::get<0>(goal_positions[i]) << "," << std::get<1>(goal_positions[i]) << ")" << std::endl;
  }
}

std::vector<uint8_t> Instance::get_map_data()
{
  return map_data; // should copy the vector
}

 std::vector<std::tuple<int, int>> Instance::get_start_positions()
{
  return start_positions; // should copy the vector
}

 std::vector<std::tuple<int, int>> Instance::get_goal_positions()
{
  return goal_positions; // should copy the vector
}
