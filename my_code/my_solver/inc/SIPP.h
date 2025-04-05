/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#pragma once
#include <utils.h>

#include <random>

// #include <boost/heap/pairing_heap.hpp>
// #include <boost/heap/d_ary_heap.hpp>
#include <boost/heap/priority_queue.hpp>
// #include <queue>
// #include <unordered_map>
// #include <unordered_set>
// #include <boost/heap/fibonacci_heap.hpp>

#include "IterInfo.h"
#include "SafeIntervalTable.h"

class SIPP_settings
{
public:
  SIPP_settings(SIPP_implementation implementation_, INFO_type info_type_, double w_)
      : implementation(implementation_), info_type(info_type_), w(w_)
  {
  }

  SIPP_implementation implementation;
  INFO_type           info_type;
  double              w;
};

namespace sipp
{

class SIPPNode : public Node
{
public:
  TimePoint       time_point;
  SIPPNode const* parent;
  double          h2;
  double          h3;


  // constructors
  SIPPNode(int location, const TimeInterval& interval, double g_, double h_, double h2_, double h3_, SIPPNode const* parent_)
      : Node(g_, h_), time_point(location, interval), parent(parent_), h2(h2_), h3(h3_)
  {
  }

  SIPPNode(const TimePoint& time_point_, double g_, double h_, double h2_, double h3_, SIPPNode const* parent_)
      : Node(g_, h_), time_point(time_point_), parent(parent_), h2(h2_), h3(h3_)
  {
  }

  auto operator==(const SIPPNode& other) const -> bool;
};

class SIPPNodeComparator
{
public:
  std::mt19937*                       rnd_generator;
  mutable std::bernoulli_distribution dist;

  explicit SIPPNodeComparator(std::mt19937* rnd_generator_) : rnd_generator(rnd_generator_), dist(0.5)
  {
  }

  auto operator()(SIPPNode const* n1, SIPPNode const* n2) const -> bool;
};


class SIPPNodeComparatorSuboptimal
{
public:
  std::mt19937*                       rnd_generator;
  mutable std::bernoulli_distribution dist;
  int                                 max_suboptimality_absolute;

  SIPPNodeComparatorSuboptimal(std::mt19937* rnd_generator_, int max_suboptimality_absolute_)
      : rnd_generator(rnd_generator_), dist(0.5), max_suboptimality_absolute(max_suboptimality_absolute_)
  {
  }

  auto operator()(SIPPNode const* n1, SIPPNode const* n2) const -> bool;
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


// using PriorityQueue = std::priority_queue<const SIPPNode*, std::vector<const SIPPNode*>, SIPPNodeComparator>;
using PriorityQueue = boost::heap::priority_queue<const SIPPNode*, boost::heap::compare<SIPPNodeComparator>>;
// using PriorityQueue = boost::heap::d_ary_heap<const SIPPNode*, boost::heap::arity<2>, boost::heap::compare<SIPPNodeComparator>>;
using PriorityQueueSuboptimal = boost::heap::priority_queue<const SIPPNode*, boost::heap::compare<SIPPNodeComparatorSuboptimal>>;
// using PriorityQueue = boost::heap::fibonacci_heap<const SIPPNode*, boost::heap::compare<SIPPNode::compare_node_ptrs>>;
// using PriorityQueue = boost::heap::pairing_heap<const SIPPNode*, boost::heap::compare<SIPPNode::compare_node_ptrs>>;
// typedef std::unordered_set<TimePoint, TimePoint::hash_time_point> UnorderedSet;
// typedef std::unordered_map<Point2d, int, Point2d::hash_point>     UnorderedMap;

auto extract_path(SIPPNode const* final_node) -> TimePointPath;

}  // namespace sipp

class SIPP
{
public:
  SafeIntervalTable safe_interval_table;
  int               generated_all_iter  = 0;
  int               generated_this_iter = 0;
  int               expanded_all_iter   = 0;
  int               expanded_this_iter  = 0;
  int               iteration_num       = 0;
  SIPPInfo          iter_info;

  // constructor
  SIPP(const Instance& instance_, std::mt19937& rnd_generator_, const SIPP_settings& settings_);

  // destructor
  ~SIPP() = default;

  auto plan(int agent_num, const std::unordered_set<int>& already_planned) -> TimePointPath;
  auto plan_sipp_mine(int agent_num, const std::unordered_set<int>& already_planned) -> TimePointPath;
  auto plan_mapflns_heuristic(int agent_num) -> TimePointPath;
  auto plan_suboptimal(int agent_num, const std::unordered_set<int>& already_planned, double w = 1.0) -> TimePointPath;

private:
  void                 initialize_iter_info();
  void                 update_iter_info(const sipp::SIPPNode& expanded, bool last_iter);
  const Instance&      instance;
  sipp::NodePool       node_pool;
  std::vector<int>     known_max;  // known node database
  std::vector<int>     known_min;  // known node database
  std::mt19937&        rnd_generator;
  const SIPP_settings& settings;
};
