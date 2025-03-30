/*
 * Author: Jan Chleboun
 * Date: 12-03-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */


#include <gtest/gtest.h>

#include <climits>

#include "ConstraintTable.h"
#include "Instance.h"
#include "LNS.h"
#include "test_utils.h"

TEST(LNSCollisionVectors, SinglePath)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/empty_5_5.map", base_path + "/tests/test_scen/empty_5_5_scen_1.scen", 1);

  // find initial solution
  auto         rnd_generator = std::mt19937(0);
  LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 10}, {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0});
  LNS          lns(*instance, rnd_generator, nullptr, lns_settings);
  Solution     sol;
  sol.paths = {TimePointPath({{0, {0, 0}}, {1, {1, 1}}, {2, {2, 5}}, {3, {6, 7}}, {2, {8, 8}}, {1, {9, 10}}})};

  auto [location_time_to_agentnum, edge_collisions] = lns.create_collision_vectors(sol, 11);

  // check position collisions
  for (int i = 0; i < instance->get_map_data().width * instance->get_map_data().height; i++)
  {
    assertm(i < static_cast<int>(location_time_to_agentnum.size()), "Index out of bounds");
    for (int j = 0; j < 10; j++)
    {
      assertm(j < static_cast<int>(location_time_to_agentnum[i].size()), "Index out of bounds");
      if (i == 0 && j == 0)
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 0);
      }
      else if (i == 1 && ((j == 1) || (j >= 9 && j <= 10)))
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 0);
      }
      else if (i == 2 && ((j >= 2 && j <= 5) || j == 8))
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 0);
      }
      else if (i == 3 && j >= 6 && j <= 7)
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 0);
      }
      else
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], -1);
      }
    }
  }

  // check edge collisions
  for (int i = 0; i < static_cast<int>(instance->get_map_data().width * instance->get_map_data().height); i++)
  {
    assertm(i < static_cast<int>(edge_collisions[0].size()), "Index out of bounds");
    for (int j = 0; j < 10; j++)
    {
      assertm(j < static_cast<int>(edge_collisions[0][i].size()), "Index out of bounds");
      for (Direction dir : magic_enum::enum_values<Direction>())
      {
        if (dir == Direction::NONE)
        {
          continue;
        }
        assertm(static_cast<int>(dir) - 1 < static_cast<int>(edge_collisions.size()), "Index out of bounds");

        if (dir == Direction::RIGHT && i == 1 && j == 1)
        {
          EXPECT_EQ(edge_collisions[static_cast<int>(dir) - 1][i][j], 0);
        }
        else if (dir == Direction::RIGHT && i == 2 && j == 2)
        {
          EXPECT_EQ(edge_collisions[static_cast<int>(dir) - 1][i][j], 0);
        }
        else if (dir == Direction::RIGHT && i == 3 && j == 6)
        {
          EXPECT_EQ(edge_collisions[static_cast<int>(dir) - 1][i][j], 0);
        }
        else if (dir == Direction::LEFT && i == 2 && j == 8)
        {
          EXPECT_EQ(edge_collisions[static_cast<int>(dir) - 1][i][j], 0);
        }
        else if (dir == Direction::LEFT && i == 1 && j == 9)
        {
          EXPECT_EQ(edge_collisions[static_cast<int>(dir) - 1][i][j], 0);
        }
        else
        {
          EXPECT_EQ(edge_collisions[static_cast<int>(dir) - 1][i][j], -1);
        }
      }
    }
  }
}

TEST(LNSCollisionVectors, TwoPaths)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 2);
  // find initial solution
  auto         rnd_generator = std::mt19937(0);
  LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 10}, {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0});
  LNS          lns(*instance, rnd_generator, nullptr, lns_settings);
  lns.find_initial_solution();
  ASSERT_TRUE(lns.solution.feasible) << "Initial solution is not feasible";
  ASSERT_TRUE(lns.solution.is_valid(*instance)) << "Initial solution is not valid";

  // both paths should have len 5
  EXPECT_EQ(lns.solution.paths[0].size(), 5);
  EXPECT_EQ(lns.solution.paths[1].size(), 5);

  // create collision vectors
  auto [location_time_to_agentnum, edge_collisions] = lns.create_collision_vectors(lns.solution, 6);

  // check position collisions
  for (int i = 0; i < instance->get_map_data().width * instance->get_map_data().height; i++)
  {
    for (int j = 0; j < 5; j++)
    {
      if ((i == 0 && j == 0) || (i == 1 && j == 1))
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 0);
        EXPECT_EQ(edge_collisions[static_cast<int>(Direction::RIGHT) - 1][i + 1][j + 1], 0);
      }
      else if ((i == 2 && j == 2) || (i == 5 && j == 3))
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 0);
        EXPECT_EQ(edge_collisions[static_cast<int>(Direction::DOWN) - 1][i + instance->get_map_data().width][j + 1], 0);
      }
      else if (i == 8 && j == 4)
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 0);
      }
      else if ((i == 8 && j == 0) || (i == 7 && j == 1))
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 1);
        EXPECT_EQ(edge_collisions[static_cast<int>(Direction::LEFT) - 1][i - 1][j + 1], 1);
      }
      else if ((i == 6 && j == 2) || (i == 3 && j == 3))
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 1);
        EXPECT_EQ(edge_collisions[static_cast<int>(Direction::UP) - 1][i - instance->get_map_data().width][j + 1], 1);
      }
      else if (i == 0 && j == 4)
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], 1);
      }
      else
      {
        EXPECT_EQ(location_time_to_agentnum[i][j], -1) << "Location-time pair (" << i << ", " << j << ") should be safe";
      }
    }
  }
}

