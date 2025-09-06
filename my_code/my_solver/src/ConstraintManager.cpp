/*
 * Author: Jan Chleboun
 * Date: 21-06-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include "ConstraintManager.h"

#include <climits>

void ConstraintManager::add_trajectory(const TimePointPath& path, int agent_num)
{
  TimeIntervalNode_it prev_interval = std::nullopt;
  for (int i = 0; i < static_cast<int>(path.size()); i++)
  {
    // add to the safe interval table, which will add it to the constraint table also
    // std::cout << "Adding constraint. Location: " << path[i].location << ", interval: " << path[i].interval
    //           << ", agent_num: " << agent_num << std::endl;

    prev_interval = safe_interval_table.add_constraint(path[i], agent_num, constraint_table, prev_interval);
    // assertm(is_valid(), "Adding constraint made CM invalid.");

    // add to the edge table
    if (i > 0)  // skip the first point as there is no edge
    {
      // the time when the edge is used is the start time of the later interval
      edge_table.add(path[i - 1].location, path[i].location, path[i].interval.t_min, agent_num);
    }
  }
}

bool ConstraintManager::is_valid()
{
  for (int i = 0; i < instance.get_num_free_cells(); i++)
  {
    int loc = instance.free_location_to_location(i);
    auto prev = get_first_time_interval(loc);
    assertm(prev.has_value(), "First time interval must have value.");
    assertm(prev.value()->location == loc, "First interval must be associated with this location.");
    if (prev.value()->interval.t_min != 0)
    {
      std::cout << "First time interval must start at 0";
      return false;
    }
    auto curr = prev.value()->next_interval;
    if (!curr.has_value())
    {
      // prev must take the whole time
      // there can not be any previous or next intervals
      bool valid = prev.value()->interval.t_min == 0 && prev.value()->interval.t_max == INT_MAX &&
                   !prev.value()->prev_interval.has_value() && !prev.value()->next_interval.has_value();
      if (!valid)
      {
        return false;
      }
    }
    while (curr.has_value())
    {
      // check whether the right location is used
      if (curr.value()->location != loc)
      {
        std::cout << "Interval " << curr.value()->interval << " with location " << curr.value()->location
                  << " is not associated with the current location " << loc << std::endl;
        return false;
      }
      if (prev.value()->interval.t_max + 1 != curr.value()->interval.t_min)
      {
        std::cout << "Intervals " << prev.value()->interval << " and " << curr.value()->interval
                  << " are not directly after each other." << std::endl;
        return false;
      }
      // check whether previous interval is right
      assertm(prev.has_value() && curr.has_value() && curr.value()->prev_interval.has_value(),
              "Prev and curr and curr->prev must have value at this point. ");
      const TimeIntervalNode prev1 = *prev.value();
      const TimeIntervalNode prev2 = *(curr.value()->prev_interval.value());
      if (prev1.agent_num != prev2.agent_num || prev1.type != prev2.type ||
          prev1.interval.t_min != prev2.interval.t_min || prev1.interval.t_max != prev2.interval.t_max)
      {
        std::cout << "Wrong prev interval, prev: " << prev.value()->interval << ", agent: " << prev.value()->agent_num
                  << ", curr->prev: " << curr.value()->prev_interval.value()->interval
                  << ", agent:" << curr.value()->prev_interval.value()->agent_num << std::endl;
        return false;
      }
      prev = curr;
      curr = curr.value()->next_interval;
    }
    if (prev.value()->interval.t_max != INT_MAX)
    {
      std::cout << "The last interval " << prev.value()->interval << " associated with location "
                << prev.value()->location << " does not end at infinity." << std::endl;
      return false;
    }
  }
  return true;
}

/*
#ifdef CM_PARALLELIZATION
void ConstraintManager::add_trajectory_parallel(const TimePointPath& path, int agent_num)
{
#pragma omp parallel for
  for (int i = 0; i < (int)path.size(); i++)
  {
    // lock the location
    omp_set_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);

    safe_interval_table.add_constraint(path[i], agent_num, constraint_table);
    if (i > 0)
    {
      // the time when the edge is used is the start time of the later interval
      edge_table.add(path[i - 1].location, path[i].location, path[i].interval.t_min);
    }
    // release the lock
    omp_unset_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);
  }
}
#endif
*/

void ConstraintManager::remove_trajectory(const TimePointPath& path, int agent_num)
{
  for (int i = 0; i < (int)path.size(); i++)
  {
    // remove the constraint from safe interval table, removes it also from constraint table
    // std::cout << "Removing constraint. Location: " << path[i].location << ", interval: " << path[i].interval
    //           << ", agent_num: " << agent_num << std::endl;
    safe_interval_table.remove_constraint(path[i], agent_num, constraint_table);
    // assertm(is_valid(), "Removing constraint made CM invalid.");

    // remove the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      edge_table.remove(path[i - 1].location, path[i].location, path[i].interval.t_min);
    }
  }
}

/*
#ifdef CM_PARALLELIZATION
void SafeIntervalTable::remove_constraints_parallel(const TimePointPath& path)
{
#pragma omp parallel for
  for (int i = 0; i < (int)path.size(); i++)
  {
    // lock the location
    omp_set_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);

    // remove the point constraint
    remove_constraint(path[i]);

    // remove the edge constraint
    if (i > 0)  // skip the first point as there is no edge
    {
      edge_table.remove(path[i - 1].location, path[i].location, path[i].interval.t_min);
    }

    // unlock the location
    omp_unset_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);
  }
}
#endif
*/

void ConstraintManager::build_sequential(const std::vector<TimePointPath>& paths)
{
  assertm(static_cast<int>(paths.size()) > 0, "No paths to add.");
  for (int i = 0; i < static_cast<int>(paths.size()); i++)
  {
    add_trajectory(paths[i], i);
  }
}

