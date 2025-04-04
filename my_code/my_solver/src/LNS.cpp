/*
 * Author: Jan Chleboun
 * Date: 08-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include "LNS.h"

#include <numeric>
#include <queue>
#include <random>

#include "SIPP.h"
#include "utils.h"


LNS::LNS(const Instance& instance_, std::mt19937& rnd_generator_, SharedData* shared_data_, const LNS_settings& settings_)
    : Solver("LNS", instance_, rnd_generator_),
      destroy_operator(Operator([this](Solution& sol) {
        if (settings.destroy_settings.type == DESTROY_TYPE::RANDOMWALK)
        {
          destroy_randomwalk(sol);
        }
        else if (settings.destroy_settings.type == DESTROY_TYPE::INTERSECTION)
        {
          destroy_intersection(sol);
        }
        else if (settings.destroy_settings.type == DESTROY_TYPE::ADAPTIVE)
        {
          destroy_adaptive(sol);
        }
        else
        {
          destroy_random(sol);
        }
      })),
      shared_data(shared_data_),
      settings(settings_),
      repair_operator(Operator([this](Solution& sol) { repair_default(sol); })),
      constraint_table(instance)
{
  // SIPP's lifetime can not be longer than LNS's lifetime, because then map_data might be deleted before SIPP
  planner = std::make_unique<SIPP>(instance, rnd_generator, settings.sipp_settings);

  // initialize destroy weights for adaptive LNS to 1
  if (settings.destroy_settings.type == DESTROY_TYPE::ADAPTIVE)
  {
    // the number of destroy operator is 1 less than destoroy types, because ADAPTIVE is not an operator
    destroy_weights.assign(magic_enum::enum_count<DESTROY_TYPE>() - 1, 1);
  }
}


void LNS::initialize_constraint_table(const std::vector<TimePointPath>& paths)
{
  assertm(!constraint_table_initialized, "Constraint table already initialized");
  constraint_table_initialized = true;
  // build the constraint table
  // Clock time_ct_build;
  // time_ct_build.start();
  constraint_table.build_sequential(solution.paths);
  // constraint_table.build_parallel(solution.paths);
  // constraint_table.build_parallel_2(solution.paths);
  // auto [time_wall, time_cpu] = time_ct_build.end();
  // std::cout << "Building the constraint_table took: " << time_wall << " s wall time and " << time_cpu << " s cpu_time" << std::endl;
}

void LNS::solve()
{
  // start measuring time
  Time::time_point start_find_initial_solution = Time::now();

  // calculate initial solution
  bool found = find_initial_solution();
  if (!found)
  {
    return;
  }
  assertm(found, "Could not find the initial solution");
  assertm(solution.is_valid(instance), "Found invalid solution");

  initial_solution_time = ((dsec)(Time::now() - start_find_initial_solution)).count();
  std::cout << "Initial solution found in " << initial_solution_time << " second." << std::endl;

  curr_time = initial_solution_time;

  // initialize the constraint table (needed for randomwalk and intersection destroy operator)
  if (settings.destroy_settings.type != DESTROY_TYPE::RANDOM && !constraint_table_initialized)
  {
    initialize_constraint_table(solution.paths);
  }

  // improve the solution
  while (iteration_num < settings.max_iter && curr_time < settings.time_limit)
  {
    // check for visualization thread end
    if (settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      if (shared_data->is_end.load(std::memory_order_acquire))
      {
        break;
      }
    }
    iteration_num++;

    // copy solution
    Solution perturbed_sol = solution;

    // destroy operator
    destroy_operator.apply(perturbed_sol);

    // repair operator
    repair_operator.apply(perturbed_sol);


    // accept only feasible solutions
    bool accepted    = false;
    int  improvement = 0;
    if (!perturbed_sol.feasible)
    {
      discard_solution(perturbed_sol, solution);
      std::cout << "Discarded infeasible solution." << std::endl;
    }
    else
    {
      // accept only improving solutions
      perturbed_sol.calculate_cost(instance);
      improvement = solution.sum_of_delays - perturbed_sol.sum_of_delays;

      // find out which destroy strategy was used
      auto destroy_strategy_index_help = magic_enum::enum_index<DESTROY_TYPE>(last_destroy_strategy);
      int  destroy_strategy_index      = -1;
      if (destroy_strategy_index_help.has_value())
      {
        destroy_strategy_index = destroy_strategy_index_help.value();
      }
      assertm(destroy_strategy_index >= 0 && destroy_strategy_index < static_cast<int>(magic_enum::enum_count<DESTROY_TYPE>()),
              "Wrong destroy strategy index.");

      if (improvement <= 0)
      {
        // TODO update the weights
        if (settings.destroy_settings.type == DESTROY_TYPE::ADAPTIVE)
        {
          destroy_weights[destroy_strategy_index] = (1 - decay_factor) * destroy_weights[destroy_strategy_index];
        }
        discard_solution(perturbed_sol, solution);
        // std::cout << "Discarded worse solution, difference: " << perturbed_sol.sum_of_costs - solution.sum_of_costs << std::endl;
      }
      else
      {
        // update the weights
        if (settings.destroy_settings.type == DESTROY_TYPE::ADAPTIVE)
        {
          destroy_weights[destroy_strategy_index] =
              reaction_factor * static_cast<double>(improvement) / static_cast<double>(settings.destroy_settings.size) +
              (1 - reaction_factor) * destroy_weights[destroy_strategy_index];
        }
        accepted = true;
        // remember the perturbed solution
        std::cout << "Solution improved by: " << solution.sum_of_delays - perturbed_sol.sum_of_delays << std::endl;
        solution = std::move(perturbed_sol);
      }
    }

    // construct LNS iteration info and add to the shared data
    if (settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      // get the pointer to the original perturbed solution (which might have been moved)
      Solution* sol = &solution;
      if (!accepted)
      {
        sol = &perturbed_sol;
      }
      LNSIterationInfo lns_info(iteration_num, accepted, improvement, sipp_info, *sol);
      sipp_info.clear();
      shared_data->update_lns_info(lns_info);
    }
  }
}

auto LNS::find_initial_solution() -> bool
{
  // create initial solution
  solution = PrioritizedPlanning(settings.sipp_settings.implementation);
  if (solution.feasible)
  {
    solution.calculate_cost(instance);
  }

  // construct LNS iteration info and add to the shared data
  if (settings.sipp_settings.info_type == INFO_type::visualisation)
  {
    LNSIterationInfo lns_info(0, solution.feasible, 0, sipp_info, solution);
    sipp_info.clear();
    shared_data->update_lns_info(lns_info);
  }

  std::cout << "Solution has sum of costs: " << solution.sum_of_costs << ", sol has delay: " << solution.sum_of_delays << std::endl;
  return solution.feasible;
}

auto LNS::PrioritizedPlanning(SIPP_implementation sipp_implementation) -> Solution
{
  Solution sol;
  sol.paths.resize(instance.get_num_of_agents(), TimePointPath());
  const int agent_num = instance.get_num_of_agents();

  // create a sequence from 0 to agent_num
  std::vector<int> priorities(agent_num);
  std::iota(priorities.begin(), priorities.end(), 0);

  // shuffle the sequence to get random priorities
  std::shuffle(priorities.begin(), priorities.end(), rnd_generator);

  // initialize iter info
  if (settings.sipp_settings.info_type == INFO_type::visualisation)
  {
    sipp_info.clear();
    sipp_info.resize(agent_num);
  }

  // plan path for each agent according to its priority
  for (int i = 0; i < agent_num; i++)
  {
    // get agent with priority i
    int agent_id = priorities[i];

    // plan path for agent
    TimePointPath tp_path;
    if (sipp_implementation == SIPP_implementation::SIPP_suboptimal)
    {
      assertm(settings.sipp_settings.w >= 1.0, "SIPP suboptimality factor must be 1.0 or more.");
      // suboptimal sipp
      tp_path = planner->plan_suboptimal(agent_id, settings.sipp_settings.w);
    }
    else
    {
      assertm(settings.sipp_settings.w == 1.0, "Suboptimality factor must be 1 for optimal algorithms.");
      // optimal sipp
      if (sipp_implementation == SIPP_implementation::SIPP_mine)
      {
        tp_path = planner->plan(agent_id);
      }
      else if (sipp_implementation == SIPP_implementation::SIPP_mapf_lns)
      {
        tp_path = planner->plan_mapflns_heuristic(agent_id);
      }
      else
      {
        throw std::runtime_error("Not implemented yet");
      }
    }

    // get the visualization info
    if (settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      sipp_info[agent_id] = std::move(planner->iter_info);
      planner->iter_info.clear();
    }

    // check validity
    if (tp_path.empty())
    {
      // std::cout << "Unable to find initial solution by prioritized planning. " << std::endl;
      sol.feasible = false;
      return sol;
    }

    // check validity
    assertm(instance.check_timepointpath_validity(tp_path), "SIPP planned an invalid timepointpath.");

    // update the safe interval table
    planner->safe_interval_table.add_constraints(tp_path);

    // add to the solution
    sol.paths[agent_id] = tp_path;
  }
  return sol;
}

void LNS::destroy_random(Solution& sol) const
{
  last_destroy_strategy = DESTROY_TYPE::RANDOM;
  assertm(settings.destroy_settings.size >= 0 && settings.destroy_settings.size <= instance.get_num_of_agents(),
          "Invalid neighborhood size.");
  std::vector<int> path_idcs(sol.paths.size());
  std::iota(path_idcs.begin(), path_idcs.end(), 0);
  std::shuffle(path_idcs.begin(), path_idcs.end(), rnd_generator);
  assertm((int)path_idcs.size() > settings.destroy_settings.size, "Not enough paths for the destroy operator.");
  int num_to_destroy = std::min(settings.destroy_settings.size, instance.get_num_of_agents());
  sol.destroyed_paths =
      std::vector(std::make_move_iterator(path_idcs.begin()), std::make_move_iterator(path_idcs.begin() + num_to_destroy));
  assertm(sol.destroyed_paths.size() > 0, "No paths were destroyed.");
  sol.feasible = false;
}

void LNS::destroy_randomwalk(Solution& sol) const
{
  last_destroy_strategy = DESTROY_TYPE::RANDOMWALK;

  assertm(constraint_table_initialized, "Constraint table is not initialized.");
  assertm(settings.destroy_settings.size >= 0 && settings.destroy_settings.size <= instance.get_num_of_agents(),
          "Invalid neighborhood size.");
  // if neighborhood size the same as number of agents or more, return all agents
  if (settings.destroy_settings.size >= instance.get_num_of_agents())
  {
    destroy_random(sol);
    return;
  }

  // the tabu list
  static std::unordered_set<int> tabu_list;

  // find the most delayed agent that is not on the tabu list
  assertm(static_cast<int>(sol.delays.size()) == instance.get_num_of_agents(),
          "The length of delay list must be the same as the numebr of agents.");

  int max_delay    = 0;
  int most_delayed = -1;
  int latest_time  = 0;
  for (int i = 0; i < instance.get_num_of_agents(); i++)
  {
    if (sol.paths[i].back().interval.t_min > latest_time)
    {
      latest_time = sol.paths[i].back().interval.t_min;
    }

    // skip agents in tabu list
    if (tabu_list.find(i) != tabu_list.end())
    {
      continue;
    }

    if (sol.delays[i] > max_delay)
    {
      max_delay    = sol.delays[i];
      most_delayed = i;
    }
  }

  // check, whether to reset tabu list
  if (max_delay == 0)
  {
    // reset the tabu list and try again
    if (!tabu_list.empty())
    {
      tabu_list.clear();
      destroy_randomwalk(sol);
      return;
    }

    // select random agents
    destroy_random(sol);
    return;
  }

  assertm(max_delay > 0, "Max delay is invalid.");
  assertm(most_delayed >= 0 && most_delayed < instance.get_num_of_agents() && sol.delays[most_delayed] == max_delay,
          "Invalid id of most delayed agent");

  std::unordered_set<int> chosen     = {most_delayed};
  int                     upperbound = sol.paths[most_delayed].back().interval.t_min - 1;
  assertm(upperbound > 0, "Upperbound is too small.");

  // perform the random walk, maximum 10 iterations
  for (int i = 0; i < 10; i++)
  {
    // choose a random agent
    std::uniform_int_distribution<int> dist(0, chosen.size() - 1);
    int                                idx = dist(rnd_generator);
    assertm(idx >= 0 && idx < static_cast<int>(chosen.size()), "Index out of bounds.");
    // iterator to the first element
    auto it = chosen.begin();
    std::advance(it, idx);
    int chosen_agent = *it;

    // choose a random time from its path
    int                                tmax = sol.paths[chosen_agent].back().interval.t_min - 1;
    std::uniform_int_distribution<int> dist_time(0, tmax);
    int                                chosen_t = dist_time(rnd_generator);

    // choose location
    int rw_start_location = location_at_time(sol.paths[chosen_agent], chosen_t);

    // perform randomwalk
    if (random_walk(chosen_agent, settings.destroy_settings.size, rw_start_location, chosen_t, upperbound, chosen))
    {
      break;
    }
  }

  // if not enough agents selected, fill with random agents
  // find all agents that were not selected yet
  std::vector<int> not_chosen;
  for (int i = 0; i < instance.get_num_of_agents(); i++)
  {
    if (chosen.find(i) == chosen.end())
    {
      not_chosen.push_back(i);
    }
  }
  int iterations = settings.destroy_settings.size - static_cast<int>(chosen.size());
  for (int i = 0; i < iterations; i++)
  {
    // choose a random element that was not yet chosen
    std::uniform_int_distribution<int> dist(0, not_chosen.size() - 1);
    int                                idx = dist(rnd_generator);
    auto                               it  = not_chosen.begin();
    std::advance(it, idx);
    chosen.insert(*it);    // add to the neighborhood
    not_chosen.erase(it);  // remove from the not yet chosen elements
  }
  assertm(static_cast<int>(chosen.size()) == settings.destroy_settings.size, "Neighborhood is not full.");
  sol.destroyed_paths = std::vector(std::make_move_iterator(chosen.begin()), std::make_move_iterator(chosen.end()));
  // shuffle the paths
  std::shuffle(sol.destroyed_paths.begin(), sol.destroyed_paths.end(), rnd_generator);
  assertm(sol.destroyed_paths.size() > 0, "No paths were destroyed.");
  sol.feasible = false;
}


auto LNS::random_walk(int agent_num, int neighborhood_size, int start_location, int start_time, int upperbound,
                      std::unordered_set<int>& chosen) const -> bool
{
  int curr = start_location;
  for (int t = start_time; t < upperbound; t++)
  {
    std::vector<int> next_locations = instance.get_neighbor_locations(curr);
    next_locations.push_back(curr);  // agent can also stay
    std::shuffle(next_locations.begin(), next_locations.end(), rnd_generator);
    // try all possible neighbors in random order
    for (auto loc : next_locations)
    {
      int next_h_val = instance.get_heuristic_distance(agent_num, loc);
      // if lowerbound on the time to reach goal smaller than limit, accept the move
      if (t + 1 + next_h_val < upperbound)
      {
        // get the agent that is in conflict and add it to the neighborhood
        int conflicting_agent = constraint_table.get_blocking_agent(curr, loc, t + 1);

        // if there is an agent in collision, add it to the neighborhood
        if (conflicting_agent != -1)
        {
          chosen.insert(conflicting_agent);
        }

        curr = loc;
        break;
      }
    }
    // test whether enough agents selected
    if (static_cast<int>(chosen.size()) == neighborhood_size)
    {
      return true;
    }
  }
  assertm(static_cast<int>(chosen.size()) <= neighborhood_size, "Enough agents selected, but false returned.");
  return false;
}

void LNS::destroy_intersection(Solution& sol) const
{
  last_destroy_strategy = DESTROY_TYPE::INTERSECTION;
  assertm(constraint_table_initialized, "Constraint table is not initialized.");

  // find all intersection vertices
  std::vector<int> intersection_vertices_free;
  intersection_vertices_free.reserve(instance.get_num_free_cells());  // upperbound on number of intersection vertices

  for (int i = 0; i < instance.get_num_free_cells(); i++)
  {
    const auto& agents_counts = constraint_table.get_agents_counts_free(i);
    if (static_cast<int>(agents_counts.size()) >= 3)
    {
      intersection_vertices_free.push_back(i);
    }
  }

  // create queue
  std::queue<int> openlist;

  // pick a random intersection vertex
  {
    std::uniform_int_distribution<int> dist(0, intersection_vertices_free.size() - 1);
    int                                idx    = dist(rnd_generator);
    int                                chosen = instance.free_location_to_location(intersection_vertices_free[idx]);
    openlist.push(chosen);
  }


  std::unordered_set<int> neighborhood;  // TODO try set
  std::vector<bool>       known(instance.get_num_cells(), false);

  while (!openlist.empty())
  {
    // retrieve current node
    int current = openlist.front();
    openlist.pop();
    // std::cout << "Current: " << current << std::endl;
    if (known[current])
    {
      continue;
    }
    known[current] = true;

    // check whether intersection vertex
    auto agents_counts = constraint_table.get_agents_counts(current);
    if (static_cast<int>(agents_counts.size()) >= 3)
    {
      // add agents that visit this vertex to the neighborhood
      get_intersection_agents(neighborhood, current);
    }

    // check whether enough agents selected
    // std::cout << "Selected: " << static_cast<int>(neighborhood.size()) << std::endl;
    if (settings.destroy_settings.size == static_cast<int>(neighborhood.size()))
    {
      break;
    }

    // add neighbors to the queue
    auto neighbors = instance.get_neighbor_locations(current);
    for (auto it : neighbors)
    {
      openlist.push(it);
    }
  }

  // if not enough agents selected, fill with random agents
  // find all agents that were not selected yet
  std::vector<int> not_chosen;
  for (int i = 0; i < instance.get_num_of_agents(); i++)
  {
    if (neighborhood.find(i) == neighborhood.end())
    {
      not_chosen.push_back(i);
    }
  }
  int iterations = settings.destroy_settings.size - static_cast<int>(neighborhood.size());
  for (int i = 0; i < iterations; i++)
  {
    // choose a random element that was not yet chosen
    std::uniform_int_distribution<int> dist(0, not_chosen.size() - 1);
    int                                idx = dist(rnd_generator);
    auto                               it  = not_chosen.begin();
    std::advance(it, idx);
    neighborhood.insert(*it);  // add to the neighborhood
    not_chosen.erase(it);      // remove from the not yet chosen elements
  }

  assertm(settings.destroy_settings.size == static_cast<int>(neighborhood.size()), "Wrong neighborhood size.");
  // convert to vector and shuffle
  sol.destroyed_paths = std::vector(std::make_move_iterator(neighborhood.begin()), std::make_move_iterator(neighborhood.end()));
  // shuffle the paths
  std::shuffle(sol.destroyed_paths.begin(), sol.destroyed_paths.end(), rnd_generator);
  assertm(sol.destroyed_paths.size() > 0, "No paths were destroyed.");
  sol.feasible = false;
}

void LNS::get_intersection_agents(std::unordered_set<int>& neighborhood, int current) const
{
  // find maximal T
  int t_max = constraint_table.get_last_constraint_start(current);

  // choose random time
  std::uniform_int_distribution<int> dist(0, t_max);
  int                                t     = dist(rnd_generator);
  int                                delta = 0;
  while (static_cast<int>(neighborhood.size()) < settings.destroy_settings.size && t + delta <= t_max && t - delta >= 0)
  {
    // find the agent, that blocks the cell at time t + delta
    int blocking_agent_1 = constraint_table.get_blocking_agent(current, current, t + delta);
    if (blocking_agent_1 >= 0)
    {
      neighborhood.insert(blocking_agent_1);
    }
    // find the agent, that blocks the cell at time t - delta
    int blocking_agent_2 = constraint_table.get_blocking_agent(current, current, t - delta);
    if (static_cast<int>(neighborhood.size()) < settings.destroy_settings.size && blocking_agent_2 >= 0)
    {
      neighborhood.insert(blocking_agent_2);
    }
    delta++;
  }
}

void LNS::destroy_adaptive(Solution& sol) const
{
  // choose strategy by roulette wheel selection
  // calculate the sum of weights
  auto sum_of_weights = std::reduce(destroy_weights.begin(), destroy_weights.end());
  assertm(sum_of_weights > 0, "Invalid weights.");

  // choose a random number in the range [0, sum_of_weights]
  std::uniform_real_distribution<double> dist(0.0, sum_of_weights);
  double                                 rand_value = dist(rnd_generator);

  // spin the roulette
  double       cummulative_sum = 0.0;
  DESTROY_TYPE chosen_type     = DESTROY_TYPE::ADAPTIVE;
  for (int i = 0; i < static_cast<int>(destroy_weights.size()); i++)
  {
    cummulative_sum += destroy_weights[i];
    if (rand_value <= cummulative_sum)
    {
      auto chosen = magic_enum::enum_cast<DESTROY_TYPE>(i);
      if (chosen.has_value())
      {
        chosen_type = chosen.value();
        break;
      }
      throw std::runtime_error("Unknown DESTROY_TYPE encountered in adaptive destroy.");
    }
  }
  assertm(chosen_type != DESTROY_TYPE::ADAPTIVE, "Did not select any destroy strategy.");

  // std::cout << "Weights are: " << std::endl;
  // for (auto it : destroy_weights)
  // {
  //   std::cout << it << ", ";
  // }
  // std::cout << std::endl;
  // std::cout << "Chosen destroy: " << magic_enum::enum_name<DESTROY_TYPE>(chosen_type) << std::endl;

  // apply the destroy operator
  switch (chosen_type)
  {
    case DESTROY_TYPE::RANDOM:
      destroy_random(sol);
      break;
    case DESTROY_TYPE::RANDOMWALK:
      destroy_randomwalk(sol);
      break;
    case DESTROY_TYPE::INTERSECTION:
      destroy_intersection(sol);
      break;
    default:
      throw std::runtime_error("Unknown DESTROY_TYPE encountered in adaptive destroy.");
  }
}

void LNS::repair_default(Solution& sol) const
{
  assertm(!sol.feasible, "Can not repair a feasible solution");
  assertm((int)sol.destroyed_paths.size() > 0, "There are no destroyed paths in the solution to be repaired.");

  if (settings.sipp_settings.info_type == INFO_type::visualisation)
  {
    sipp_info.clear();
    sipp_info.resize(sol.destroyed_paths.size());
  }

  // remove the destroyed paths
  for (auto& it : sol.destroyed_paths)
  {
    // std::cout << "Removing path: " << it << std::endl;
    planner->safe_interval_table.remove_constraints(sol.paths[it]);
    if (constraint_table_initialized)
    {
      constraint_table.remove_constraints(sol.paths[it], it);
    }
    sol.paths[it].clear();
  }
  sol.feasible = true;

  // replan
  for (int i = 0; i < static_cast<int>(sol.destroyed_paths.size()); i++)
  {
    int           path_idx = sol.destroyed_paths[i];
    TimePointPath tp_path  = planner->plan(path_idx);
    if (tp_path.empty())
    {
      // TODO reset the safe interval table and constrant table
      sol.feasible = false;
      return;
    }

    // get the visualization info
    if (settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      sipp_info[i] = std::move(planner->iter_info);
      planner->iter_info.clear();
    }

    planner->safe_interval_table.add_constraints(tp_path);
    if (constraint_table_initialized)
    {
      constraint_table.add_constraints(tp_path, path_idx);
    }
    sol.paths[path_idx] = tp_path;
  }
}

void LNS::discard_solution(const Solution& sol, const Solution& prev_sol) const
{
  for (int it : sol.destroyed_paths)
  {
    // remove the destroyed paths
    if (!sol.paths[it].empty())
    {
      planner->safe_interval_table.remove_constraints(sol.paths[it]);
      if (constraint_table_initialized)
      {
        constraint_table.remove_constraints(sol.paths[it], it);
      }
    }
  }

  // restore the old paths
  for (int it : sol.destroyed_paths)
  {
    planner->safe_interval_table.add_constraints(prev_sol.paths[it]);
    if (constraint_table_initialized)
    {
      constraint_table.add_constraints(prev_sol.paths[it], it);
    }
  }
}
