#include <gtest/gtest.h>
#include "utils.h"
#include "Astar.h"

// test Point2d class


/* test Astar implementation
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
*/

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

