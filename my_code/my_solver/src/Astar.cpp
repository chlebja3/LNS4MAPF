/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <cmath>
#include "Astar.h"

class AstarNode : Node
{
public:
  int x, y;

  AstarNode(const int x, const int y, const double g, const double h) : Node(g, h), x(x), y(y){};

  bool operator==(const AstarNode& other)
  {
    return x == other.x && y == other.y && g == other.g && h == other.h;
  }
};

Path ASTAR::find_path(const std::tuple<int, int>& start, const std::tuple<int, int>& goal, const std::vector<uint8_t>& map_data, const int map_width,
                      const int map_height)
{
  const int start_x = std::get<0>(start);
  const int start_y = std::get<1>(start);
  const int goal_x  = std::get<0>(goal);
  const int goal_y  = std::get<1>(goal);

  Path path;

  // check whether start and goal valid
  if (start_x < 0 || start_y < 0 || start_x >= map_width || start_y >= map_height)
    return path;
  if (goal_x < 0 || goal_y < 0 || goal_x >= map_width || goal_y >= map_height)
    return path;

  // check whether start and goal not obstructed
  if (map_data[start_y * map_width + start_x * map_height] == 1 || map_data[start_y * map_width + start_x * map_height] == 1)
    return path;

  // calculate the heuristic
  double h = std::hypot(goal_x - start_x, goal_y - start_y);

  // create the start node
  AstarNode start_node(start_x, start_y, 0, h);


  return path;
}

