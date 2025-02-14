/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "Solver.h"

#include <boost/algorithm/string_regex.hpp>
#include <fstream>
#include <iostream>
#include <unordered_set>

bool Solution::is_valid(std::shared_ptr<Instance> instance) const
{
  // check that each agent has a path
  if (instance->num_of_agents != (int)paths.size())
  {
    // std::cout << "Agent without a path." << std::endl;
    return false;
  }

  // check that each path starts and ends in the correct position and find the longest path
  int T_max = 0;
  for (int i = 0; i < (int)paths.size(); i++)
  {
    // check that no path has length 0
    if (paths[i].empty())
    {
      // std::cout << "Empty path." << std::endl;
      return false;
    }

    // check that the path starts in the start position
    if (!(paths[i].front() == instance->get_start_positions()[i]))
    {
      // std::cout << "Path does not start in the start position." << std::endl;
      return false;
    }

    // check that the path ends in the goal position
    if (!(paths[i].back() == instance->get_goal_positions()[i]))
    {
      // std::cout << "Path does not end in the goal position." << std::endl;
      return false;
    }

    // remember the length of the longest path
    if ((int)paths[i].size() > T_max) T_max = paths[i].size();
  }
  //  std::cout << "Max length of path: " << T_max << std::endl;

  // now check that there are no collisions or invalid points or invalid moves
  for (int t = 0; t < T_max; t++)
  {
    // create reservation set for each time step to check for dynamic coliisions
    std::unordered_set<Point2d, Point2d::hash_point> occupied;

    for (int i = 0; i < (int)paths.size(); i++)
    {
      int       idx          = t;
      const int cur_path_len = (int)paths[i].size();

      // paths that are already finished at time t
      if (t >= cur_path_len)
      {
        // stay at the last point
        idx = cur_path_len - 1;
      }

      if (idx > 0)
      {
        // check if the move is valid
        if (abs(paths[i][idx].x - paths[i][idx - 1].x) + abs(paths[i][idx].y - paths[i][idx - 1].y) > 1)
        {
          // std::cout << "Invalid move." << std::endl;
          return false;
        }
      }

      // check the path does not leave the map
      if (!instance->get_map_data().is_in(paths[i][idx]))
      {
        // std::cout << "Path leaves the map in point " << paths[i][idx] << std::endl;
        return false;
      }

      // check collisions with static obstacles
      if (instance->get_map_data().index(paths[i][idx]) != 0)
      {
        // std::cout << "Collision with static obstacle." << std::endl;
        return false;
      }

      // check collisions with dynamic obstacles
      if (occupied.find(paths[i][idx]) != occupied.end())
      {
        // std::cout << "Collision with dynamic obstacle." << std::endl;
        return false;
      }

      // add the point to the reservation set
      occupied.insert(paths[i][idx]);
    }
  }

  // check that the agents do not swap edges
  for (int i = 0; i < (int)paths.size(); i++)
  {
    for (int j = i + 1; j < (int)paths.size(); j++)
    {
      for (int t = 1; t < T_max; t++)
      {
        const int cur_path_len_i = (int)paths[i].size();
        const int cur_path_len_j = (int)paths[j].size();

        // paths that are already finished at time t
        if (t >= cur_path_len_i || t >= cur_path_len_j) continue;

        if (paths[i][t] == paths[j][t - 1] && paths[i][t - 1] == paths[j][t])
        {
          // std::cout << "Agents swap edges." << std::endl;
          return false;
        }
      }
    }
  }

  return true;
}

void Solution::load(const std::string& filename)
{
  // open the solution file
  std::ifstream sol_file(filename.c_str());

  // check if the file is open
  if (!sol_file.is_open())
  {
    throw std::runtime_error("Cannot open file '" + filename + "'");
  }

  // read agents one by one
  std::string line;
  int         expected_agent_num = 0;

  while (getline(sol_file, line))
  {
    // remove leading and trailing spaces
    line = boost::algorithm::trim_copy(line);

    // find the agent number
    size_t idx = line.find("Agent ");
    if (idx == std::string::npos)
    {
      throw std::runtime_error("Cannot find agent number in line '" + line + "'");
    }

    size_t idx2 = line.find(":");
    if (idx == std::string::npos)
    {
      throw std::runtime_error("Cannot find : in line '" + line + "'");
    }

    // get the agent number
    int agent_num;
    try
    {
      agent_num = std::stoi(line.substr(idx + 6, idx2 - idx - 6));
    }
    catch (const std::exception& e)
    {
      throw std::runtime_error("Cannot convert agent number in line '" + line + "'");
    }

    // check that the agent number is correct
    if (agent_num != expected_agent_num)
    {
      throw std::runtime_error("Unexpected agent number in line '" + line + "'");
    }
    else
      expected_agent_num++;

    // split the line
    std::vector<std::string> split_line;
    boost::algorithm::split_regex(split_line, line.substr(idx2 + 1), boost::regex("->"));

    // read the path
    Path path;
    for (const std::string& point_str : split_line)
    {
      if (point_str.empty()) continue;
      // remove the leading and trailing spaces
      std::string point_str_trimmed = boost::algorithm::trim_copy(point_str);

      // find the opening bracket of the point
      size_t idx = point_str_trimmed.find("(");
      if (idx == std::string::npos)
      {
        throw std::runtime_error("Cannot find '(' in point '" + point_str + "'");
      }

      // find the closing bracket of the point
      size_t idx2 = point_str_trimmed.find(")");
      if (idx2 == std::string::npos)
      {
        throw std::runtime_error("Cannot find ')' in point '" + point_str + "'");
      }

      // find ,
      size_t idx_comma = point_str_trimmed.find(",");
      if (idx_comma == std::string::npos)
      {
        throw std::runtime_error("Cannot find ',' in point '" + point_str + "'");
      }

      // get the coordinates
      int x, y;
      try
      {
        // load in format (y,x)
        y = std::stoi(point_str_trimmed.substr(idx + 1, idx_comma - 1));
        x = std::stoi(point_str_trimmed.substr(idx_comma + 1, idx2 - idx_comma - 1));
      }
      catch (const std::exception& e)
      {
        throw std::runtime_error("Cannot convert coordinates in line '" + point_str_trimmed +
                                 "', x: " + point_str_trimmed.substr(idx + 1, idx_comma - 1) +
                                 ", y: " + point_str_trimmed.substr(idx_comma + 1, idx2 - idx_comma - 1));
      }

      // add the point to the path
      path.push_back(Point2d(x, y));
    }
    // add the path to the solution
    paths.push_back(path);
  }


  // close the file
  sol_file.close();
}


Solver::Solver(const std::string& name, std::shared_ptr<Instance> instance) : name(name), instance(instance){};
