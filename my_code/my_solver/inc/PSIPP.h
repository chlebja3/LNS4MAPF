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
   * @param max_pushes_ The maximum number of allowed pushes.
   * @param push_cost_ Cost of one push in node ordering; infinity = pushes compared first.
   * @param closed_list_per_push_count_ Whether PSIPP_mine_ap keeps a separate closed list for each number of pushes.
   */
  PSIPP_settings(PSIPP_implementation implementation_, INFO_type info_type_, double w_, int p_ = 1.0,
                 int max_pushes_ = 1, double push_cost_ = std::numeric_limits<double>::infinity(),
                 bool closed_list_per_push_count_ = false)
      : implementation(implementation_),
        info_type(info_type_),
        w(w_),
        p(p_),
        max_pushes(max_pushes_),
        push_cost(push_cost_),
        closed_list_per_push_count(closed_list_per_push_count_)
  {
  }

  PSIPP_implementation implementation; /**< The implementation which should be used. */
  INFO_type info_type;                 /**< The type of information, that should be generated during the search. */
  double w;                            /**< The suboptimality factor. */
  double p;                            /**< The parameter p for the Bounded Suboptimal PSIPP algorithm. */
  int max_pushes;                      // maximum of allowed pushes
  double push_cost;                    /**< Cost of one push in node ordering; infinity = pushes compared first. */
  bool closed_list_per_push_count;     /**< PSIPP_mine_ap: separate closed list per number of pushes (capped at
                                            max_pushes). */
  // bool                generate_blocked = false;
};

/**
 * @brief Namespace for PSIPP nodes.
 */
namespace psipp
{

/**
 * @brief One pushed constraint in a PushList; links to the constraints pushed before it.
 */
struct PushNode
{
  Constraint constraint; /**< The pushed constraint. */
  const PushNode* prev;  /**< The previously pushed constraint, nullptr for the first one. */
};

/**
 * @brief Immutable list of pushed constraints shared between a node and its successors. Copying is O(1); extending
 * allocates one PushNode in the PushArena, so a successor never copies the constraints of its parent.
 */
class PushList
{
public:
  class iterator
  {
  public:
    explicit iterator(const PushNode* node_) : node(node_) {}
    auto operator*() const -> const Constraint& { return node->constraint; }
    auto operator->() const -> const Constraint* { return &node->constraint; }
    auto operator++() -> iterator&
    {
      node = node->prev;
      return *this;
    }
    auto operator!=(const iterator& other) const -> bool { return node != other.node; }

  private:
    const PushNode* node;
  };

  PushList() = default;
  PushList(const PushNode* head_, int count_) : head(head_), count(count_) {}

  /**
   * @return The number of pushed constraints.
   */
  auto size() const -> size_t { return static_cast<size_t>(count); }

  /**
   * @return Iterator from the most recently pushed constraint to the first one.
   */
  auto begin() const -> iterator { return iterator(head); }
  auto end() const -> iterator { return iterator(nullptr); }

private:
  const PushNode* head = nullptr; /**< The most recently pushed constraint. */
  int count = 0;                  /**< Number of constraints, stored here so that comparing nodes reads no list. */
  friend class PushArena;
};

/**
 * @brief Owns the PushNodes of one search. Nodes are stored in chunks of fixed capacity, so their addresses stay
 * valid until reset(), which keeps the memory for the next search.
 */
class PushArena
{
public:
  /**
   * @brief Extend a list by one constraint.
   *
   * @param list The list to extend; it is not modified.
   * @param constraint The pushed constraint.
   *
   * @return The list with the constraint appended.
   */
  auto push(PushList list, const Constraint& constraint) -> PushList;

  /**
   * @brief Invalidate all lists created so far. Call at the start of each search.
   */
  void reset();

private:
  static constexpr size_t CHUNK_SIZE = 4096;
  std::vector<std::vector<PushNode>> chunks; /**< Chunks with reserved capacity CHUNK_SIZE (never reallocated). */
  size_t chunk_idx = 0;                      /**< The chunk currently being filled. */
};

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
  PushList pushed;        /**< The constraints pushed on the way to this node. */
  std::uint32_t tie = 0;   /**< Random tie-break key, drawn once when the node enters the pool. */

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
            PSIPPNode const* parent_, PushList pushed_)
      : Node(g_, h_), time_point(location, interval), parent(parent_), h2(h2_), h3(h3_), pushed(pushed_)
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
            PushList pushed_)
      : Node(g_, h_), time_point(time_point_), parent(parent_), h2(h2_), h3(h3_), pushed(pushed_)
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
  double push_cost;                         /**< Cost of one push; infinity = lexicographic on number of pushes. */

  /**
   * @brief Constructor for PSIPPNodeComparator.
   *
   * @param push_cost_ Cost of one push added to f (>= 0); infinity compares the number of pushes first.
   */
  explicit PSIPPNodeComparator(double push_cost_ = std::numeric_limits<double>::infinity()) : push_cost(push_cost_)
  {
    assertm(!(push_cost < 0) && !std::isnan(push_cost), "push_cost must be >= 0");
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
 * @brief Comparator class for Bounded Suboptimal PSIPP
 */
class PSIPPNodeComparatorSuboptimal
{
public:
  int max_suboptimality_absolute;           /**< Maximum absolute suboptimality. */

  /**
   * @brief Constructor for PSIPPNodeComparatorSuboptimal.
   *
   * @param max_suboptimality_absolute_ Maximum absolute suboptimality.
   */
  explicit PSIPPNodeComparatorSuboptimal(int max_suboptimality_absolute_)
      : max_suboptimality_absolute(max_suboptimality_absolute_)
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
   * @param rnd_generator_ Random number generator used to draw the tie-break key of every added node.
   */
  NodePool(int size, std::mt19937& rnd_generator_);

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
  std::mt19937* rnd_generator; /**< Draws the tie-break keys. */
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

  /**
   * @brief Reset the known nodes database (closed list), only the layers that could have been written.
   */
  void reset_known();

  /**
   * @brief Index into the known nodes database.
   *
   * @param location The location.
   * @param num_pushed The number of pushes of the node (ignored unless closed_list_per_push_count is set).
   * @return location + num_cells * layer, where layer = min(num_pushed, max_pushes) if per push count, else 0.
   */
  auto known_index(int location, int num_pushed) const -> int;

  const Instance& instance;       /**< The instance of the problem. */
  psipp::NodePool node_pool;      /**< The pool of SIPP nodes. */
  psipp::PushArena push_arena;    /**< Storage of the pushed lists of the current search. */
  std::vector<int> known_max;     /**< The known maximum time for each node. */
  std::vector<int> known_min;     /**< The known minimum time for each node. */
  int known_layers_dirty = 1;     /**< Number of closed-list layers (per push count) possibly written since reset. */
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
