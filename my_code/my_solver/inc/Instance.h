#pragma once
#include <string>
#include <vector>

class Instance
{
  public:
    int num_of_cols;
    int num_of_rows;
    int num_of_agents;
    int map_size;
	
    // constructor
    Instance(const std::string& map_fname, int num_of_agents = 0, int num_of_rows = 0, int num_of_cols = 0);

    // function to load a map
    bool loadMap();

    // function to print the map
    void print_map();

    // function to obtain the map data
    std::vector<uint8_t> get_map_data();

  private:
    std::string map_fname;
    std::string type;
    std::vector<uint8_t> map_data;
};



