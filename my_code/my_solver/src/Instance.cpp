#include "Instance.h"
#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>

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
        // TODO 
        std::cout << "Not implemented." << std::endl;
      }
       
    }
  }

  // cloase the map file
  map_file.close();

	return true;
}