// test many paths
TEST(LNSCollisionVectors, ConsistencyWithConstraintTable)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", 200);

  // find initial solution
  auto         rnd_generator = std::mt19937(0);
  LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOM, 10}, {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0});
  LNS          lns(*instance, rnd_generator, nullptr, lns_settings);
  lns.find_initial_solution();
  ASSERT_TRUE(lns.solution.feasible) << "Initial solution is not feasible";
  ASSERT_TRUE(lns.solution.is_valid(*instance)) << "Initial solution is not valid";

  // find maximum path len
  int max_path_len = 0;
  for (const auto& path : lns.solution.paths)
  {
    max_path_len = std::max(max_path_len, static_cast<int>(path.size()));
  }

  // create collision vectors
  auto [location_time_to_agentnum, edge_collisions] = lns.create_collision_vectors(lns.solution, max_path_len);

  // check consistency with constraint table
  for (int i = 0; i < instance->get_map_data().width * instance->get_map_data().height; i++)  // locations
  {
    for (int j = 0; j < max_path_len; j++)  // times
    {
      if (location_time_to_agentnum[i][j] != -1)
      {
        auto [beggining, end] = lns.planner->constraint_table.get_safe_intervals(i, TimeInterval(j, j));
        EXPECT_TRUE(beggining == end) << "Location-time pair (" << i << ", " << j << ") should not be safe";
      }
    }
  }
}

// test destroy operators
TEST(LNSDestroyOperators, RightNumberOfDestroyed)
{
  // load instance
  int                       agent_num = 50;
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance  = std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map",
                                                                  base_path + "/tests/test_scen/den520d-random-0.scen", agent_num);

  std::array<int, 5> neighborhood_sizes = {1, 4, 8, 16, 32};

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
      EXPECT_EQ(lns.solution.destroyed_paths.size(), neighborhood_size);
    }
  }
}

TEST(LNSDestroyRandomWalk, MostDelayedPresent)
{
  // load instance
  int                       agent_num = 500;
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance  = std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map",
                                                                  base_path + "/tests/test_scen/den520d-random-0.scen", agent_num);

  std::array<int, 5> neighborhood_sizes = {1, 4, 8, 16, 32};

  auto rnd_generator = std::mt19937(0);
  for (auto neighborhood_size : neighborhood_sizes)
  {
    LNS_settings lns_settings(0, 5, {DESTROY_TYPE::RANDOMWALK, neighborhood_size},
                              {SIPP_implementation::SIPP_mine, INFO_type::no_info, 1.0});
    LNS          lns(*instance, rnd_generator, nullptr, lns_settings);
    lns.find_initial_solution();
    ASSERT_TRUE(lns.solution.feasible) << "Initial solution is not feasible";
    ASSERT_TRUE(lns.solution.is_valid(*instance)) << "Initial solution is not valid";

    // destroy agents
    lns.destroy_operator.apply(lns.solution);

    // find the most delayed agent
    int max_delay    = 0;
    int most_delayed = 0;
    for (int i = 0; i < agent_num; i++)
    {
      max_delay = std::max(max_delay, lns.solution.delays[i]);
      if (lns.solution.delays[i] == max_delay)
      {
        most_delayed = i;
      }
    }

    // check that the most delayed agent is present in the destroyed paths
    bool found = false;
    for (const auto& destroyed : lns.solution.destroyed_paths)
    {
      if (destroyed == most_delayed)
      {
        found = true;
        break;
      }
    }
    EXPECT_TRUE(found) << "Most delayed agent should be present in the destroyed paths";
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
