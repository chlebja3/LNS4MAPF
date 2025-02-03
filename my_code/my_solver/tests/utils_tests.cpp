/*
 * Author: Jan Chleboun
 * Date: 03-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <gtest/gtest.h>

// Include the header where Point2d is declared
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

// Color Generation test
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
