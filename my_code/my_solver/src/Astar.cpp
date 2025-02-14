/*
 * Author: Jan Chleboun
 * Date: 08-01-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "Astar.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <queue>
#include <unordered_set>


class AstarNode : public Node
{
public:
  Point2d                    position;
  std::shared_ptr<AstarNode> parent;

  AstarNode(const int x, const int y, const double g, const double h, const std::shared_ptr<AstarNode> parent)
      : Node(g, h), position(Point2d(x, y)), parent(parent){};
  AstarNode(const Point2d& position, const double g, const double h, const std::shared_ptr<AstarNode> parent)
      : Node(g, h), position(position), parent(parent){};

  bool operator==(const AstarNode& other) const
  {
    return g == other.g && h == other.h && position == other.position;
  }

  struct compare_node_ptrs
  {
    bool operator()(const std::shared_ptr<AstarNode> n1, const std::shared_ptr<AstarNode> n2) const
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
};


namespace ASTAR
{
typedef std::priority_queue<std::shared_ptr<AstarNode>, std::vector<std::shared_ptr<AstarNode>>, AstarNode::compare_node_ptrs>
                                                         PriorityQueue;
typedef std::unordered_set<Point2d, Point2d::hash_point> UnorderedSet;

Path extract_path(const std::shared_ptr<AstarNode>& final_node);
}  // namespace ASTAR

Path ASTAR::find_path(const Point2d& start, const Point2d& goal, const Map& map_data)
{
  Path path;

  // check whether start and goal inside the map
  if (!map_data.is_in(start) || !map_data.is_in(goal)) return path;

  // check whether start and goal not obstructed
  if (map_data.index(start) == 1 && map_data.index(goal) == 1) return path;

  // calculate the heuristic
  double h = std::hypot(goal.x - start.x, goal.y - start.y);

  // create frontier - possible datastructures priority_queue, ordered set and pairing heap, TODO try all of them
  ASTAR::PriorityQueue open_list;

  // create the start node
  open_list.push(std::make_shared<AstarNode>(start.x, start.y, 0, h, nullptr));

  // create known nodes database
  ASTAR::UnorderedSet known;

  // main loop
  while (!open_list.empty())
  {
    // pop the node to be processed
    std::shared_ptr<AstarNode> current = std::move(open_list.top());
    open_list.pop();

    // check whether node is in known set
    if (known.find(current->position) != known.end()) continue;

    // check whether goal is reached
    if (current->position.x == goal.x && current->position.y == goal.y)
    {
      // extract the path
      return ASTAR::extract_path(current);
    }

    // add to known nodes
    known.insert(current->position);

    // expand current node
    std::vector<Point2d> neighbors = map_data.find_neighbors(current->position);
    for (Point2d& neighbor : neighbors)
    {
      // check whether neighbor already known
      if (known.find(neighbor) != known.end()) continue;

      double h = std::hypot(goal.x - neighbor.x, goal.y - neighbor.y);
      // add to the open list
      open_list.push(std::make_shared<AstarNode>(neighbor, current->g + 1, h, current));
    }
  }

  // could not find a path
  return {};
}

Path ASTAR::extract_path(const std::shared_ptr<AstarNode>& final_node)
{
  Path                       path;
  std::shared_ptr<AstarNode> curr = final_node;
  while (curr != nullptr)
  {
    path.push_back(curr->position);
    curr = curr->parent;
  }
  std::reverse(path.begin(), path.end());
  return path;
}
