/*
 * Author: Jan Chleboun
 * Date: 09-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "SIPP.h"

#include <math.h>

#include <memory>
#include <queue>

SIPP::SIPP(const Map& map_data) : map_data(map_data){};

class TimeInterval
{
public:
  int t_min, t_max;

  bool operator==(const TimeInterval& other) const
  {
    return t_min == other.t_min && t_max == other.t_max;
  }
};

class SIPPNode : public Node
{
public:
  Point2d                   position;
  TimeInterval              interval;
  std::shared_ptr<SIPPNode> parent;

  SIPPNode(const int x, const int y, const double g, const double h, const std::shared_ptr<SIPPNode> parent)
      : Node(g, h), position(Point2d(x, y)), parent(parent){};
  SIPPNode(const Point2d& position, const double g, const double h, const std::shared_ptr<SIPPNode> parent)
      : Node(g, h), position(position), parent(parent){};

  bool operator==(const SIPPNode& other) const
  {
    return g == other.g && h == other.h && position == other.position && interval == other.interval;
  }

  struct compare_node_ptrs
  {
    bool operator()(const std::shared_ptr<SIPPNode> n1, const std::shared_ptr<SIPPNode> n2) const
    {
      // compare f
      if (n1->f < n2->f)
        return false;
      else if (n1->f > n2->f)
        return true;
      else  // f1 == f2
      {
        if (n1->h == n2->h)        // tie
          return rand() % 2 == 1;  // choose randomly
        else
          return n1->h > n2->h;  // if f is the same, prefer nodes closer to the goal
      }
    }
  };

  struct hash_node_ptrs  // unused
  {
    size_t operator()(const std::shared_ptr<SIPPNode>& node) const
    {
      // hash x and y, bit shift to break the symmetry (x, y) wont have the same hash as (y, x) and xor to finalize
      return std::hash<int>()(node.get()->position.x) ^ (std::hash<int>()(node.get()->position.y) << 1);
    }
  };

  struct equal_node_ptrs  // unused
  {
    bool operator()(const std::shared_ptr<SIPPNode>& n1, const std::shared_ptr<SIPPNode>& n2) const
    {
      return *n1 == *n2;
    }
  };
};

namespace sipp
{
typedef std::priority_queue<std::shared_ptr<SIPPNode>, std::vector<std::shared_ptr<SIPPNode>>, SIPPNode::compare_node_ptrs> PriorityQueue;

Path extract_path(const std::shared_ptr<SIPPNode>& final_node);
}  // namespace sipp

Path SIPP::plan(const Point2d& start, const Point2d& goal, const Map& map_data)
{
  // check whether start and goal inside the map
  if (!map_data.is_in(start) || !map_data.is_in(goal)) return Path();

  // check whether start and goal not obstructed
  if (map_data.index(start) == 1 && map_data.index(goal) == 1) return Path();

  // create openlist
  sipp::PriorityQueue open_list;

  // calculate the heuristic
  double h = std::hypot(goal.x - start.x, goal.y - start.y);

  // create the start node
  open_list.push(std::make_shared<SIPPNode>(start.x, start.y, 0, h, nullptr));

  // main loop
  while (!open_list.empty())
  {
    // pop the node to be processed
    std::shared_ptr<SIPPNode> current = std::move(open_list.top());
    open_list.pop();

    // check whether node is in known set

    // check whether goal is reached
    if (current->position.x == goal.x && current->position.y == goal.y)
    {
      // extract the path
      return sipp::extract_path(current);
    }

    // add to known nodes

    // expand current node
  }
  return Path();
}

Path sipp::extract_path(const std::shared_ptr<SIPPNode>& final_node)
{
  Path                      path;
  std::shared_ptr<SIPPNode> curr = final_node;
  while (curr != nullptr)
  {
    // TODO push as many times as the interval length
    path.push_back(curr->position);
    curr = curr->parent;
  }
  std::reverse(path.begin(), path.end());
  return path;
}
