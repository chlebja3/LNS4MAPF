/*
 * Author: Jan Chleboun
 * Date: 04-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gtest/gtest.h>

#include "Astar.h"
#include "test_utils.h"
#include "utils.h"

// Test Case: Basic Shortest Path (No Obstacles)
TEST(AStarTest, BasicShortestPath)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/corner_5_5.map", base_path + "/tests/test_scen/corner_5_5_scen_1.scen", 1);

  Path path = ASTAR::find_path(0, *instance);

  std::vector<Point2d> expected_path = {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {3, 4}, {4, 4}};
  // the paths dont have to be the same, but should have the same length and be collision free

  EXPECT_EQ(path.size(), expected_path.size());  // same length
  for (auto &it : path)
  {
    EXPECT_EQ(instance->get_map_data().index(it), 0) << "The path returned by the algorithm is not obstacle free.";
  }
}

// Test Case: Start == Goal
TEST(AStarTest, StartEqualsGoal)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/corner_5_5.map", base_path + "/tests/test_scen/corner_5_5_scen_1.scen", 2);

  Path path = ASTAR::find_path(1, *instance);

  EXPECT_EQ(path.size(), 1);  // Should return only the start node
  EXPECT_EQ(path[0], instance->get_start_positions()[1]);
}

// Test Case: No Path Possible (Blocked Goal)
TEST(AStarTest, NoPathPossible)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/corner_5_5.map", base_path + "/tests/test_scen/corner_5_5_scen_1.scen", 3);

  Path path = ASTAR::find_path(2, *instance);

  EXPECT_TRUE(path.empty());  // No path should be returned
}

// Test Case: Complex Path with Obstacles
TEST(AStarTest, ComplexPath)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/corner_5_5.map", base_path + "/tests/test_scen/corner_5_5_scen_1.scen", 4);

  Path path = ASTAR::find_path(3, *instance);

  std::vector<Point2d> expected_path = {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}};

  EXPECT_EQ(path.size(), expected_path.size());  // same length
  for (auto &it : path)
  {
    EXPECT_EQ(instance->get_map_data().index(it), 0) << "The path returned by the algorithm is not obstacle free.";
  }
}

// Test Case: Large Open Grid (Performance Test)
TEST(AStarTest, LargeGrid)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance  = std::make_unique<Instance>(base_path + "/tests/test_maps/empty_100_100.map",
                                                                  base_path + "/tests/test_scen/empty_100_100_scen_1.scen", 1);

  Path path = ASTAR::find_path(0, *instance);

  EXPECT_EQ(path.size(), 1 + 99 + 99);  // Should find a path of length 199
  EXPECT_EQ(path.front(), instance->get_start_positions()[0]);
  EXPECT_EQ(path.back(), instance->get_goal_positions()[0]);
}
