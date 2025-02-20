/*
 * Author: Jan Chleboun
 * Date: 03-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Include the header where Point2d is declared
#include "test_utils.h"
#include "utils.h"

// a custom assertion to check whether the value is in expected range, inclusive
::testing::AssertionResult inRange(int val, int a, int b)
{
  if ((val >= a) && (val <= b))
    return ::testing::AssertionSuccess();
  else if (val < a)
    return ::testing::AssertionFailure() << val << " is less than " << a;
  else
    return ::testing::AssertionFailure() << val << " is greater than " << b;
}

// Node tests
// test, whether nodes are compared the right way
TEST(NodeComparison, GreaterThan)
{
  // ints
  EXPECT_TRUE(Node(1, 2) > Node(2, 0)) << "Wrong comparison positive.";
  EXPECT_TRUE(Node(5.3, 4.1) > Node(1.2, 7.8)) << "Wrong comparison positive.";
  EXPECT_FALSE(Node(1, 2) > Node(2, 2)) << "Wrong comparison positive.";
  EXPECT_FALSE(Node(4, 5) > Node(12, 0)) << "Wrong comparison positive.";

  // negative numbers
  EXPECT_TRUE(Node(-1, -1) > Node(-2, -2)) << "Wrong comparison negative.";
  EXPECT_TRUE(Node(-1.1, -5.6) > Node(-10.5, -1.0)) << "wrong comparison negative.";
  EXPECT_FALSE(Node(-10, -1) > Node(-1, -8)) << "wrong comparison negative.";
  EXPECT_FALSE(Node(-1, -1) > Node(-1, 0)) << "wrong comparison negative.";

  // mix
  EXPECT_TRUE(Node(1, 1) > Node(-2, -2)) << "Wrong comparison positive and negative.";
  EXPECT_TRUE(Node(1, -5) > Node(-10, 3)) << "wrong comparison positive and negative.";
  EXPECT_FALSE(Node(-10, -1) > Node(1, 8)) << "wrong comparison positive and negative.";
  EXPECT_FALSE(Node(1, -1) > Node(1, 0)) << "wrong comparison positive and negative.";

  // equal
  EXPECT_FALSE(Node(0, 0) > Node(0, 0)) << "Wrong comparison on equal.";
  EXPECT_FALSE(Node(1.2, 2.4) > Node(2.4, 1.2)) << "Wrong comparison on equal.";
  EXPECT_FALSE(Node(-3, -5) > Node(-3, -5)) << "Wrong comparison on equal.";
}

TEST(NodeComparison, LessThan)
{
  // ints
  EXPECT_FALSE(Node(1, 2) < Node(2, 0)) << "Wrong comparison positive.";
  EXPECT_FALSE(Node(5.3, 4.1) < Node(1.2, 7.8)) << "Wrong comparison positive.";
  EXPECT_TRUE(Node(1, 2) < Node(2, 2)) << "Wrong comparison positive.";
  EXPECT_TRUE(Node(4, 5) < Node(12, 0)) << "Wrong comparison positive.";

  // negative numbers
  EXPECT_FALSE(Node(-1, -1) < Node(-2, -2)) << "Wrong comparison negative.";
  EXPECT_FALSE(Node(-1.1, -5.6) < Node(-10.5, -1.0)) << "wrong comparison negative.";
  EXPECT_TRUE(Node(-10, -1) < Node(-1, -8)) << "wrong comparison negative.";
  EXPECT_TRUE(Node(-1, -1) < Node(-1, 0)) << "wrong comparison negative.";

  // mix
  EXPECT_FALSE(Node(1, 1) < Node(-2, -2)) << "Wrong comparison positive and negative.";
  EXPECT_FALSE(Node(1, -5) < Node(-10, 3)) << "wrong comparison positive and negative.";
  EXPECT_TRUE(Node(-10, -1) < Node(1, 8)) << "wrong comparison positive and negative.";
  EXPECT_TRUE(Node(1, -1) < Node(1, 0)) << "wrong comparison positive and negative.";

  // equal
  EXPECT_FALSE(Node(0, 0) < Node(0, 0)) << "Wrong comparison on equal.";
  EXPECT_FALSE(Node(1.2, 2.4) < Node(2.4, 1.2)) << "Wrong comparison on equal.";
  EXPECT_FALSE(Node(-3, -5) < Node(-3, -5)) << "Wrong comparison on equal.";
}

TEST(NodeComparison, Equal)
{
  EXPECT_TRUE(Node(0, 0) == Node(0, 0)) << "Wrong comparison on equal.";
  EXPECT_TRUE(Node(1, 2) == Node(1, 2)) << "Wrong comparison on equal.";
  EXPECT_TRUE(Node(-4, -2) == Node(-4, -2)) << "Wrong comparison on equal.";
  EXPECT_TRUE(Node(100.314, 15.0) == Node(100.314, 15.0)) << "Wrong comparison on equal.";

  EXPECT_FALSE(Node(0, 0) == Node(1, 0)) << "Wrong comparison on different.";
  EXPECT_FALSE(Node(0, 1) == Node(1, 0)) << "Wrong comparison on different.";
  EXPECT_FALSE(Node(1.1123, 13.1) == Node(111.23, 1.31)) << "Wrong comparison on different.";
  EXPECT_FALSE(Node(2, 1) == Node(1, 2)) << "Wrong comparison on different.";
}
// Point2d tests
// Basic Tests for Constructor and Public Variables
TEST(Point2dTest, Constructor)
{
  Point2d p(3, 4);
  EXPECT_EQ(p.x, 3) << "Wrong x value in constructor.";
  EXPECT_EQ(p.y, 4) << "Wrong y value in constructor.";
}

// Test Addition Operator `+`
TEST(Point2dTest, OperatorPlus)
{
  Point2d p1(1, 1);
  Point2d p2(2, 3);
  Point2d result = p1 + p2;

  EXPECT_EQ(result.x, 3) << "Wrong addition result.";
  EXPECT_EQ(result.y, 4) << "Wrong addition result.";
}

// Test Equality Operator `==`
TEST(Point2dTest, OperatorEquality)
{
  Point2d p1(2, 2);
  Point2d p2(2, 2);
  Point2d p3(3, 4);

  EXPECT_TRUE(p1 == p2) << "Points should be equal.";       // Should be equal
  EXPECT_FALSE(p1 == p3) << "Points should not be equal.";  // Should not be equal
}

// Test `add()` Method (Modifies the Object)
TEST(Point2dTest, AddMethod)
{
  Point2d p1(1, 1);
  Point2d p2(2, 3);

  p1.add(p2);

  EXPECT_EQ(p1.x, 3) << "Wrong x after addition.";
  EXPECT_EQ(p1.y, 4) << "Wrong y after addition.";
}

// Edge Case: Adding Zero
TEST(Point2dTest, AddZero)
{
  Point2d p1(5, 7);
  Point2d zero(0, 0);

  p1.add(zero);

  EXPECT_EQ(p1.x, 5) << "Wrong x after adding 0.";
  EXPECT_EQ(p1.y, 7) << "Wrong y after adding 0.";
}

// Edge Case: Adding Negative Values
TEST(Point2dTest, AddNegative)
{
  Point2d p1(5, 7);
  Point2d neg(-2, -3);

  p1.add(neg);

  EXPECT_EQ(p1.x, 3) << "Wrong x after adding a negative number.";
  EXPECT_EQ(p1.y, 4) << "Wrong y after adding a negative number.";
}

// Edge Case: Adding Large Values (Overflow Test)
TEST(Point2dTest, LargeValues)
{
  Point2d p1(1000000000, 1000000000);
  Point2d p2(1000000000, 1000000000);

  Point2d result = p1 + p2;

  EXPECT_EQ(result.x, 2000000000) << "Overflow not handled in x.";
  EXPECT_EQ(result.y, 2000000000) << "Overflow not handled in y.";
}

// test finding the neighbours

// Helper function to create a map
namespace UTILS_TEST
{
Map create_test_map()
{
  Map my_map;
  my_map.width  = 3;
  my_map.height = 3;
  my_map.data   = {0, 1, 0, 0, 0, 1, 1, 0, 1};
  return my_map;
}
}  // namespace UTILS_TEST

// Test Case: Center Position (Now Order-Independent)
TEST(FindNeighborsTest, CenterPosition)
{
  Map     my_map = UTILS_TEST::create_test_map();
  Point2d pos(1, 1);

  std::vector<Point2d> neighbors = my_map.find_neighbors(pos);

  EXPECT_THAT(neighbors, ::testing::UnorderedElementsAre(Point2d(1, 2), Point2d(0, 1)));
}

// Test Case: Corner Position (Top-Left)
TEST(FindNeighborsTest, TopLeftCorner)
{
  Map     my_map = UTILS_TEST::create_test_map();
  Point2d pos(0, 0);

  std::vector<Point2d> neighbors = my_map.find_neighbors(pos);

  std::vector<Point2d> expected = {{0, 1}};
  EXPECT_EQ(neighbors, expected);
}

// Test Case: Corner Position (Bottom-Right)
TEST(FindNeighborsTest, BottomRightCorner)
{
  Map     my_map = UTILS_TEST::create_test_map();
  Point2d pos(2, 2);

  std::vector<Point2d> neighbors = my_map.find_neighbors(pos);

  std::vector<Point2d> expected = {{1, 2}};  // No valid neighbors
  EXPECT_EQ(neighbors, expected);
}

// Test Case: Left Edge (Now Order-Independent)
TEST(FindNeighborsTest, LeftEdge)
{
  Map     my_map = UTILS_TEST::create_test_map();
  Point2d pos(0, 1);

  std::vector<Point2d> neighbors = my_map.find_neighbors(pos);

  EXPECT_THAT(neighbors, ::testing::UnorderedElementsAre(Point2d(0, 0), Point2d(1, 1)));
}

// Test Case: Out-of-Bounds Position
TEST(FindNeighborsTest, OutOfBounds)
{
  Map     my_map = UTILS_TEST::create_test_map();
  Point2d pos(-1, -1);

  std::vector<Point2d> neighbors = my_map.find_neighbors(pos);

  EXPECT_TRUE(neighbors.empty());  // Should return empty vector
}

// Test Case: Position Inside an Obstacle
TEST(FindNeighborsTest, InsideObstacle)
{
  Map     my_map = UTILS_TEST::create_test_map();
  Point2d pos(2, 0);  // This is an obstacle

  std::vector<Point2d> neighbors = my_map.find_neighbors(pos);

  EXPECT_TRUE(neighbors.empty());  // No neighbors should be returned
}

// Test Case: Large Map Edge Case
TEST(FindNeighborsTest, LargeMapEdge)
{
  Map my_map;
  my_map.width  = 100;
  my_map.height = 100;
  my_map.data.resize(100 * 100, 0);  // All free cells

  Point2d pos(99, 99);  // Bottom-right corner

  std::vector<Point2d> neighbors = my_map.find_neighbors(pos);

  EXPECT_THAT(neighbors, ::testing::UnorderedElementsAre(Point2d(98, 99), Point2d(99, 98)));
}

// test detection of overlap of time intervals
// Test Case: No overlap
TEST(TimeIntervalIntersectionTest, NoOverlap)
{
  TimeInterval i1(0, 1);
  TimeInterval i2(2, 3);
  EXPECT_FALSE(overlap(i1, i2)) << "Intervals should not intersect.";
}

// Test Case: Overlap
TEST(TimeIntervalIntersectionTest, Overlap)
{
  TimeInterval i1(0, 2);
  TimeInterval i2(1, 3);
  EXPECT_TRUE(overlap(i1, i2)) << "Intervals should intersect.";
  EXPECT_TRUE(overlap(i2, i1)) << "Intervals should intersect.";
}

// Test Case: One Interval Inside Another
TEST(TimeIntervalIntersectionTest, Inside)
{
  TimeInterval i1(0, 4);
  TimeInterval i2(1, 3);
  EXPECT_TRUE(overlap(i1, i2)) << "Intervals should intersect.";
  EXPECT_TRUE(overlap(i2, i1)) << "Intervals should intersect.";
}

// Test Case: One Interval Touching Another
TEST(TimeIntervalIntersectionTest, Touching)
{
  TimeInterval i1(0, 1);
  TimeInterval i2(1, 2);
  EXPECT_TRUE(overlap(i1, i2)) << "Intervals should intersect.";
  EXPECT_TRUE(overlap(i2, i1)) << "Intervals should intersect.";
}

// Test Case: same interval
TEST(TimeIntervalIntersectionTest, SameInterval)
{
  TimeInterval i1(0, 1);
  EXPECT_TRUE(overlap(i1, i1)) << "Intervals should intersect.";
}

// Test Case: intervals of length 1
TEST(TimeIntervalIntersectionTest, LengthOne)
{
  TimeInterval i1(1, 1);
  TimeInterval i2(2, 2);
  EXPECT_FALSE(overlap(i1, i2)) << "Intervals should not intersect.";
  EXPECT_FALSE(overlap(i2, i1)) << "Intervals should not intersect.";
}


// Test conversion from timepointpath to Path simple
TEST(test_utils, timepointpath_to_path_simple)
{
  // generate a simple timepointpath
  TimePointPath tp_path;
  tp_path.push_back({{0, 1}, {0, 10}});
  tp_path.push_back({{0, 2}, {3, 6}});
  tp_path.push_back({{1, 2}, {6, 6}});
  tp_path.push_back({{2, 2}, {7, INT_MAX}});

  // convert to path
  Path path          = timepointpath_to_path(tp_path);
  Path expected_path = {{0, 1}, {0, 1}, {0, 1}, {0, 2}, {0, 2}, {0, 2}, {1, 2}, {2, 2}};

  // check that the path has the right length
  EXPECT_EQ(path.size(), expected_path.size()) << "Wrong path length.";

  // check that the path has the right time intervals
  EXPECT_EQ(path, expected_path) << "Wrong path.";
}

// Test conversion from timepointpath to Path
TEST(test_utils, timepointpath_to_path_random)
{
  // generate a random timepointpath
  const int     len     = 1000;
  TimePointPath tp_path = generate_random_timepointpath(len);

  // convert to path
  Path path = timepointpath_to_path(tp_path);

  // check that the path has the right length
  EXPECT_EQ(path.size(), len) << "Wrong path length.";

  // check that the path has the right time intervals
  for (auto it = tp_path.begin(); it != tp_path.end(); it++)
  {
    int t_max = it->interval.t_min;
    if (it != std::prev(tp_path.end()))
    {
      t_max = std::next(it)->interval.t_min - 1;
    }
    for (int t = it->interval.t_min; t <= t_max; t++)
    {
      EXPECT_EQ(path[t], it->position) << "Wrong position at time " << t;
    }
  }
}

// Test find_direction
TEST(test_utils, DirectionRight)
{
  Point2d p1(0, 0);
  Point2d p2(1, 0);

  EXPECT_EQ(find_direction(p1, p2), Direction::RIGHT) << "Wrong direction.";
}

TEST(test_utils, DirectionLeft)
{
  Point2d p1(1, 3);
  Point2d p2(0, 3);

  EXPECT_EQ(find_direction(p1, p2), Direction::LEFT) << "Wrong direction.";
}

TEST(test_utils, DirectionUp)
{
  Point2d p1(1, 1);
  Point2d p2(1, 0);

  EXPECT_EQ(find_direction(p1, p2), Direction::UP) << "Wrong direction.";
}

TEST(test_utils, DirectionDown)
{
  Point2d p1(2, 1);
  Point2d p2(2, 2);

  EXPECT_EQ(find_direction(p1, p2), Direction::DOWN) << "Wrong direction.";
}

TEST(test_utils, DirectionNone)
{
  Point2d p1(1, 1);
  Point2d p2(1, 1);

  EXPECT_EQ(find_direction(p1, p2), Direction::NONE) << "Wrong direction.";
}


// Color Generation test

// // Test intersection finding
// generate 100 random colors and check, whether they are valid (just a simple test to test the  unit test environment)
TEST(ColorGeneration, ValueCheck)
{
  for (int i = 0; i < 100; i++)
  {
    sf::Color col = generate_random_color(255, 255, 255, 0.25);
    EXPECT_TRUE(inRange(col.r, 0, 255)) << "Invalid value in red channel.";
    EXPECT_TRUE(inRange(col.g, 0, 255)) << "Invalid value in green channel.";
    EXPECT_TRUE(inRange(col.b, 0, 255)) << "Invalid value in blue channel.";
    EXPECT_TRUE(inRange(col.a, 0, 255)) << "Invalid value in alpha channel.";
  }
}
