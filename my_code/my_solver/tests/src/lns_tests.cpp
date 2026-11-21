/*
 * Author: Jan Chleboun
 * Date: 12-03-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */


#include <limits>
#include <gtest/gtest.h>

#include <algorithm>
#include <climits>
#include <numeric>
#include <random>

#include "Instance.h"
#include "LNS.h"
#include "SafeIntervalTable.h"
#include "test_utils.h"


// test destroy operators
TEST(LNSDestroyOperators, RightNumberOfDestroyed)
{
  // load instance
  int                       agent_num = 50;
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance  = std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map",
                                                                  base_path + "/tests/test_scen/den520d-random-0.scen", agent_num);

  std::array<int, 5> neighborhood_sizes = {2, 4, 8, 16, 32};

  // find initial solution
  for (auto destroy_type : magic_enum::enum_values<DESTROY_TYPE>())
  {
    for (auto neighborhood_size : neighborhood_sizes)
    {
      auto         rnd_generator = std::mt19937(0);
      LNS_settings lns_settings(0, 5, {destroy_type, neighborhood_size}, {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0});
      LNS          lns(*instance, rnd_generator, nullptr, lns_settings);
      lns.find_initial_solution();
      ASSERT_TRUE(lns.solution.feasible) << "Initial solution is not feasible";
      ASSERT_TRUE(lns.solution.is_valid(*instance)) << "Initial solution is not valid";

      // destroy agents
      lns.destroy_operator.apply(lns.solution);

      // check if the right number of agents were destroyed
      EXPECT_TRUE(static_cast<int>(lns.solution.destroyed_paths.size()) <= neighborhood_size);
    }
  }
}

TEST(LNSDestroyRandomWalk, TabuList)
{
  // load instance
  int                       agent_num = 2;
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance  = std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map",
                                                                  base_path + "/tests/test_scen/den520d-random-0.scen", agent_num);

  auto         rnd_generator = std::mt19937(0);
  LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOMWALK, 1}, {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0});
  LNS          lns(*instance, rnd_generator, nullptr, lns_settings);
  lns.find_initial_solution();
  ASSERT_TRUE(lns.solution.feasible) << "Initial solution is not feasible";
  ASSERT_TRUE(lns.solution.is_valid(*instance)) << "Initial solution is not valid";
  // both agents delayed equally, so only the tabu list can make the second pick differ (with zero delays the
  // operator falls back to random destroy and the test would depend on the random stream)
  lns.solution.delays = {5, 5};

  // destroy agents
  lns.destroy_operator.apply(lns.solution);
  ASSERT_EQ(lns.solution.destroyed_paths.size(), 1);
  int destroyed_1 = lns.solution.destroyed_paths[0];

  // destroy again
  lns.destroy_operator.apply(lns.solution);
  ASSERT_EQ(lns.solution.destroyed_paths.size(), 1);
  int destroyed_2 = lns.solution.destroyed_paths[0];

  // check that the same agent was not destroyed twice
  EXPECT_NE(destroyed_1, destroyed_2);

  // check, that the tabu list reset
  lns.destroy_operator.apply(lns.solution);
  ASSERT_EQ(lns.solution.destroyed_paths.size(), 1);
}

namespace
{
auto start_goal_distances(const Instance& instance, const std::vector<int>& order) -> std::vector<int>
{
  std::vector<int> d;
  for (int a : order)
  {
    d.push_back(instance.get_heuristic_distance(a, instance.get_start_locations()[a]));
  }
  return d;
}
}  // namespace

TEST(LNSPPOrder, HeuristicOrdersAreSortedPermutations)
{
  int         agent_num = 50;
  std::string base_path = get_base_path_tests();
  Instance    instance(base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen",
                       agent_num);
  for (PP_ORDER order_type : {PP_ORDER::SHORTEST_FIRST, PP_ORDER::LONGEST_FIRST})
  {
    auto         rnd_generator = std::mt19937(0);
    LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 1}, {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0},
                              {PSIPP_implementation::PSIPP_mine, INFO_type::no_info, 1.0}, true, false, true, order_type);
    LNS          lns(instance, rnd_generator, nullptr, lns_settings);
    for (int attempt = 0; attempt < 3; attempt++)
    {
      std::vector<int> order = lns.pp_agent_order();
      std::vector<int> sorted_ids = order;
      std::sort(sorted_ids.begin(), sorted_ids.end());
      std::vector<int> expected_ids(agent_num);
      std::iota(expected_ids.begin(), expected_ids.end(), 0);
      ASSERT_EQ(sorted_ids, expected_ids) << "order is not a permutation";

      std::vector<int> d = start_goal_distances(instance, order);
      if (order_type == PP_ORDER::SHORTEST_FIRST)
      {
        EXPECT_TRUE(std::is_sorted(d.begin(), d.end()));
      }
      else
      {
        EXPECT_TRUE(std::is_sorted(d.rbegin(), d.rend()));
      }
    }
  }
}

