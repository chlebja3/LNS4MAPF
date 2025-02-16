/*
 * Author: Jan Chleboun
 * Date: 09-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description: Implementation of the SIPP algorithm for path planning in a grid map. The algorithm is based on the A* search algorithm, but
 * searches also in the time dimension. To reduce dimensionality of the space, the time is discretized into intervals.
 */

#include "SIPP.h"

#include <math.h>

#include <memory>
#include <queue>
#include <unordered_set>

#include "utils.h"
SIPP::SIPP(const std::shared_ptr<Instance> instance) : map_data(instance->get_map_data()), constraint_table(ConstraintTable(instance)){};
SIPP::SIPP(const Map& map_data, const ConstraintTable& constraint_table) : map_data(map_data), constraint_table(constraint_table){};


class SIPPNode : public Node
{
public:
  TimePoint                 time_point;
  std::shared_ptr<SIPPNode> parent;


  // constructors
  SIPPNode(const int x, const int y, const int time_min, const int time_max, const double g, const double h,
           const std::shared_ptr<SIPPNode> parent)
      : Node(g, h), time_point(TimePoint(x, y, time_min, time_max)), parent(parent){};

  SIPPNode(const Point2d& position, const TimeInterval& interval, const double g, const double h, const std::shared_ptr<SIPPNode> parent)
      : Node(g, h), time_point(position, interval), parent(parent){};

  SIPPNode(const TimePoint& time_point, const double g, const double h, const std::shared_ptr<SIPPNode> parent)
      : Node(g, h), time_point(time_point), parent(parent){};

  bool operator==(const SIPPNode& other) const
  {
    return g == other.g && h == other.h && time_point == other.time_point;
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
};

namespace sipp
{
typedef std::priority_queue<std::shared_ptr<SIPPNode>, std::vector<std::shared_ptr<SIPPNode>>, SIPPNode::compare_node_ptrs> PriorityQueue;
typedef std::unordered_set<TimePoint, TimePoint::hash_time_point>                                                           UnorderedSet;

TimePointPath extract_path(const std::shared_ptr<SIPPNode>& final_node);
}  // namespace sipp

TimePointPath SIPP::plan(const Point2d& start, const Point2d& goal)
{
  // check whether start and goal inside the map
  if (!map_data.is_in(start) || !map_data.is_in(goal)) return TimePointPath();

  // check whether start and goal not obstructed
  if (map_data.index(start) == 1 || map_data.index(goal) == 1) return TimePointPath();

  // create openlist
  sipp::PriorityQueue open_list;

  // calculate the heuristic
  double h = std::hypot(goal.x - start.x, goal.y - start.y);

  // retrieve the first safe interval
  TimeInterval time_interval_start = constraint_table.get_first_safe_interval(start);

  // create the start node
  open_list.push(std::make_shared<SIPPNode>(start, time_interval_start, 0, h, nullptr));

  // create known nodes database
  sipp::UnorderedSet known;

  // main loop
  while (!open_list.empty())
  {
    // pop the node to be processed
    std::shared_ptr<SIPPNode> current = std::move(open_list.top());
    open_list.pop();

    // check whether node is in known set
    if (known.find(current->time_point) != known.end()) continue;


    // check whether goal is reached and that the goal will not be obstructed in future
    if (current->time_point.position == goal)
    {
      // extract the path
      return sipp::extract_path(current);
    }

    // add to known nodes
    known.insert(current->time_point);

    // expand current node
    std::vector<Point2d> neighbors = map_data.find_neighbors(current->time_point.position);
    for (Point2d& neighbor : neighbors)
    {
      // calculate the heuristic
      double h = std::hypot(goal.x - neighbor.x, goal.y - neighbor.y);

      // retrieve the safe intervals for the neighbor
      std::vector<TimeInterval> time_intervals =
          constraint_table.get_safe_intervals(neighbor, current->time_point.position, current->time_point.interval + 1);

      for (const TimeInterval& interval : time_intervals)
      {
        // if the earliest reachable time of the neighbor is greater than maximum time of the current node + travel time, then the neighbor
        // is not reachable without collision
        if (interval.t_min > current->time_point.interval.t_max + 1) continue;

        TimePoint neighbor_time_point(neighbor, interval);

        // check whether known
        if (known.find(neighbor_time_point) != known.end()) continue;

        // add to the open list
        open_list.push(std::make_shared<SIPPNode>(neighbor_time_point, current->g + 1, h, current));
      }
    }
  }
  return TimePointPath();
}

TimePointPath sipp::extract_path(const std::shared_ptr<SIPPNode>& final_node)
{
  TimePointPath             path;
  std::shared_ptr<SIPPNode> curr = final_node;
  while (curr != nullptr)
  {
    path.push_back(curr->time_point);
    curr = curr->parent;
  }
  std::reverse(path.begin(), path.end());
  return path;
}
