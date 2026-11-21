/*
 * Author: Jan Chleboun
 * Date: 16-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gtest/gtest.h>
#include <utils.h>

#include <climits>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <utility>
#include <vector>

#include "ConstraintManager.h"
#include "Instance.h"
#include "LNS.h"
#include "PSIPP.h" #include "magic_enum/magic_enum.hpp"
#include "test_utils.h"

// SIPP tests

// Basic pathfinding in an empty map
TEST(PSIPPTest, BasicPathEmptyMap)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create PSIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings psipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP psipp(*instance, rnd_generator, psipp_settings, constraint_manager);

    // from (0,0) to (4,4)
    auto [path, pushed_agents] = psipp.plan(0, {});

    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";
    ASSERT_EQ(path.size(), 9) << "SIPP failed to find a basic path!";
    EXPECT_EQ(path.front().location, instance->get_start_locations()[0]) << "Path does not start at the correct point!";
    EXPECT_EQ(path.back().location, instance->get_goal_locations()[0]) << "Path does not end at the correct point!";
  }
}

// Pathfinding with static obstacles
TEST(PSIPPTest, StaticObstacleAvoidance)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/wall_5_5.map", base_path + "/tests/test_scen/wall_5_5_scen_1.scen", 1);

  // create PSIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings sipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP sipp(*instance, rnd_generator, sipp_settings, constraint_manager);

    // from (0,0) to (4,4)
    auto [path, pushed_agents] = sipp.plan(0, {});

    // check that the path size is correct
    ASSERT_EQ(path.size(), 9) << "SIPP failed to find a path around obstacles!";
    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";

    // check that the path starts and end at the correct points
    EXPECT_EQ(path.front().location, instance->get_start_locations()[0]) << "Path does not start at the correct point!";
    EXPECT_EQ(path.back().location, instance->get_goal_locations()[0]) << "Path does not end at the correct point!";

    // check that no obstacles are in the path
    for (const auto& tp : path)
    {
      EXPECT_TRUE(instance->get_map_data().index(tp.location) == 0) << "Path goes through an obstacle!";
    }
  }
}

// Handling of time constraints (SIPP-specific test)**
TEST(PSIPPTest, TimeConstraintHandling)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create PSIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings psipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP psipp(*instance, rnd_generator, psipp_settings, constraint_manager);

    // Create a wall from time constraints
    for (int i = 0; i < 5; i++)
    {
      if (i == 2)
      {
        continue;  // passage through (2,2)
      }
      TimePointPath obstacle = {TimePoint(instance->position_to_location({i, 2}), {0, 20})};
      constraint_manager.add_trajectory(obstacle, 0);
    }

    auto [path, pushed_agents] = psipp.plan(0, {});

    ASSERT_EQ(path.size(), 9) << "Path length invalid!";
    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";

    // check that the path does not go through the blocked area
    for (const auto& tp : path)
    {
      if (instance->location_to_position(tp.location).y == 2)
      {
        EXPECT_EQ(instance->location_to_position(tp.location).x, 2) << "Path goes through a blocked area!";
      }
    }

    // check the completion time of the path
    EXPECT_EQ(path.back().interval.t_min, 8) << "Path does not complete in the expected time!";
  }
}

// No possible path due to obstacles
TEST(PSIPPTest, NoPathPossible)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/biparted_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings psipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP psipp(*instance, rnd_generator, psipp_settings, constraint_manager);

    auto [path, pushed_agents] = psipp.plan(0, {});

    ASSERT_TRUE(path.empty())
        << "PSIPP should return an empty path when no path exists!";  // TODO can PSIPP find a path?
    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";
  }
}

// No path due to time constraints**
TEST(PSIPPTest, NoPathDueToTimeConstraints)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 4);

  // create PSIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings sipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP psipp(*instance, rnd_generator, sipp_settings, constraint_manager);

    // Add constraints that make it impossible to leave start until time 10
    constraint_manager.add_trajectory({TimePoint(1, {0, 10})}, 0);  // {1, 0}
    constraint_manager.add_trajectory({TimePoint(5, {0, 10})}, 1);  // {0, 1}

    // add constraints that make it impossible to reach the goal from time 10 to 20
    TimePointPath obstacle = {TimePoint(instance->position_to_location({3, 4}), {10, 20})};
    constraint_manager.add_trajectory(obstacle, 2);  // {3, 4}
    TimePointPath obstacle2 = {TimePoint(instance->position_to_location({4, 3}), {10, 20})};
    constraint_manager.add_trajectory(obstacle2, 3);  // {4, 3}

    // plan path
    auto [path, pushed_agents] = psipp.plan(0, {});

    // check that path was found
    ASSERT_FALSE(path.empty()) << "PSIPP returned no path!";
    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";

    // check that the path does not leave the start until time 10
    for (const auto& tp : path)
    {
      if (tp.location != instance->get_start_locations()[0])
      {
        EXPECT_TRUE(tp.interval.t_min > 10) << "Path leaves start before time 10!";
      }
    }

    // check that the path does not reach the goal before time 21
    for (const auto& tp : path)
    {
      if (tp.location == instance->get_goal_locations()[0])
      {
        EXPECT_TRUE(tp.interval.t_min > 20) << "Path reaches goal before or at time 20!";
      }
    }
  }
}

// dynamic obstacle at the goal after time of possible arrival**
TEST(PSIPPTest, DynamicObstacleAtGoal)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create PSIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings psipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP psipp(*instance, rnd_generator, psipp_settings, constraint_manager);

    // add a constraint that blocks the goal after time 20
    TimePointPath obstacle = {TimePoint(instance->get_goal_locations()[0], {20, 30})};
    constraint_manager.add_trajectory(obstacle, 0);

    auto [path, pushed_agents] = psipp.plan(0, {});

    // the path should be found
    ASSERT_FALSE(path.empty()) << "SIPP should find a path when the goal is blocked after possible arrival time!";
    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";

    // check that the path reaches the goal at time 31
    EXPECT_EQ(path.back().interval.t_min, 31) << "Path does not reach the goal at the expected time!";
  }
}

// test that PSIPP takes into account the edge constraints
// Edge constraints at the goal
TEST(PSIPPTest, EdgeConstraintsAtGoal)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_2.scen", 1);

  // create SIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings psipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP psipp(*instance, rnd_generator, psipp_settings, constraint_manager);

    // from (2, 0) to (2, 1)

    // add edge constraints at the goal so that the agent has to go around
    TimePointPath obstacle = {TimePoint(5, {0, 0}), TimePoint(2, {1, 1}), TimePoint(1, {2, 2}), TimePoint(0, {3, 3}),
                              TimePoint(3, {4, 4}), TimePoint(6, {5, 5}), TimePoint(7, {6, 6}), TimePoint(8, {7, 7})};
    constraint_manager.add_trajectory(obstacle, 0);

    auto [path, pushed_agents] = psipp.plan(0, {});

    // check that the path has the right length
    ASSERT_EQ(path.size(), 8) << "SIPP should find a path.";
    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";

    // check that the path is correct
    EXPECT_EQ(path, TimePointPath({TimePoint(2, {0, 0}), TimePoint(1, {1, 1}), TimePoint(0, {2, 2}),
                                   TimePoint(3, {3, 3}), TimePoint(6, {4, 4}), TimePoint(7, {5, 5}),
                                   TimePoint(8, {6, 6}), TimePoint(5, {7, INT_MAX})}));
  }
}

// PSIPP plans the same path as A* when no constraints are present
TEST(PSIPPTest, SamePathAsAStar1)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // create SIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings psipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP psipp(*instance, rnd_generator, psipp_settings, constraint_manager);

    // from (0,0) to (4,4)

    // plan path by ASTAR
    Path path_astar = find_path_distance_gradient(0, *instance);

    // assert that astar found a path
    ASSERT_FALSE(path_astar.empty()) << "A* should find a path when no constraints are present!";

    // plan path by SIPP
    auto [path_psipp, pushed_agents] = psipp.plan(0, {});

    // assert that SIPP found a path
    ASSERT_FALSE(path_psipp.empty()) << "PSIPP should find a path when no constraints are present!";
    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";

    // convert SIPP path to the same format as A*
    Path path_psipp_converted = timepointpath_to_path(path_psipp);

    // check that the paths have the same length
    EXPECT_EQ(path_psipp_converted.size(), path_astar.size())
        << "The paths found by PSIPP and ASTAR should have the same length!";
  }
}

// PSIPP plans the same path as A* when no dynamic constraints are present
TEST(PSIPPTest, SamePathAsAStar2)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", 1);

  // create SIPP
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    std::mt19937 rnd_generator(0);
    PSIPP_settings psipp_settings(algo, INFO_type::no_info, 1.0);
    ConstraintManager constraint_manager(*instance);
    PSIPP psipp(*instance, rnd_generator, psipp_settings, constraint_manager);

    // from (0,0) to (4,4)

    // plan path by ASTAR
    Path path_astar = find_path_distance_gradient(0, *instance);

    // assert that astar found a path
    ASSERT_FALSE(path_astar.empty()) << "A* should find a path when no constraints are present!";

    // plan path by SIPP
    auto [path_psipp, pushed_agents] = psipp.plan(0, {});

    // assert that SIPP found a path
    ASSERT_FALSE(path_psipp.empty()) << "SIPP should find a path when no constraints are present!";
    ASSERT_TRUE(pushed_agents.empty()) << "Pushed agents when shouldnt.";

    // convert SIPP path to the same format as A*
    Path path_psipp_converted = timepointpath_to_path(path_psipp);

    // check that the paths have the same length
    EXPECT_EQ(path_psipp_converted.size(), path_astar.size())
        << "The paths found by SIPP and ASTAR should have the same length!";
  }
}

// Test Prioritized Planning
// Prioritized Planning with dummy map and 1 agent
TEST(PSIPPTest, PrioritizedPlanning1agent)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);

  // create LNS

  // plan path by Prioritized Planning
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    auto rnd_generator = std::mt19937(0);
    LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 10},
                              {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0},
                              {algo, INFO_type::no_info, 1.0}, false, true);
    // std::cout << "PSIPP setting: " << magic_enum::enum_name(lns_settings.psipp_settings.implementation) << std::endl;
    LNS lns(*instance, rnd_generator, nullptr, lns_settings);
    Solution sol = lns.PrioritizedPlanning();
    // check solution validity
    EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
  }
}

// Prioritized Planning with empty map and 2 agents
TEST(PSIPPTest, PrioritizedPlanningEmpty2agents)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 2);

  // plan path by Prioritized Planning
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    // create LNS
    auto rnd_generator = std::mt19937(0);
    LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 10},
                              {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0},
                              {algo, INFO_type::no_info, 1.0}, false, true);
    LNS lns(*instance, rnd_generator, nullptr, lns_settings);
    Solution sol = lns.PrioritizedPlanning();

    // check solution validity
    EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
  }
}

// Test Prioritized Planning
TEST(PSIPPTest, PrioritizedPlanning)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", 100);

  // plan path by Prioritized Planning
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    // create LNS
    auto rnd_generator = std::mt19937(0);
    LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 10},
                              {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0},
                              {algo, INFO_type::no_info, 1.0}, false, true);
    LNS lns(*instance, rnd_generator, nullptr, lns_settings);
    Solution sol = lns.PrioritizedPlanning();
    // check solution validity
    EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
  }
}

TEST(PSIPPTest, SameAfterDestroy)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", 200);

  // plan path by Prioritized Planning
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    // create LNS
    auto rnd_generator = std::mt19937(0);
    LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 10},
                              {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0},
                              {algo, INFO_type::no_info, 1.0}, false, true);
    LNS lns(*instance, rnd_generator, nullptr, lns_settings);
    Solution sol = lns.PrioritizedPlanning();
    // check solution validity
    EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
    int agent_num = 100;
    TimePointPath path0 = sol.paths[agent_num];
    // remove the path
    lns.get_constraint_manager().remove_trajectory(path0, agent_num);
    lns.already_planned.erase(agent_num);

    // replan
    TimePointPath path1 = lns.planner->plan(agent_num, lns.already_planned);

    // expect the same path
    EXPECT_EQ(path0.back().interval.t_min, path1.back().interval.t_min) << "Paths should have the same length!";
  }
}

// Test Prioritized Planning
TEST(PSIPPTest, MustPush)
{
  // load instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(base_path + "/tests/test_maps/must_push.map",
                                                                  base_path + "/tests/test_scen/must_push.scen", 2);

  // plan path by Prioritized Planning
  for (auto algo : magic_enum::enum_values<PSIPP_implementation>())
  {
    // create LNS
    // std::cout << "Algo:" << magic_enum::enum_name(algo) << std::endl;
    auto rnd_generator = std::mt19937(0);
    LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 10},
                              {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0},
                              {algo, INFO_type::no_info, 1.0}, false, true);
    LNS lns(*instance, rnd_generator, nullptr, lns_settings);
    // std::cout << "Running PP." << std::endl;
    Solution sol = lns.PrioritizedPlanning();
    // std::cout << "Finished." << std::endl;
    // check solution validity
    EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
  }
}

// every non-empty path must start at the start and end at the goal of the agent it is stored under
auto paths_owned_correctly(const Solution& sol, const Instance& instance) -> bool
{
  for (size_t a = 0; a < sol.paths.size(); a++)
  {
    const TimePointPath& path = sol.paths[a];
    if (path.empty())
    {
      continue;
    }
    if (path.front().location != instance.get_start_locations()[a] ||
        path.back().location != instance.get_goal_locations()[a])
    {
      return false;
    }
  }
  return true;
}

// Pushed agents must be replanned as themselves, not as the newly added agent
TEST(PSIPPTest, PushedAgentKeepsOwnPath)
{
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/random-32-32-20.map",
                                 base_path + "/tests/test_scen/random-32-32-20-random-5.scen", 225);

  std::mt19937 rng(5);
  LNS_settings lns_settings(0, 30, {DESTROY_TYPE::RANDOM, 10}, {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0},
                            {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, 5}, true, true);
  LNS lns(*instance, rng, nullptr, lns_settings);

  for (int i = 0; i < 500; i++)
  {
    Solution sol = lns.PrioritizedPlanning();
    ASSERT_TRUE(paths_owned_correctly(sol, *instance)) << "attempt " << i;
    if (sol.feasible)
    {
      EXPECT_TRUE(sol.is_valid(*instance)) << "Solution is invalid!";
      break;
    }
    lns.get_constraint_manager().reset();
    lns.already_planned.clear();
  }
}

// PP must produce valid, correctly owned paths with and without the SIPP-first attempt for the new agent
TEST(PSIPPTest, SippFirstSwitch)
{
  std::string base_path = get_base_path_tests();  // path to my_solver
  for (bool sipp_first : {true, false})
  {
    std::unique_ptr<Instance> den = std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map",
                                                               base_path + "/tests/test_scen/den520d-random-0.scen", 100);
    std::unique_ptr<Instance> push = std::make_unique<Instance>(base_path + "/tests/test_maps/must_push.map",
                                                                base_path + "/tests/test_scen/must_push.scen", 2);
    for (Instance* instance : {den.get(), push.get()})
    {
      std::mt19937 rng(0);
      LNS_settings lns_settings(0, 30, {DESTROY_TYPE::RANDOM, 10},
                                {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0},
                                {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0}, false, true, sipp_first);
      LNS lns(*instance, rng, nullptr, lns_settings);
      Solution sol = lns.PrioritizedPlanning();
      EXPECT_TRUE(sol.is_valid(*instance)) << "sipp_first=" << sipp_first;
      EXPECT_TRUE(paths_owned_correctly(sol, *instance)) << "sipp_first=" << sipp_first;
    }
  }
}

TEST(PSIPPComparator, InfinitePushCostIsLexicographic)
{
  psipp::PushArena arena;
  psipp::PushList no_push;
  psipp::PushList one_push = arena.push(no_push, Constraint(TimeInterval(0, 1), 0, 0, 0));
  psipp::PSIPPNode a(0, TimeInterval(0, 1), 100, 0, 0, 0, nullptr, no_push);
  psipp::PSIPPNode b(0, TimeInterval(0, 1), 10, 0, 0, 0, nullptr, one_push);
  psipp::PSIPPNodeComparator cmp;
  EXPECT_FALSE(cmp(&a, &b));
  EXPECT_TRUE(cmp(&b, &a));
}

TEST(PSIPPComparator, FinitePushCost)
{
  psipp::PushArena arena;
  psipp::PushList no_push;
  psipp::PushList one_push = arena.push(no_push, Constraint(TimeInterval(0, 1), 0, 0, 0));
  psipp::PSIPPNode a(0, TimeInterval(0, 1), 100, 0, 0, 0, nullptr, no_push);
  psipp::PSIPPNode b(0, TimeInterval(0, 1), 10, 0, 0, 0, nullptr, one_push);
  psipp::PSIPPNodeComparator cmp(5.0);
  EXPECT_TRUE(cmp(&a, &b));
  EXPECT_FALSE(cmp(&b, &a));
  psipp::PSIPPNodeComparator cmp_high(200.0);
  EXPECT_FALSE(cmp_high(&a, &b));
}

TEST(PSIPPTest, PushCostPPValid)
{
  std::string base_path = get_base_path_tests();
  for (double push_cost : {0.0, 2.0, 10.0, 50.0})
  {
    std::unique_ptr<Instance> den = std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map",
                                                               base_path + "/tests/test_scen/den520d-random-0.scen", 100);
    std::unique_ptr<Instance> push = std::make_unique<Instance>(base_path + "/tests/test_maps/must_push.map",
                                                                base_path + "/tests/test_scen/must_push.scen", 2);
    for (Instance* instance : {den.get(), push.get()})
    {
      std::mt19937 rng(0);
      LNS_settings lns_settings(0, 30, {DESTROY_TYPE::RANDOM, 10},
                                {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0},
                                {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, 5, push_cost}, false,
                                true);
      LNS lns(*instance, rng, nullptr, lns_settings);
      Solution sol = lns.PrioritizedPlanning();
      // with cheap pushes two agents can push each other forever; PP reports this as an infeasible solution
      if (instance == push.get() && !sol.feasible) continue;
      EXPECT_TRUE(sol.is_valid(*instance)) << "push_cost=" << push_cost;
    }
  }
}

TEST(PSIPPTest, PerPushCountClosedListValid)
{
  std::string base_path = get_base_path_tests();
  for (bool sipp_first : {true, false})
  {
    for (double push_cost : {std::numeric_limits<double>::infinity(), 2.0})
    {
      std::unique_ptr<Instance> den = std::make_unique<Instance>(
          base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", 100);
      std::unique_ptr<Instance> push = std::make_unique<Instance>(base_path + "/tests/test_maps/must_push.map",
                                                                  base_path + "/tests/test_scen/must_push.scen", 2);
      for (Instance* instance : {den.get(), push.get()})
      {
        std::mt19937 rng(0);
        LNS_settings lns_settings(
            0, 30, {DESTROY_TYPE::RANDOM, 10}, {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0},
            {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, 5, push_cost, true}, false, true,
            sipp_first);
        LNS lns(*instance, rng, nullptr, lns_settings);
        Solution sol = lns.PrioritizedPlanning();
        if (std::isinf(push_cost) || (instance == den.get() && sipp_first))
        {
          EXPECT_TRUE(sol.feasible) << "sipp_first=" << sipp_first << " push_cost=" << push_cost;
        }
        // with cheap pushes two agents can push each other forever; PP reports this as an infeasible solution
        else if (!sol.feasible)
        {
          continue;
        }
        EXPECT_TRUE(sol.is_valid(*instance)) << "sipp_first=" << sipp_first << " push_cost=" << push_cost;
        EXPECT_TRUE(paths_owned_correctly(sol, *instance)) << "sipp_first=" << sipp_first << " push_cost=" << push_cost;
      }
    }
  }
  // finite push costs where PSIPP plans every agent (sipp_first = false) and PP is known to be feasible (fixed seed)
  for (auto [num_agents, push_cost] : std::vector<std::pair<int, double>>{{100, 10.0}, {100, 50.0}, {25, 2.0}})
  {
    std::unique_ptr<Instance> instance = std::make_unique<Instance>(
        base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", num_agents);
    std::mt19937 rng(0);
    LNS_settings lns_settings(
        0, 30, {DESTROY_TYPE::RANDOM, 10}, {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0},
        {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, 5, push_cost, true}, false, true, false);
    LNS lns(*instance, rng, nullptr, lns_settings);
    Solution sol = lns.PrioritizedPlanning();
    ASSERT_TRUE(sol.feasible) << "agents=" << num_agents << " push_cost=" << push_cost;
    EXPECT_TRUE(sol.is_valid(*instance)) << "agents=" << num_agents << " push_cost=" << push_cost;
    EXPECT_TRUE(paths_owned_correctly(sol, *instance)) << "agents=" << num_agents << " push_cost=" << push_cost;
  }
}

TEST(PSIPPTest, DiagnosticsCounters)
{
  std::string base_path = get_base_path_tests();
  {
    std::unique_ptr<Instance> instance = std::make_unique<Instance>(base_path + "/tests/test_maps/must_push.map",
                                                                    base_path + "/tests/test_scen/must_push.scen", 2);
    std::mt19937 rng(0);
    LNS_settings lns_settings(0, 30, {DESTROY_TYPE::RANDOM, 10},
                              {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0},
                              {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0}, false, true);
    LNS lns(*instance, rng, nullptr, lns_settings);
    lns.PrioritizedPlanning();
    EXPECT_GE(lns.log.pp_pushed_replanned, 1);
    EXPECT_EQ(lns.log.pp_fail_no_path + lns.log.pp_fail_cycle, 0);
  }
  {
    std::unique_ptr<Instance> instance = std::make_unique<Instance>(
        base_path + "/tests/test_maps/biparted_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);
    std::mt19937 rng(0);
    LNS_settings lns_settings(0, 30, {DESTROY_TYPE::RANDOM, 10},
                              {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0},
                              {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0}, false, true, false);
    LNS lns(*instance, rng, nullptr, lns_settings);
    Solution sol = lns.PrioritizedPlanning();
    EXPECT_FALSE(sol.feasible);
    EXPECT_EQ(lns.log.pp_fail_no_path, 1);
    EXPECT_EQ(lns.log.pp_fail_cycle, 0);
  }
}

TEST(PSIPPTest, CycleCounterWithFreePushes)
{
  std::string base_path = get_base_path_tests();
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(base_path + "/tests/test_maps/must_push.map",
                                                                  base_path + "/tests/test_scen/must_push.scen", 2);
  std::mt19937 rng(0);
  // with free pushes the two agents push each other forever, which PP detects as a cycle
  LNS_settings lns_settings(0, 30, {DESTROY_TYPE::RANDOM, 10},
                            {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0},
                            {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, 5, 0.0, false}, false,
                            true, true);
  LNS lns(*instance, rng, nullptr, lns_settings);
  lns.PrioritizedPlanning();
  EXPECT_GT(lns.log.pp_fail_cycle, 0);
}

// Regression: building a node from a pushed list must not change the caller's list. The successor generation reuses
// the same list for several successors; an emptied list made PSIPP return paths through agents it never pushed.
TEST(PSIPPTest, NodeConstructorKeepsPushedList)
{
  psipp::PushArena arena;
  psipp::PushList pushed = arena.push(psipp::PushList(), Constraint(TimeInterval(3, 5), 7, 1, 2));
  psipp::PSIPPNode first(4, TimeInterval(0, 10), 0, 0, 0, 0, nullptr, pushed);
  psipp::PSIPPNode second(TimePoint(4, TimeInterval(0, 10)), 0, 0, 0, 0, nullptr, pushed);
  ASSERT_EQ(pushed.size(), 1u);
  EXPECT_EQ(pushed.begin()->agent_num, 7);
  EXPECT_EQ(first.pushed.size(), 1u);
  EXPECT_EQ(second.pushed.size(), 1u);
}

// Extending a shared list must leave the shared prefix intact, also across chunk boundaries of the arena.
TEST(PSIPPTest, PushListSharesPrefix)
{
  psipp::PushArena arena;
  psipp::PushList base = arena.push(psipp::PushList(), Constraint(TimeInterval(0, 1), 1, 0, 0));
  std::vector<psipp::PushList> extended;
  for (int i = 0; i < 10000; ++i)
  {
    extended.push_back(arena.push(base, Constraint(TimeInterval(0, 1), 100 + i, 0, 0)));
  }
  ASSERT_EQ(base.size(), 1u);
  for (int i = 0; i < 10000; ++i)
  {
    ASSERT_EQ(extended[i].size(), 2u);
    std::vector<int> agents;
    for (const auto& c : extended[i]) agents.push_back(c.agent_num);
    ASSERT_EQ(agents, (std::vector<int>{100 + i, 1}));
  }
  arena.reset();
  psipp::PushList again = arena.push(psipp::PushList(), Constraint(TimeInterval(0, 1), 5, 0, 0));
  EXPECT_EQ(again.size(), 1u);
  EXPECT_EQ(again.begin()->agent_num, 5);
}

// whether the path collides (vertex or swap) with a path of an agent that is not in the ignored set
auto collides_with_others(const TimePointPath& path, const std::unordered_map<int, TimePointPath>& others,
                          const std::unordered_set<int>& ignored) -> bool
{
  for (const auto& [agent, other] : others)
  {
    if (ignored.find(agent) != ignored.end()) continue;
    for (int i = 0; i < static_cast<int>(path.size()); i++)
    {
      for (int j = 0; j < static_cast<int>(other.size()); j++)
      {
        if (path[i].location == other[j].location && overlap(path[i].interval, other[j].interval)) return true;
        // swap: path moves a->b while the other agent moves b->a at the same time
        if (i > 0 && j > 0 && path[i].interval.t_min == other[j].interval.t_min &&
            path[i].location == other[j - 1].location && path[i - 1].location == other[j].location)
        {
          return true;
        }
      }
    }
  }
  return false;
}

// Regression: a pushed agent is assumed to wait in its previous cell during the pushed interval. PSIPP generated the
// successor entering that cell right after the wait even when the current safe interval ended earlier, so the
// returned path silently stretched the wait over a cell occupied by an agent that was not pushed. The state is the
// constraint table of a failing PP run (room-64-64-8, 400 agents, scenario 8, seed 8, shortest-first order).
TEST(PSIPPTest, PushedWaitDoesNotStretchCurrentInterval)
{
  std::string base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance = std::make_unique<Instance>(
      base_path + "/tests/test_maps/room-64-64-8.map", base_path + "/tests/test_scen/room-64-64-8-random-8.scen", 400);
  std::ifstream state(base_path + "/tests/test_solutions/psipp_room_agent299_state.txt");
  ASSERT_TRUE(state.is_open());
  std::unordered_map<int, TimePointPath> planned_paths;
  std::unordered_set<int> already_planned;
  std::string line;
  while (std::getline(state, line))
  {
    std::istringstream iss(line);
    int agent;
    iss >> agent;
    TimePointPath path;
    int loc, t_min, t_max;
    while (iss >> loc >> t_min >> t_max) path.emplace_back(loc, TimeInterval(t_min, t_max));
    planned_paths[agent] = path;
    already_planned.insert(agent);
  }
  ASSERT_EQ(already_planned.size(), 394u);

  ConstraintManager constraint_manager(*instance);
  for (const auto& [agent, path] : planned_paths) constraint_manager.add_trajectory(path, agent);
  for (int seed = 0; seed < 10; seed++)
  {
    std::mt19937 rng(seed);
    PSIPP psipp(*instance, rng, {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, 5}, constraint_manager);
    auto [path, pushed] = psipp.plan(299, already_planned);
    ASSERT_FALSE(path.empty()) << "seed " << seed;
    EXPECT_TRUE(instance->check_timepointpath_validity(path)) << "seed " << seed;
    EXPECT_FALSE(collides_with_others(path, planned_paths, pushed)) << "seed " << seed;
  }
}
