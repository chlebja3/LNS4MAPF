/*
 * Author: Jan Chleboun
 * Date: 09-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: Implementation of the SIPP algorithm for path planning in a grid map. The algorithm is based on the A* search algorithm, but
 * searches also in the time dimension. To reduce dimensionality of the space, the time is discretized into intervals.
 */

#include "SIPP.h"

#include <limits.h>
#include <math.h>

#include <memory>

#include "utils.h"

#define MOVE_TIME 1               // time to move to the next cell
#define NODE_POOL_MERGE_SIZE 500  // merge the extra list with the node pool vector only of there are more elements in the list

using namespace sipp;

SIPP::SIPP(const Instance& instance)
    : constraint_table(ConstraintTable(instance)),
      instance(instance),
      node_pool(NodePool(5 * instance.get_map_data().width * instance.get_map_data().height)){};


SIPPNode::SIPPNode(const int x, const int y, const int time_min, const int time_max, const double g, const double h,
                   SIPPNode const* const parent)
    : Node(g, h), time_point(TimePoint(x, y, time_min, time_max)), parent(parent){};

SIPPNode::SIPPNode(const Point2d& position, const TimeInterval& interval, const double g, const double h, SIPPNode const* const parent)
    : Node(g, h), time_point(position, interval), parent(parent){};

SIPPNode::SIPPNode(const TimePoint& time_point, const double g, const double h, SIPPNode const* const parent)
    : Node(g, h), time_point(time_point), parent(parent){};


bool SIPPNode::operator==(const SIPPNode& other) const
{
  return g == other.g && h == other.h && time_point == other.time_point;
}

bool SIPPNode::compare_node_ptrs::operator()(SIPPNode const* n1, SIPPNode const* n2) const
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


NodePool::NodePool(const int size)
{
  // initialize the pool with dummy nodes
  pool.resize(size, SIPPNode({0, 0}, {0, 0}, 0, 0, nullptr));
  end = 0;
}

SIPPNode* NodePool::add_node(SIPPNode node)
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


void NodePool::merge_extra(void)
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

TimePointPath SIPP::plan(const int agent_num)
{
  const Map&     map_data = instance.get_map_data();
  const Point2d& start    = instance.get_start_positions()[agent_num];
  const Point2d& goal     = instance.get_goal_positions()[agent_num];

  // check whether start and goal inside the map
  assertm(map_data.is_in(start) && map_data.is_in(goal), "Start or goal outside the map.");

  // check whether start and goal not obstructed
  assertm(map_data.index(start) == 0 && map_data.index(goal) == 0, "Start or goal obstructed.");

  // estimate the maximum time the goal can be reached (after all dynamic constraints disappear, the path can go through all free cells
  // in the worst case)
  int max_time = std::max(INT_MAX, constraint_table.get_last_dynamic_constraint() + map_data.free_cells);
  // std::cout << "Max time: " << max_time << std::endl;

  // create openlist
  sipp::PriorityQueue open_list;

  // create a vector to store all nodes so that they dont go out of scope
  std::vector<std::unique_ptr<SIPPNode>> node_list = {};
  node_list.reserve(map_data.width * map_data.height);


  // retrieve the first safe interval
  TimeInterval time_interval_start = constraint_table.get_first_safe_interval(start);

  // calculate the heuristic of the start node
  {
    // const double h = instance.get_heuristic_euclidean(agent_num, start);
    const double h = instance.get_heuristic_manhattan(agent_num, start);
    // create the start node
    open_list.push(node_pool.add_node(SIPPNode(start, time_interval_start, 0, h, nullptr)));
  }
  generated_nodes++;

  // create known nodes database
  sipp::UnorderedMap known;

  // main loop
  while (!open_list.empty())
  {
    // pop the node to be processed
    SIPPNode const* const current = std::move(open_list.top());
    open_list.pop();

    // std::cout << current->time_point.position << ", " << current->time_point.interval << std::endl;

    // check whether node was expanded already (same position and tmax higher then current tmin)
    if (known.find(current->time_point.position) != known.end() &&
        current->time_point.interval.t_min <= known[current->time_point.position])
      continue;


    // check whether goal position is reached and that there will be no more dynamic constraints at the goal location
    if (current->time_point.position == goal && current->time_point.interval.t_max == INT_MAX)
    {
      // extract the path
      TimePointPath ret = sipp::extract_path(current);

      // reset the node pool
      node_pool.merge_extra();

      return ret;
    }

    // add to known nodes
    known[current->time_point.position] = current->time_point.interval.t_max;

    // expand current node
    std::vector<Point2d> neighbors = map_data.find_neighbors(current->time_point.position);
    for (Point2d& neighbor : neighbors)
    {
      // std::cout << "Neighbor: " << neighbor << std::endl;
      // calculate the heuristic
      // const double h = instance.get_heuristic_euclidean(agent_num, neighbor);
      const double h = instance.get_heuristic_manhattan(agent_num, neighbor);

      // retrieve the safe intervals for the neighbor
      std::vector<TimeInterval> time_intervals = constraint_table.get_safe_intervals(
          neighbor, current->time_point.position, sipp::neighbor_time_interval(current->time_point.interval));

      for (const TimeInterval& interval : time_intervals)
      {
        // std::cout << "interval:" << interval << std::endl;

        // skip intervals that start later then the maximum time to reach the goal
        if (interval.t_min > max_time) continue;

        // if the earliest reachable time of the neighbor is greater than maximum time of the current node + travel time, then the neighbor
        // is not reachable without collision
        if (current->time_point.interval.t_max != INT_MAX && interval.t_min > current->time_point.interval.t_max + 1) continue;

        // check whether known
        if (known.find(neighbor) != known.end() && interval.t_min <= known[neighbor]) continue;

        TimePoint neighbor_time_point(neighbor, interval);
        assertm(neighbor_time_point.interval.t_min > current->time_point.interval.t_min, "Invalid interval.");

        // add to the open list
        open_list.push(node_pool.add_node(SIPPNode(neighbor_time_point, neighbor_time_point.interval.t_min, h, current)));
        generated_nodes++;
      }
    }
  }
  // reset the node pool
  node_pool.merge_extra();

  return TimePointPath();
}

TimePointPath sipp::extract_path(SIPPNode const* const final_node)
{
  TimePointPath   path;
  SIPPNode const* curr = final_node;
  SIPPNode const* prev = nullptr;
  while (curr != nullptr)
  {
    if (prev == nullptr)  // push the whole last interval
    {
      path.push_back(curr->time_point);
    }
    else  // push only the occupied part of other intervals
    {
      path.push_back(
          TimePoint(curr->time_point.position, TimeInterval(curr->time_point.interval.t_min, prev->time_point.interval.t_min - 1)));
    }
    prev = curr;
    curr = curr->parent;
  }
  std::reverse(path.begin(), path.end());
  return path;
}

TimeInterval sipp::neighbor_time_interval(const TimeInterval& i)
{
  TimeInterval next_ti = i;
  if (next_ti.t_max == INT_MAX)
    next_ti.t_min += MOVE_TIME;  // increase only t_min, otherwise overflow would happen
  else
    next_ti = next_ti + MOVE_TIME;  // increase both t_min and t_max
  return next_ti;
}
