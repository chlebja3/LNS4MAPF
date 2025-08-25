/**
 * @file
 * @brief Contains classes and functions related to the Push and SIPP pathfinding algorithm.
 *
 * @author: Jan Chleboun <chlebja3@fel.cvut.cz>
 * @date: 08-02-2025
 */
#pragma once
#include <utils.h>

#include <random>

// #include <boost/heap/pairing_heap.hpp>
// #include <boost/heap/d_ary_heap.hpp>
#include <boost/heap/priority_queue.hpp>
// #include <queue>
// #include <unordered_map>
#include <unordered_set>
// #include <boost/heap/fibonacci_heap.hpp>

#include "ConstraintManager.h"
#include "IterInfo.h"

/**
 * @brief Represents settings for the PSIPP algorithm.
 */
class PSIPP_settings
{
public:
  /**
   * @brief Constructor for PSIPP_settings.
   *
   * @param implementation_ The implementation which should be used.
   * @param info_type_ The type of information, that should be generated.
   * @param w_ The suboptimality factor.
   * @param p_ The parameter p for the Bounded Suboptimal PSIPP algorithm.
   */
  PSIPP_settings(PSIPP_implementation implementation_, INFO_type info_type_, double w_, int p_ = 1.0)
      : implementation(implementation_), info_type(info_type_), w(w_), p(p_)
  {
  }

  PSIPP_implementation implementation; /**< The implementation which should be used. */
  INFO_type info_type;                 /**< The type of information, that should be generated during the search. */
  double w;                            /**< The suboptimality factor. */
  double p;                            /**< The parameter p for the Bounded Suboptimal PSIPP algorithm. */
  int max_pushes = 1;                  // maximum of allowed pushes
  // bool                generate_blocked = false;
};

/**
 * @brief Namespace for PSIPP nodes.
 */
namespace psipp
{

/**
 * @brief Represents a node in the PSIPP algorithm.
 */
class PSIPPNode : public Node
{
public:
  TimePoint time_point;    /**< The time point (@see TimePoint) of the node. */
  PSIPPNode const* parent; /**< The parent node. */
  double h2;               /**< The heuristic value for the second heuristic. */
  double h3;               /**< The heuristic value for the third heuristic. */
  std::vector<Constraint> pushed;

  /**
   * @brief Constructor for PSIPPNode.
   *
   * @param location The location of the node.
   * @param interval The time interval of the node.
   * @param g_ The g-value of the node.
   * @param h_ The primary heuristic value of the node.
   * @param h2_ The secondary heuristic value.
   * @param h3_ The tertiary heuristic value.
   * @param parent_ The parent node.
   */
  PSIPPNode(int location, const TimeInterval& interval, double g_, double h_, double h2_, double h3_,
            PSIPPNode const* parent_, std::vector<Constraint>& pushed_)
      : Node(g_, h_), time_point(location, interval), parent(parent_), h2(h2_), h3(h3_), pushed(std::move(pushed_))
  {
  }

  PSIPPNode(int location, const TimeInterval& interval, double g_, double h_, double h2_, double h3_,
            PSIPPNode const* parent_)
      : Node(g_, h_), time_point(location, interval), parent(parent_), h2(h2_), h3(h3_)
  {
  }

  /**
   * @brief Constructor for PSIPPNode.
   *
   * @param time_point_ The time point of the node.
   * @param g_ The g-value of the node.
   * @param h_ The primary heuristic value of the node.
   * @param h2_ The secondary heuristic value.
   * @param h3_ The tertiary heuristic value.
   * @param parent_ The parent node.
   */
  PSIPPNode(const TimePoint& time_point_, double g_, double h_, double h2_, double h3_, PSIPPNode const* parent_,
            std::vector<Constraint>& pushed_)
      : Node(g_, h_), time_point(time_point_), parent(parent_), h2(h2_), h3(h3_), pushed(std::move(pushed_))
  {
  }

  /**
   * @brief Equality operator for PSIPPNode.
   *
   * @param other The other PSIPPNode to compare with.
   *
   * @return True if the nodes are equal, false otherwise.
   */
  auto operator==(const PSIPPNode& other) const -> bool;
};

/**
 * @brief Comparator class for PSIPPNodes
 */
class PSIPPNodeComparator
{
public:
  std::mt19937* rnd_generator;              /**< Random number generator. */
  mutable std::bernoulli_distribution dist; /**< Bernoulli distribution for generating random true/false values. */

