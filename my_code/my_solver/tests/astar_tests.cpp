/*
 * Author: Jan Chleboun
 * Date: 04-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gtest/gtest.h>

#include "Astar.h"
#include "utils.h"

// Helper function to create a simple test map
namespace ASTAR_TEST
{
Map create_test_map()
{
  Map my_map;
  my_map.width  = 5;
  my_map.height = 5;
  my_map.data = {
      0, 0, 0, 0, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 1, 0,
      0, 1, 0, 0, 0,
      0, 0, 0, 0, 0
  };
  return my_map;
}
}

// Test Case: Basic Shortest Path (No Obstacles)
TEST(AStarTest, BasicShortestPath)
{
  Map     my_map = ASTAR_TEST::create_test_map();
  Point2d start(0, 0);
  Point2d goal(4, 4);

  Path path = ASTAR::find_path(start, goal, my_map);

  std::vector<Point2d> expected_path = {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}, {3, 4}, {4, 4}};
  // the paths dont have to be the same, but should have the same length and be collision free

  EXPECT_EQ(path.size(), expected_path.size()); // same length
  for (auto &it: path)
  {
    EXPECT_EQ(my_map.index(it), 0) << "The path returned by the algorithm is not obstacle free.";
  }
}

// Test Case: Start == Goal
TEST(AStarTest, StartEqualsGoal)
{
  Map     my_map = ASTAR_TEST::create_test_map();
  Point2d start(2, 2);
  Point2d goal(2, 2);

  Path path = ASTAR::find_path(start, goal, my_map);

  EXPECT_EQ(path.size(), 1);  // Should return only the start node
  EXPECT_EQ(path[0], start);
}

// Test Case: No Path Possible (Blocked Goal)
TEST(AStarTest, NoPathPossible)
{
  Map     my_map = ASTAR_TEST::create_test_map();
  Point2d start(0, 0);
  Point2d goal(3, 2);  // This is inside an obstacle!

  Path path = ASTAR::find_path(start, goal, my_map);

  EXPECT_TRUE(path.empty());  // No path should be returned
}

// Test Case: Complex Path with Obstacles
TEST(AStarTest, ComplexPath)
{
  Map     my_map = ASTAR_TEST::create_test_map();
  Point2d start(0, 0);
  Point2d goal(2, 4);

  Path path = ASTAR::find_path(start, goal, my_map);

  std::vector<Point2d> expected_path = {{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 3}, {2, 4}};

  EXPECT_EQ(path.size(), expected_path.size()); // same length
  for (auto &it: path)
  {
    EXPECT_EQ(my_map.index(it), 0) << "The path returned by the algorithm is not obstacle free.";
  }
}

// Test Case: Edge Case (Start Outside Bounds)
TEST(AStarTest, StartOutsideBounds)
{
  Map     my_map = ASTAR_TEST::create_test_map();
  Point2d start(-1, -1);
  Point2d goal(4, 4);

  Path path = ASTAR::find_path(start, goal, my_map);

  EXPECT_TRUE(path.empty());  // Should return no path
}

// Test Case: Goal Outside Bounds
TEST(AStarTest, GoalOutsideBounds)
{
  Map     my_map = ASTAR_TEST::create_test_map();
  Point2d start(0, 0);
  Point2d goal(10, 10);

  Path path = ASTAR::find_path(start, goal, my_map);

  EXPECT_TRUE(path.empty());  // Should return no path
}

// Test Case: Large Open Grid (Performance Test)
TEST(AStarTest, LargeGrid)
{
  Map my_map;
  my_map.width  = 100;
  my_map.height = 100;
  my_map.data.resize(100 * 100, 0);  // No obstacles

  Point2d start(0, 0);
  Point2d goal(99, 99);

  Path path = ASTAR::find_path(start, goal, my_map);

  EXPECT_EQ(path.size(), 1+99+99);  // Should find a path of length 199
  EXPECT_EQ(path.front(), start);
  EXPECT_EQ(path.back(), goal);
}
