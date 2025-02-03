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

  struct compare_node_ptrs
  {
    bool operator()(const std::shared_ptr<AstarNode> n1, const std::shared_ptr<AstarNode> n2) const
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

  struct hash_node_ptrs
  {
    size_t operator()(const std::shared_ptr<AstarNode>& node) const
    {
      // hash x and y, bit shift to break the symmetry (x, y) wont have the same hash as (y, x) and xor to finalize
      return std::hash<int>()(node.get()->x) ^ (std::hash<int>()(node.get()->y) << 1);
    }
  };

  struct equal_node_ptrs
  {
    bool operator()(const std::shared_ptr<AstarNode>& n1, const std::shared_ptr<AstarNode>& n2) const
    {
      return *n1.get() == *n2.get();
    }
  };
};


namespace ASTAR
{
typedef std::priority_queue<std::shared_ptr<AstarNode>, std::vector<std::shared_ptr<AstarNode>>, AstarNode::compare_node_ptrs> PriorityQueue;
typedef std::unordered_set<std::shared_ptr<AstarNode>, AstarNode::hash_node_ptrs, AstarNode::equal_node_ptrs>                  UnorderedSet;

void expand_node(const AstarNode& node, const Map& map_data, ASTAR::PriorityQueue& open_list, ASTAR::UnorderedSet& known);
}  // namespace ASTAR

Path ASTAR::find_path(const std::tuple<int, int>& start, const std::tuple<int, int>& goal, const Map& map_data)
{
  const int start_x = std::get<0>(start);
  const int start_y = std::get<1>(start);
  const int goal_x  = std::get<0>(goal);
  const int goal_y  = std::get<1>(goal);

  Path path;

  // check whether start and goal valid
  if (start_x < 0 || start_y < 0 || start_x >= map_data.width || start_y >= map_data.height)
    return path;
  if (goal_x < 0 || goal_y < 0 || goal_x >= map_data.width || goal_y >= map_data.height)
    return path;

  // check whether start and goal not obstructed
  if (map_data.data[start_y * map_data.width + start_x * map_data.height] == 1 || map_data.data[start_y * map_data.width + start_x * map_data.height] == 1)
    return path;

  // calculate the heuristic
  double h = std::hypot(goal_x - start_x, goal_y - start_y);

  // create frontier - possible datastructures priority_queue, ordered set and pairing heap, TODO try all of them
  ASTAR::PriorityQueue open_list;

  // create the start node
  open_list.push(std::make_shared<AstarNode>(start_x, start_y, 0, h));

  // create known nodes database
  ASTAR::UnorderedSet known;

  // main loop
  while (!open_list.empty())
    {
      // pop the node to be processed
      std::shared_ptr<AstarNode> current = std::move(open_list.top());
      open_list.pop();

      // check whether node is in known set
      if (known.find(current) != known.end())
        continue;

      // check whether goal is reached
      if (current.get()->x == goal_x && current.get()->y == goal_y)
        {
          // TODO extract the path

          break;
        }

      // add to known nodes
      known.insert(current);

      // expand current node
    }


  return path;
}

void ASTAR::expand_node(const AstarNode& node, const Map& map_data, ASTAR::PriorityQueue& open_list, ASTAR::UnorderedSet& known)
{
}
