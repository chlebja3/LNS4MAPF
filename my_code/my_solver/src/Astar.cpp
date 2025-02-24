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
#include <list>
#include <memory>
#include <queue>
#include <unordered_set>

#define NODE_POOL_MERGE_SIZE 100
int ASTAR::generated_nodes = 0;

class AstarNode : public Node
{
public:
  int              location;
  AstarNode const* parent;  // pointer to const parent

  AstarNode(const int location, const double g, const double h, AstarNode const* const parent)
      : Node(g, h), location(location), parent(parent){};

  bool operator==(const AstarNode& other) const
  {
    return g == other.g && h == other.h && location == other.location;
  }

  struct compare_node_ptrs
  {
    bool operator()(AstarNode const* n1, AstarNode const* n2) const
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
typedef std::priority_queue<AstarNode const*, std::vector<AstarNode const*>, AstarNode::compare_node_ptrs> PriorityQueue;

Path extract_path(AstarNode const* const final_node);

class NodePool
{
public:
  NodePool(const int size);
  AstarNode* add_node(AstarNode node);
  void       merge_extra(void);

private:
  int                    end = 0;
  std::vector<AstarNode> pool;
  std::list<AstarNode>   extra;
};
}  // namespace ASTAR

ASTAR::NodePool::NodePool(const int size)
{
  // initialize the pool with dummy nodes
  pool.resize(size, AstarNode(0, 0, 0, nullptr));
  end = 0;
}

AstarNode* ASTAR::NodePool::add_node(AstarNode node)
{
  // check, whether there is still space in the node pool
  if (end < (int)pool.size())
  {
    // move to the end of the pool and increment the count
    pool[end++] = std::move(node);
    return &pool[end - 1];
  }

  // if there is no space in the vector, add to the list, which will be merged with the vector at the end of the iteration
  extra.push_back(node);
  return &extra.back();
}


void ASTAR::NodePool::merge_extra(void)
{
  if (extra.size() > NODE_POOL_MERGE_SIZE)
  {
    // merge the list with the vector
    // be careful, this invalidates all pointers to the nodes in the node pool
    pool.insert(pool.end(), std::move_iterator{extra.begin()}, std::move_iterator{extra.end()});
    extra.clear();
  }
  // reset the node counter
  end = 0;
}


Path ASTAR::find_path(const int agent_num, const Instance& instance)
{
  assertm(agent_num < (int)instance.get_start_locations().size(), "Agent number out of range.");
  const Map& map_data = instance.get_map_data();
  const int  start    = instance.get_start_locations()[agent_num];
  const int  goal     = instance.get_goal_locations()[agent_num];
  generated_nodes     = 0;

  static ASTAR::NodePool node_pool(instance.get_map_data().width * instance.get_map_data().height);

  Path path;

  // check whether start and goal inside the map
  assertm(map_data.is_in(start) && map_data.is_in(goal), "Start or goal outside the map.");

  // check whether start and goal not obstructed
  assertm(map_data.index(start) == 0 && map_data.index(goal) == 0, "Start or goal obstructed.");


  // create frontier - possible datastructures priority_queue, ordered set and pairing heap
  ASTAR::PriorityQueue open_list;

  // calculate the heuristic
  const double h = instance.get_heuristic_manhattan(agent_num, start);

  // add the start node to the priority queue
  open_list.push(node_pool.add_node(AstarNode(start, 0, h, nullptr)));
  ASTAR::generated_nodes++;
  // std::cout << ASTAR::generated_nodes << std::endl;

  // create known nodes database
  std::vector<bool> known;
  known.resize(map_data.width * map_data.height, false);

  // main loop
  while (!open_list.empty())
  {
    // pop the node to be processed
    AstarNode const* const current = std::move(open_list.top());
    open_list.pop();

    // check whether node is in known set
    if (known[current->location]) continue;

    // check whether goal is reached
    if (current->location == goal)
    {
      // extract the path
      Path ret = ASTAR::extract_path(current);

      // merge the extra generated nodes
      node_pool.merge_extra();

      return ret;
    }

    // add to known nodes
    known[current->location] = true;

    // expand current node
    std::vector<int> neighbors = map_data.find_neighbors(current->location);
    for (auto& neighbor : neighbors)
    {
      // check whether neighbor already known
      if (known[neighbor]) continue;

      // calculate heuristic
      // const double h = instance.get_heuristic_euclidean(agent_num, neighbor);
      const double h = instance.get_heuristic_manhattan(agent_num, neighbor);

      // add to the open list
      open_list.push(node_pool.add_node(AstarNode(neighbor, current->g + 1, h, current)));
      ASTAR::generated_nodes++;
    }
  }

  node_pool.merge_extra();
  // could not find a path
  return {};
}

Path ASTAR::extract_path(AstarNode const* const final_node)
{
  Path             path;
  AstarNode const* curr = final_node;
  while (curr != nullptr)
  {
    path.push_back(curr->location);
    curr = curr->parent;
  }
  std::reverse(path.begin(), path.end());
  return path;
}