  /**
   * @brief Constructor for PSIPPNodeComparator.
   *
   * @param rnd_generator_ Pointer to a random number generator.
   */
  explicit PSIPPNodeComparator(std::mt19937* rnd_generator_) : rnd_generator(rnd_generator_), dist(0.5) {}

  /**
   * @brief Comparison operator for PSIPPNode.
   *
   * @param n1 The first PSIPPNode to compare.
   * @param n2 The second PSIPPNode to compare.
   *
   * @return True if n1 is less than n2, false otherwise.
   */
  auto operator()(PSIPPNode const* n1, PSIPPNode const* n2) const -> bool;
};

/**
 * @brief Comparator class for Bounded Suboptimal PSIPP
 */
class PSIPPNodeComparatorSuboptimal
{
public:
  std::mt19937* rnd_generator;              /**< Random number generator. */
  mutable std::bernoulli_distribution dist; /**< Bernoulli distribution for generating random true/false values. */
  int max_suboptimality_absolute;           /**< Maximum absolute suboptimality. */

  /**
   * @brief Constructor for PSIPPNodeComparatorSuboptimal.
   *
   * @param rnd_generator_ Pointer to a random number generator.
   * @param max_suboptimality_absolute_ Maximum absolute suboptimality.
   */
  PSIPPNodeComparatorSuboptimal(std::mt19937* rnd_generator_, int max_suboptimality_absolute_)
      : rnd_generator(rnd_generator_), dist(0.5), max_suboptimality_absolute(max_suboptimality_absolute_)
  {
  }

  /**
   * @brief Comparison operator for PSIPPNode.
   *
   * @param n1 The first PSIPPNode to compare.
   * @param n2 The second PSIPPNode to compare.
   *
   * @return True if n1 is less than n2, false otherwise.
   */
  auto operator()(PSIPPNode const* n1, PSIPPNode const* n2) const -> bool;
};

/**
 * @brief Represents a pool of PSIPP nodes.
 * This class manages the allocation and deallocation of PSIPP nodes. Saves a lot of time. If there are not enough nodes
 * in the pool, it allocates more and stores them in a list, which is then merged into the pool at the end of the
 * iteration. Once a node is allocated, it is not deallocated until the pool is destroyed.
 */
class NodePool
{
public:
  /**
   * @brief Constructor for NodePool.
   *
   * @param size The initial size of the node pool.
   */
  explicit NodePool(int size);

  /**
   * @brief Add a node to the pool.
   *
   * @param node The PSIPPNode to add to the pool.
   *
   * @return Pointer to the added PSIPPNode.
   */
  auto add_node(PSIPPNode node) -> PSIPPNode*;

  /**
   * @brief Merge the extra nodes into the pool. Call this after a search algorithm is completed, to carry over the
   * extra nodes to the next run.
   */
  void merge_extra();

private:
  int end = 0;                 /**< The end index of the pool. Nodes after end are empty. */
  std::vector<PSIPPNode> pool; /**< The pool of SIPP nodes. */
  std::list<PSIPPNode> extra;  /**< The list of extra SIPP nodes. */
};

// using PriorityQueue = std::priority_queue<const SIPPNode*, std::vector<const SIPPNode*>, SIPPNodeComparator>;

/**
 * @brief Priority queue for SIPP nodes.
 */
using PriorityQueue = boost::heap::priority_queue<const PSIPPNode*, boost::heap::compare<PSIPPNodeComparator>>;

// using PriorityQueue = boost::heap::d_ary_heap<const SIPPNode*, boost::heap::arity<2>,
// boost::heap::compare<SIPPNodeComparator>>;

/**
 * @brief Priority queue for suboptimal SIPP nodes.
 */
using PriorityQueueSuboptimal =
    boost::heap::priority_queue<const PSIPPNode*, boost::heap::compare<PSIPPNodeComparatorSuboptimal>>;

// using PriorityQueue = boost::heap::fibonacci_heap<const SIPPNode*,
// boost::heap::compare<SIPPNode::compare_node_ptrs>>; using PriorityQueue = boost::heap::pairing_heap<const SIPPNode*,
// boost::heap::compare<SIPPNode::compare_node_ptrs>>; typedef std::unordered_set<TimePoint, TimePoint::hash_time_point>
// UnorderedSet; typedef std::unordered_map<Point2d, int, Point2d::hash_point>     UnorderedMap;

/**
 * @brief Extracts the path from the final node to the start node.
 *
 * @param final_node The final node of the path.
 *
 * @return A vector of TimePoints representing the path from the start node to the final node.
 */
auto extract_path(PSIPPNode const* final_node) -> TimePointPath;

}  // namespace psipp