TEST(LNSPPOrder, FirstOnlyFallsBackToRandom)
{
  int         agent_num = 50;
  std::string base_path = get_base_path_tests();
  Instance    instance(base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen",
                       agent_num);
  auto         rnd_generator = std::mt19937(0);
  LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 1}, {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0},
                            {PSIPP_implementation::PSIPP_mine, INFO_type::no_info, 1.0}, true, false, true,
                            PP_ORDER::LONGEST_FIRST, true);
  LNS          lns(instance, rnd_generator, nullptr, lns_settings);

  std::vector<int> first = start_goal_distances(instance, lns.pp_agent_order());
  EXPECT_TRUE(std::is_sorted(first.rbegin(), first.rend()));
  // with 50 agents a random permutation is sorted by distance with negligible probability
  std::vector<int> second = start_goal_distances(instance, lns.pp_agent_order());
  EXPECT_FALSE(std::is_sorted(second.rbegin(), second.rend()));
}

TEST(LNSPPOrder, RandomOrderUnchanged)
{
  // the default RANDOM order must consume the generator exactly like the original std::shuffle
  int         agent_num = 30;
  std::string base_path = get_base_path_tests();
  Instance    instance(base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen",
                       agent_num);
  auto         rnd_generator = std::mt19937(7);
  LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 1}, {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0});
  LNS          lns(instance, rnd_generator, nullptr, lns_settings);

  auto             reference_generator = std::mt19937(7);
  std::vector<int> expected(agent_num);
  std::iota(expected.begin(), expected.end(), 0);
  std::shuffle(expected.begin(), expected.end(), reference_generator);
  EXPECT_EQ(lns.pp_agent_order(), expected);
}

// LNS with the thesis configuration on a dense instance, where SIPP repairs often fail
static void run_dense_lns(bool psipp_repair, int seed, Logger& log_out, bool& valid, bool& planned_all,
                          bool repair_sipp_first = true,
                          double repair_push_cost = std::numeric_limits<double>::infinity())
{
  int         agent_num = 250;
  std::string base_path = get_base_path_tests();
  Instance    instance(base_path + "/tests/test_maps/random-32-32-20.map",
                       base_path + "/tests/test_scen/random-32-32-20-random-5.scen", agent_num);
  auto         rnd_generator = std::mt19937(seed);
  LNS_settings lns_settings(2000, 20, {DESTROY_TYPE::BLOCKED, 8},
                            {SIPP_implementation::SIPP_suboptimal_ap, INFO_type::no_info, 1.0, 5},
                            {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, 5}, true, true, true,
                            PP_ORDER::LONGEST_FIRST, true, psipp_repair, repair_sipp_first, repair_push_cost);
  LNS          lns(instance, rnd_generator, nullptr, lns_settings);
  lns.solve();
  ASSERT_TRUE(lns.found_initial_solution);
  valid       = lns.solution.is_valid(instance) && lns.get_constraint_manager().is_valid();
  planned_all = static_cast<int>(lns.already_planned.size()) == agent_num;
  log_out     = lns.log;
}

TEST(LNSRepair, FailedRepairRestoresAlreadyPlanned)
{
  Logger log;
  bool   valid = false, planned_all = false;
  run_dense_lns(false, 0, log, valid, planned_all);
  EXPECT_TRUE(valid);
  EXPECT_TRUE(planned_all) << "agents of a discarded repair are missing in already_planned";
}

TEST(LNSRepair, PSIPPRepairPushesAndStaysValid)
{
  Logger log;
  bool   valid = false, planned_all = false;
  run_dense_lns(true, 0, log, valid, planned_all);
  EXPECT_TRUE(valid);
  EXPECT_TRUE(planned_all);
  EXPECT_GT(log.repair_pushes, 0) << "no repair pushed an agent outside the neighborhood";
  EXPECT_GE(log.repair_pushed_agents, log.repair_pushes);
}

TEST(LNSRepair, DirectPSIPPRepairStaysValid)
{
  // PSIPP for every destroyed agent (no SIPP first), with infinite and with finite push cost
  for (double push_cost : {std::numeric_limits<double>::infinity(), 2.0})
  {
    Logger log;
    bool   valid = false, planned_all = false;
    run_dense_lns(true, 0, log, valid, planned_all, false, push_cost);
    EXPECT_TRUE(valid) << "push cost " << push_cost;
    EXPECT_TRUE(planned_all) << "push cost " << push_cost;
    EXPECT_GT(log.repair_pushes, 0) << "push cost " << push_cost;
  }
}
