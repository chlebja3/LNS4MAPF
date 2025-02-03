/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <cmath>
#include <memory>
#include <queue>
#include <unordered_set>
#include "Astar.h"

class AstarNode : Node
{
public:
  int x, y;

  AstarNode(const int x, const int y, const double g, const double h) : Node(g, h), x(x), y(y){};

  bool operator==(const AstarNode& other) const
  {
    return x == other.x && y == other.y && g == other.g && h == other.h;
  }

  struct compare
  {
    bool operator()(const std::unique_ptr<AstarNode> n1, const std::unique_ptr<AstarNode> n2) const
    {
      // compare f
      if (n1->f < n2->f)
        return true;
      else if (n1->f > n2->f)
        return false;
      else  // f1 == f2
        {
          if (n1->h == n2->h)        // tie
            return rand() % 2 == 1;  // choose randomly
          else
            return n1->h < n2->h;  // if f is the same, prefer nodes closer to the goal
        }
    }
  };

  struct hash
  {
    size_t operator()(const AstarNode& node) const
    {
      // hash x and y, bit shift to break the symmetry (x, y) wont have the same hash as (y, x) and xor to finalize
      return std::hash<int>()(node.x) ^ (std::hash<int>()(node.y) << 1);
    }
  };

  struct equal
  {
    bool operator()(const AstarNode& n1, const AstarNode& n2) const
    {
      return n1 == n2;
    }
  };
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

  // create frontier - possible datastructures priority_queue, ordered set and pairing heap, TODO try all of them
  std::priority_queue<std::unique_ptr<AstarNode>, std::vector<std::unique_ptr<AstarNode>>, AstarNode::compare> open_list;

  // create the start node
  open_list.push(std::make_unique<AstarNode>(start_x, start_y, 0, h));

  // create known nodes database
  std::unordered_set<AstarNode, AstarNode::hash, AstarNode::equal> known;

  // main loop
  while (!open_list.empty())
    {
      // pop the node to be processed
      std::unique_ptr<AstarNode> current = std::move(const_cast<std::unique_ptr<AstarNode>&>(open_list.top()));
      open_list.pop();

      // check whether node is in known set
      if (known.find(*current.get()) != known.end())
        continue;
      else
        known.insert(*current.get());

      //TODO doesnt this introduce memory leaks?

      // add to known nodes

      // check whether goal is reached
    }


  return path;
}