/**
 * @brief Class representing the PSIPP algorithm.
 */
class PSIPP
{
public:
  PSIPP(const Instance& instance_, std::mt19937& rnd_generator_, const PSIPP_settings& settings_,
        const ConstraintManager& constraint_manager_);

  /**
   * @brief Plans a path for the given agent number.
   *
   * @param agent_num The agent number to plan the path for.
   * @param already_planned A set of already planned agents.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan(int agent_num, const std::unordered_set<int>& already_planned)
      -> std::pair<TimePointPath, std::unordered_set<int>>;

  /**
   * @brief Plans a path for the given agent number using the multi-heuristic version of the SIPP algorithm.
   *
   * @param agent_num The agent number to plan the path for.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan_psipp_mine(int agent_num) -> std::pair<TimePointPath, std::unordered_set<int>>;

  /**
   * @brief Plans a path for the given agent number using the ap multi-heuristic version of the SIPP algorithm.
   *
   * @param agent_num The agent number to plan the path for.
   * @param already_planned A set of already planned agents.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan_psipp_mine_ap(int agent_num, const std::unordered_set<int>& already_planned)
      -> std::pair<TimePointPath, std::unordered_set<int>>;

  /**
   * @brief Plans a path for the given agent number using the original Bounded Suboptimal SIPP algorithm.
   *
   * @param agent_num The agent number to plan the path for.
   * @param already_planned A set of already planned agents.
   * @param w The suboptimality factor.
   * @param ap Whether to use the ap version.
   *
   * @return A TimePointPath representing the planned path for the agent.
   */
  auto plan_suboptimal(int agent_num, const std::unordered_set<int>& already_planned, double w = 1.0, bool ap = false)
      -> std::pair<TimePointPath, std::unordered_set<int>>;

  [[nodiscard]] auto get_generated_all_iter() const -> int
  {
    return generated_all_iter;
  }

  [[nodiscard]] auto get_expanded_all_iter() const -> int
  {
    return expanded_all_iter;
  }

  [[nodiscard]] auto get_iter_info() -> PSIPPInfo
  {
    PSIPPInfo info = std::move(iter_info);
    iter_info.clear();
    return info;
  }

private:
  /**
   * @brief Initialize the information about the iteration. Reset the number of generated and expanded nodes and the
   * iteration number.
   */
  void initialize_iter_info();

  /**
   * @brief Update the iteration information.
   *
   * @param expanded The expanded node.
   * @param last_iter Whether this is the last iteration. If so, the alltime numbers of generated and expanded nodes are
   * updated.
   */
  void update_iter_info(const psipp::PSIPPNode& expanded, bool last_iter);

  const Instance& instance;       /**< The instance of the problem. */
  psipp::NodePool node_pool;      /**< The pool of SIPP nodes. */
  std::vector<int> known_max;     /**< The known maximum time for each node. */
  std::vector<int> known_min;     /**< The known minimum time for each node. */
  std::mt19937& rnd_generator;    /**< The random number generator. */
  const PSIPP_settings& settings; /**< The settings for the SIPP algorithm. */
  const ConstraintManager& constraint_manager;
  int generated_all_iter = 0;  /**< The number of nodes generated in all iterations. */
  int generated_this_iter = 0; /**< The number of nodes generated in the current iteration. */
  int expanded_all_iter = 0;   /**< The number of nodes expanded in all iterations. */
  int expanded_this_iter = 0;  /**< The number of nodes expanded in the current iteration. */
  int iteration_num = 0;       /**< The current iteration number. */
  PSIPPInfo iter_info;         /**< The iteration information. */
};
