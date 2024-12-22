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
    Instance(const std::string& map_fname, const std::string& scene_fname, int num_of_agents = 0, int num_of_rows = 0, int num_of_cols = 0);

    // function to load a map
    bool load_map();

    // function to load a scene 
    bool load_scene();

    // function to print the map
    void print_map();

    // function to print the agents starts and goal positions
    void print_agents();

    // function to obtain the map data
    std::vector<uint8_t> get_map_data();

    // functions to obtain the agent starts and goals
    std::vector<std::tuple<int, int>> get_start_positions();
    std::vector<std::tuple<int, int>> get_goal_positions();
  private:
    std::string map_fname;
    std::string scene_fname;
    std::string type;
    std::vector<uint8_t> map_data;
    std::vector<std::tuple<int, int>> start_positions;
    std::vector<std::tuple<int, int>> goal_positions;
};



