/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#pragma once
#include <utils.h>

// #include <boost/heap/pairing_heap.hpp>
// #include <boost/heap/priority_queue.hpp>
#include <queue>
#include <unordered_set>

#include "ConstraintTable.h"

namespace sipp
{

class SIPPNode : public Node
{
public:
  TimePoint       time_point;
  SIPPNode const* parent;


  // constructors
  SIPPNode(const int x, const int y, const int time_min, const int time_max, const double g, const double h, SIPPNode const* const parent);

  SIPPNode(const Point2d& position, const TimeInterval& interval, const double g, const double h, SIPPNode const* const parent);

  SIPPNode(const TimePoint& time_point, const double g, const double h, SIPPNode const* const parent);

  bool operator==(const SIPPNode& other) const;

  struct compare_node_ptrs
  {
    bool operator()(SIPPNode const* n1, SIPPNode const* n2) const;
  };
};

class NodePool
{
public:
  NodePool(const int size);
  SIPPNode* add_node(SIPPNode node);
  void      merge_extra(void);

private:
  int                   end = 0;
  std::vector<SIPPNode> pool;
  std::list<SIPPNode>   extra;
};


typedef std::priority_queue<SIPPNode const*, std::vector<SIPPNode const*>, SIPPNode::compare_node_ptrs> PriorityQueue;
// typedef boost::heap::priority_queue<SIPPNode const*, boost::heap::compare<SIPPNode::compare_node_ptrs>> PriorityQueue;
// typedef boost::heap::pairing_heap<SIPPNode const*, boost::heap::compare<SIPPNode::compare_node_ptrs>> PriorityQueue;
typedef std::unordered_set<TimePoint, TimePoint::hash_time_point> UnorderedSet;
typedef std::unordered_map<Point2d, int, Point2d::hash_point>     UnorderedMap;

TimeInterval  neighbor_time_interval(const TimeInterval& i);
TimePointPath extract_path(SIPPNode const* const final_node);

}  // namespace sipp

class SIPP
{
public:
  ConstraintTable constraint_table;
  int             generated_nodes = 0;

  // constructor
  SIPP(const Instance& instance);

  // destructor
  ~SIPP() = default;

  TimePointPath plan(const int agent_num);

private:
  const Instance& instance;
  sipp::NodePool  node_pool;
};
