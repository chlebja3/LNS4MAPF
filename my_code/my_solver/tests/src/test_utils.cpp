/*
 * Author: Jan Chleboun
 * Date: 12-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "test_utils.h"

#include <gtest/gtest.h>

#include <filesystem>

std::string get_base_path_tests()
{
  return std::filesystem::canonical("/proc/self/exe").parent_path().parent_path().parent_path().string();  // Linux only
}

TimePointPath generate_random_timepointpath(int len)
{
  TimePointPath                tp_path         = {};
  int                          i               = 0;
  Point2d                      cur_pos         = {0, 0};  // start at (0, 0)
  const std::array<Point2d, 4> possible_shifts = {Point2d{0, 1}, Point2d{0, -1}, Point2d{1, 0}, Point2d{-1, 0}};
  while (i < len)
  {
    // choose a random count of elements (maximum 10)
    int count = std::min(rand() % 10 + 1, len - i);

    // add the timepoint to the path
    tp_path.emplace_back(cur_pos, TimeInterval(i, i + count - 1));

    // increment i
    i += count;

    // choose a random neighboring position
    cur_pos = cur_pos + possible_shifts[rand() % 4];
  }
  return tp_path;
}

