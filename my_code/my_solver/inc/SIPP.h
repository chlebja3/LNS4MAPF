/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#pragma once
#include <utils.h>

// #include <boost/heap/pairing_heap.hpp>
#include <boost/heap/d_ary_heap.hpp>
// #include <boost/heap/priority_queue.hpp>
// #include <queue>
// #include <unordered_map>
// #include <unordered_set>
// #include <boost/heap/fibonacci_heap.hpp>

#include "ConstraintTable.h"
#include "IterInfo.h"

namespace sipp
{

class SIPPNode : public Node
{
public:
  TimePoint       time_point;
  SIPPNode const* parent;


  // constructors
  SIPPNode(int location, const TimeInterval& interval, double g_, double h_, double h2_, SIPPNode const* parent_);

  SIPPNode(const TimePoint& time_point_, double g_, double h_, double h2_, SIPPNode const* parent_);

  auto operator==(const SIPPNode& other) const -> bool;

  struct compare_node_ptrs
  {
    auto operator()(SIPPNode const* n1, SIPPNode const* n2) const -> bool;
  };

  struct compare_node_ptrs_suboptimal
  {
    auto operator()(SIPPNode const* n1, SIPPNode const* n2) const -> bool;
  };

  double h2;
};

class NodePool
{
public:
  explicit NodePool(int size);
  auto add_node(SIPPNode node) -> SIPPNode*;
  void merge_extra();

private:
  int                   end = 0;
  std::vector<SIPPNode> pool;
  std::list<SIPPNode>   extra;
};


// using PriorityQueue = std::priority_queue<const SIPPNode*, std::vector<const SIPPNode*>, SIPPNode::compare_node_ptrs>;
// using PriorityQueue = boost::heap::priority_queue<const SIPPNode*, boost::heap::compare<SIPPNode::compare_node_ptrs>>;
using PriorityQueue = boost::heap::d_ary_heap<const SIPPNode*, boost::heap::arity<2>, boost::heap::compare<SIPPNode::compare_node_ptrs>>;
using PriorityQueueSuboptimal =
    boost::heap::d_ary_heap<const SIPPNode*, boost::heap::arity<2>, boost::heap::compare<SIPPNode::compare_node_ptrs_suboptimal>>;
// using PriorityQueue = boost::heap::fibonacci_heap<const SIPPNode*, boost::heap::compare<SIPPNode::compare_node_ptrs>>;
// using PriorityQueue = boost::heap::pairing_heap<const SIPPNode*, boost::heap::compare<SIPPNode::compare_node_ptrs>>;
// typedef std::unordered_set<TimePoint, TimePoint::hash_time_point> UnorderedSet;
// typedef std::unordered_map<Point2d, int, Point2d::hash_point>     UnorderedMap;

auto extract_path(SIPPNode const* final_node) -> TimePointPath;

}  // namespace sipp

class SIPP
{
public:
  ConstraintTable constraint_table;
  int             generated_all_iter  = 0;
  int             generated_this_iter = 0;
  int             expanded_all_iter   = 0;
  int             expanded_this_iter  = 0;
  int             iteration_num       = 0;
  SIPPInfo        iter_info;

  bool generate_iter_info = false;

  // constructor
  SIPP(const Instance& instance, bool generate_iter_info);

  // destructor
  ~SIPP() = default;

  auto plan(int agent_num) -> TimePointPath;

private:
  void             initialize_iter_info();
  void             update_iter_info(const sipp::SIPPNode& expanded, bool last_iter);
  const Instance&  instance;
  sipp::NodePool   node_pool;
  std::vector<int> known_max;  // known node database
  std::vector<int> known_min;  // known node database
};
