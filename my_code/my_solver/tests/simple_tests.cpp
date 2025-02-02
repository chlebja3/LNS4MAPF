#include <gtest/gtest.h>
#include "utils.h"
#include "Astar.h"

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

// test, whether nodes are compared the right way
TEST(NodeComparisonGt, ValueCheck)
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

TEST(NodeComparisonLt, ValueCheck)
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

TEST(NodeComparisonEq, ValueCheck)
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

// test Astar implementation
TEST(Astar, EdgeCases)
{
  std::vector<uint8_t> map_data = {1, 0, 0, 0};
  Path                 p        = ASTAR::find_path({0, 0}, {1, 1}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Obstacle on start not detected";

  p = ASTAR::find_path({1, 1}, {0, 0}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Obstacle on goal not detected";

  map_data = {1, 1, 1, 1};
  p        = ASTAR::find_path({1, 0}, {0, 1}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Obstacle on start and goal not detected";

  // start out of map
  map_data = {0, 0, 0, 0};
  p        = ASTAR::find_path({2, 0}, {0, 0}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Invalid start not detected";

  p = ASTAR::find_path({-1, 0}, {0, 0}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Invalid start not detected";

  p = ASTAR::find_path({0, -1}, {1, 1}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Invalid start not detected";

  p = ASTAR::find_path({0, 2}, {1, 1}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Invalid start not detected";

  // goal out of map
  p = ASTAR::find_path({1, 0}, {-1, 0}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Invalid goal not detected";

  p = ASTAR::find_path({1, 0}, {2, 0}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Invalid goal not detected";

  p = ASTAR::find_path({0, 0}, {1, -1}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Invalid goal not detected";

  p = ASTAR::find_path({0, 1}, {1, 2}, map_data, 2, 2);
  EXPECT_TRUE(p.size() == 0) << "Invalid goal not detected";
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

