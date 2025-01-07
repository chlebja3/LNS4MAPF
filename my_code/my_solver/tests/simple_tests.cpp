#include <gtest/gtest.h>
#include "utils.h"

// a custom assertion to check whether the value is in expected range, inclusive
::testing::AssertionResult inRange(int val, int a, int b)
{
    if((val >= a) && (val <= b))
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

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

