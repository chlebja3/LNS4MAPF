/*
 * Author: Jan Chleboun
 * Date: 19-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gtest/gtest.h>
#include <math.h>
#include <stdint.h>

#include <iostream>

#include "Instance.h"
#include "test_utils.h"

// Test 1: Test initialization when instance loaded
TEST(InstanceTest, Initialized)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);
  // check whether the instance is initialized
  EXPECT_TRUE(instance->is_initialized());
}

// Test 2: Check initialization when empty instance is created
TEST(InstanceTest, NotInitialized)
{
  // create empty instance
  std::unique_ptr<Instance> instance = std::make_unique<Instance>();

  // check whether the instance is initialized
  EXPECT_FALSE(instance->is_initialized());
}

// Test 3: Test manual initialization
TEST(InstanceTest, ManualInitialization)
{
  // create empty instance
  std::unique_ptr<Instance> instance = std::make_unique<Instance>();

  // initialize instance
  std::string base_path = get_base_path_tests();  // path to my_solver
  instance->initialize(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);

  // check whether the instance is initialized
  EXPECT_TRUE(instance->is_initialized());
}

// Test 4: Test map loading
TEST(InstanceTest, LoadMap)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 1);

  // check whether the map looks as expected
  std::vector<uint8_t> expected_map = {0, 0, 0, 0, 1, 0, 0, 0, 0};
  const Map&           loaded_map   = instance->get_map_data();

  // check that the loaded map is right
  EXPECT_EQ(loaded_map.width, 3) << "Width of the map is not correct";
  EXPECT_EQ(loaded_map.height, 3) << "Height of the map is not correct";
  EXPECT_EQ(loaded_map.free_cells, 8) << "Number of free cells is not correct";

  EXPECT_EQ(loaded_map.data, expected_map) << "Map is not correct";
}

// Test 5: Test scene loading
TEST(InstanceTest, LoadScene)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 3);

  // check that the number of agents is right
  EXPECT_EQ(instance->num_of_agents, 3);

  // check that the start positions are right
  const std::vector<Point2d> expected_starts = {{0, 0}, {2, 2}, {1, 2}};
  EXPECT_EQ(instance->get_start_positions(), expected_starts);

  // check that the goal positions are right
  const std::vector<Point2d> expected_goals = {{2, 2}, {0, 0}, {2, 1}};
  EXPECT_EQ(instance->get_goal_positions(), expected_goals);
}

// Test 6: Check manhattan distance heuristics
TEST(InstanceTest, ManhattanDistance)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 3);

  // expected heuristics for each agent
  std::vector<std::vector<int>> expected_heuristics = {
      {4, 3, 2, 3, 2, 1, 2, 1, 0}, {0, 1, 2, 1, 2, 3, 2, 3, 4}, {3, 2, 1, 2, 1, 0, 3, 2, 1}};
  // check with the calculated
  for (int i = 0; i < instance->num_of_agents; i++)
  {
    for (int y = 0; y < instance->get_map_data().height; y++)
    {
      for (int x = 0; x < instance->get_map_data().width; x++)
      {
        EXPECT_EQ(instance->get_heuristic_manhattan(i, Point2d(x, y)), expected_heuristics[i][y * instance->get_map_data().width + x])
            << "Heuristic is not correct";
      }
    }
  }
}

// Test 7: Check euclidean distance heuristics
TEST(InstanceTest, EuclideanDistance)
{
  // load instance
  std::string               base_path = get_base_path_tests();  // path to my_solver
  std::unique_ptr<Instance> instance =
      std::make_unique<Instance>(base_path + "/tests/test_maps/dummy_3_3.map", base_path + "/tests/test_scen/dummy_3_3_scen_1.scen", 3);

  // expected heuristics for each agent
  std::vector<std::vector<double>> expected_heuristics = {
      {std::sqrt(8.0), std::sqrt(5.0), 2.0, std::sqrt(5.0), std::sqrt(2.0), 1.0, 2.0, 1.0, 0.0},
      {0, 1, 2, 1, std::sqrt(2.0), std::sqrt(5.0), 2.0, std::sqrt(5.0), std::sqrt(8.0)},
      {std::sqrt(5.0), std::sqrt(2.0), 1.0, 2.0, 1.0, 0.0, std::sqrt(5.0), std::sqrt(2.0), 1.0}};
  // check with the calculated
  for (int i = 0; i < instance->num_of_agents; i++)
  {
    for (int y = 0; y < instance->get_map_data().height; y++)
    {
      for (int x = 0; x < instance->get_map_data().width; x++)
      {
        EXPECT_EQ(instance->get_heuristic_euclidean(i, Point2d(x, y)), expected_heuristics[i][y * instance->get_map_data().width + x])
            << "Heuristic is not correct";
      }
    }
  }
}
