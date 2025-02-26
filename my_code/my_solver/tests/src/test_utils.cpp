/*
 * Author: Jan Chleboun
 * Date: 12-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "test_utils.h"

#include <gtest/gtest.h>

#include <climits>
#include <filesystem>


std::string get_base_path_tests()
{
  return std::filesystem::canonical("/proc/self/exe").parent_path().parent_path().parent_path().string();  // Linux only
}

TimePointPath generate_random_timepointpath(int len_wanted)
{
  int                      len             = len_wanted - 1;  // decrease the length as the last element has to be handled separately
  TimePointPath            tp_path         = {};
  int                      i               = 0;
  int                      cur_loc         = 0;  // start at (0, 0)
  const int                instance_width  = 5;  // lets say the instance has width 5
  const std::array<int, 4> possible_shifts = {-instance_width, -1, 1, instance_width};
  while (i < len)
  {
    // choose a random count of elements (maximum 10)
    int count = std::min(rand() % 10 + 1, len - i);

    // choose a random length of the interval
    int interval_len = i + count - 1 + rand() % (INT_MAX - i - count + 1);

    // add the timepoint to the path
    tp_path.emplace_back(cur_loc, TimeInterval(i, interval_len));

    // increment i
    i += count;

    // choose a random neighboring position
    cur_loc = cur_loc + possible_shifts[rand() % 4];

    // make sure the position is valid
    while (cur_loc < 0 || cur_loc > instance_width * instance_width) cur_loc = cur_loc + possible_shifts[rand() % 4];
  }
  // last interval
  tp_path.emplace_back(cur_loc, TimeInterval(i, INT_MAX));

  return tp_path;
}

auto filter_time_intervals(std::pair<std::list<TimeInterval>::const_iterator, std::list<TimeInterval>::const_iterator> start_goal,
                           TimeInterval interval, int from, int to, ConstraintTable& table) -> std::vector<TimeInterval>
{
  std::list<TimeInterval>::const_iterator start;
  std::list<TimeInterval>::const_iterator end;
  std::tie(start, end) = start_goal;
  std::vector<TimeInterval> ret;

  int reach_time = 0;
  if (interval.t_min == INT_MAX)
  {
    reach_time = INT_MAX;
  }
  else
  {
    reach_time = interval.t_min + 1;
  }

  for (auto it = start; it != end; it++)
  {
    TimeInterval new_interval(it->t_min, it->t_max);
    // modify the interval to be valid
    if (new_interval.t_min < reach_time)
    {
      new_interval.t_min = reach_time;
    }
    assertm(new_interval.t_min <= new_interval.t_max, "Modifying the time interval made it invalid.");

    // check edge collision
    if (table.edge_constraint_table.get(to, from, new_interval.t_min))
    {
      continue;
    }

    // add to the covnerted intervals
    ret.push_back(new_interval);
  }
  return ret;
}

