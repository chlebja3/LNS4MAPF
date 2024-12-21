#include "Instance.h"
#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>
#include <stdexcept>

Instance::Instance(const std::string& map_fname, int num_of_agents, int num_of_rows, int num_of_cols):
	num_of_cols(num_of_cols), num_of_rows(num_of_rows), num_of_agents(num_of_agents), map_fname(map_fname)
{
  loadMap();
  std::cout << "Successfully initialized instance." << std::endl;
}

bool Instance::loadMap()
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
          // treat @ as obstacle
          else if (line[i] == '@')
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

std::vector<uint8_t> Instance::get_map_data()
{
  return map_data; // should copy the vector
}
