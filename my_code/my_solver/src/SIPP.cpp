/*
 * Author: Jan Chleboun
 * Date: 09-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "SIPP.h"

#include <memory>

SIPP::SIPP(const Map& map_data) : map_data(map_data){};

class SIPPNode : public Node
{
public:
  Point2d                   position;
  std::shared_ptr<SIPPNode> parent;

  SIPPNode(const int x, const int y, const double g, const double h, const std::shared_ptr<SIPPNode> parent)
      : Node(g, h), position(Point2d(x, y)), parent(parent){};
  SIPPNode(const Point2d& position, const double g, const double h, const std::shared_ptr<SIPPNode> parent)
      : Node(g, h), position(position), parent(parent){};

  bool operator==(const SIPPNode& other) const
  {
    return g == other.g && h == other.h && position == other.position;
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

Path SIPP::plan(const Point2d& start, const Point2d& goal, const Map& map_data)
{
  // TODO
}