[[nodiscard]] auto ConstraintManager::get_blocking_agent(int from, int to, int time) const -> std::pair<int, int>
{
  assertm(instance.get_map_data().is_in(from) && instance.get_map_data().is_in(to), "Invalid location.");
  int vertex_constraint = constraint_table.get_blocking_agent(to, time);

  // find edge constraint
  int edge_constraint = -1;
  if (from != to)
  {
    // we want to look for opposite edge, because there can not be any edge ending in to location without location
    // constraint
    edge_constraint = edge_table.get(to, from, time);
  }
  return std::make_pair(vertex_constraint, edge_constraint);
}

[[nodiscard]] auto ConstraintManager::get_first_time_interval(int location) -> TimeIntervalNode_const_it
{
  auto first_si = safe_interval_table.get_first_safe_interval(location);
  auto first_co = constraint_table.get_first_constraint(location);
  assertm(first_si.has_value() || first_co.has_value(), "There has to be at least one safe interval or constraint.");
  auto curr = first_si;
  if (!curr.has_value() ||
      (first_co.has_value() && first_co.value()->interval.t_max < first_si.value()->interval.t_min))
  {
    curr = first_co;
  }
  return curr;
}

[[nodiscard]] auto ConstraintManager::get_all_intervals(int location, const TimeInterval& time_interval) const
    -> std::pair<std::list<TimeIntervalNode>::const_iterator, std::list<TimeIntervalNode>::const_iterator>
{
  auto [first_si, last_si] = get_safe_intervals(location, time_interval);
  // std::cout << "first: " << first_si->interval << ", is end: " << is_end_of_list(location, first_si)
  //           << ", last: " << is_end_of_list(location, last_si) << std::endl;
  // no safe intervals
  if (is_end_of_list(location, first_si))
  {
    return {first_si, first_si};
  }
  TimeIntervalNode_const_it first = {first_si};
  assertm(first.has_value(), "The first safe interval must have value.");
  // find the first interval
  while (first.value()->interval.t_min > time_interval.t_min)
  {
    assertm(first.has_value(), "The interval must have value.");
    if (first.value()->interval.t_min == 0) break;
    first = first.value()->prev_interval;
  }

  TimeIntervalNode_const_it last = {last_si};
  // if last is cend, go further from prev safe interval
  if (is_end_of_list(location, last_si))
  {
    if (time_interval.t_max < INT_MAX)
    {
      last = {std::prev(last_si)};  // start with the prev safe interval
      while (last.has_value() && last.value()->interval.t_min <= time_interval.t_max)  // find first interval after
      {
        last = last.value()->next_interval;
        // assertm(last.has_value(), "The interval must have value.");
      }
    }
    if (!last.has_value()) last = {last_si};
  }
  else
  {
    assertm(last.has_value(), "The last interval must have value.");
    while (last.value()->prev_interval.value()->interval.t_min > time_interval.t_max)
    {
      assertm(last.has_value(), "The interval must have value.");
      last = last.value()->prev_interval;
    }
    assertm(first.has_value() && last.has_value(), "First and last must have value.");
  }
  return std::make_pair(first.value(), last.value());
}

/*
#ifdef CM_PARALLELIZATION
void ConstraintTable::build_parallel(const std::vector<TimePointPath>& paths)
{
#pragma omp parallel for
  for (int agent_num = 0; agent_num < static_cast<int>(paths.size()); agent_num++)
  {
    const TimePointPath& path = paths[agent_num];
    for (int i = 0; i < (int)path.size(); i++)
    {
      // add the point constraint
      omp_set_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);
      add_constraint(path[i], agent_num);
      // add the edge constraint
      if (i > 0)  // skip the first point as there is no edge
      {
        // the time when the edge is used is the start time of the later interval
        edge_constraint_table.add(path[i - 1].location, path[i].location, path[i].interval.t_min, agent_num);
      }
      omp_unset_lock(&locks.locks[instance.location_to_free_location(path[i].location)]);
    }
  }
}

void ConstraintTable::build_parallel_2(const std::vector<TimePointPath>& paths)
{
  // calculate how many constraints to add
  int total_work = 0;
  for (const auto& path : paths)
  {
    total_work += static_cast<int>(path.size());
  }
  assertm(total_work > 0, "No constraints to add.");

  std::vector<const TimePoint*> timepoints(total_work);
  std::vector<int>              agent_nums(total_work);
  std::vector<int>              prev_location(total_work, -1);
  int                           idx = 0;
  for (int i = 0; i < static_cast<int>(paths.size()); i++)
  {
    const auto& path = paths[i];
    for (int j = 0; j < (int)path.size(); j++)
    {
      timepoints[idx] = &path[j];
      agent_nums[idx] = i;
      if (j > 0)
      {
        prev_location[idx] = path[j - 1].location;
      }
      idx++;
    }
  }

#pragma omp parallel for
  for (int i = 0; i < total_work; i++)
  {
    // add the point constraint
    omp_set_lock(&locks.locks[instance.location_to_free_location(timepoints[i]->location)]);
    add_constraint(*timepoints[i], agent_nums[i]);
    // add the edge constraint
    if (prev_location[i] != -1)  // skip the first point as there is no edge
    {
      // the time when the edge is used is the start time of the later interval
      edge_constraint_table.add(prev_location[i], timepoints[i]->location, timepoints[i]->interval.t_min,
agent_nums[i]);
    }
    omp_unset_lock(&locks.locks[instance.location_to_free_location(timepoints[i]->location)]);
  }
}
#endif
*/
