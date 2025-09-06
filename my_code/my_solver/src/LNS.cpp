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
#include <set>

#include "SIPP.h"
#include "utils.h"

#define MIN_BLOCKED_THRESHOLD 0.01
#define BLOCKED_REACTION_FACTOR 0.1

LNS::LNS(const Instance& instance_, std::mt19937& rnd_generator_, SharedData* shared_data_, LNS_settings& settings_)
    : Solver("LNS", instance_, rnd_generator_),
      destroy_operator(Operator(
          [this](Solution& sol)
          {
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
            else if (settings.destroy_settings.type == DESTROY_TYPE::RANDOM_CHOOSE)
            {
              destroy_random_choose(sol);
            }
            else if (settings.destroy_settings.type == DESTROY_TYPE::BLOCKED)
            {
              destroy_blocked(sol);
            }
            else
            {
              destroy_random(sol);
            }
          })),
      settings(settings_),
      shared_data(shared_data_),
      repair_operator(Operator([this](Solution& sol) { repair_default(sol); })),
      constraint_manager(instance)
{
  // SIPP's lifetime can not be longer than LNS's lifetime, because then map_data might be deleted before SIPP
  planner = std::make_unique<SIPP>(instance, rnd_generator, settings.sipp_settings, constraint_manager);
  psipp = std::make_unique<PSIPP>(instance, rnd_generator, settings.psipp_settings, constraint_manager);

  // initialize destroy weights for adaptive LNS to 1
  if (settings.destroy_settings.type == DESTROY_TYPE::ADAPTIVE)
  {
    // the number of destroy operator is 1 less than destoroy types, because ADAPTIVE is not an operator
    // destroy_weights.assign(magic_enum::enum_count<DESTROY_TYPE>() - 1, 1);
    destroy_weights.assign(3, 1);  // RANDOM, RANDOMWALK, INTERSECTION
  }
}

void LNS::solve()
{
  // start measuring time
  Clock clock;
  clock.start();

  // calculate initial solution
  while (!found_initial_solution && clock.get_current_time().first < settings.time_limit)
  {
    found_initial_solution = find_initial_solution();

    // reset the safe interval table and already planned if needed
    if (!found_initial_solution)
    {
      std::cout << "RESETTING CONSTRAINT MAKANGER" << std::endl;
      constraint_manager.reset();
      already_planned.clear();
    }

    // restart only if the settings allow it
    if (!settings.restarts)
    {
      break;
    }
  }

  if (!found_initial_solution || settings.max_iter == 0)
  {
    return;
  }
  assertm(found_initial_solution, "Could not find the initial solution");
  assertm(solution.is_valid(instance), "Found invalid solution");

  // std::cout << "Initial solution found in " << initial_solution_time << " second." << std::endl;

  // improve the solution
  while (iteration_num < settings.max_iter && clock.get_current_time().first < settings.time_limit)
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

    // time the iteration
    Clock iteration_clock;
    iteration_clock.start();

    // copy solution
    Solution perturbed_sol = solution;

    // destroy operator
    destroy_operator.apply(perturbed_sol);

    // repair operator
    repair_operator.apply(perturbed_sol);

    // accept only feasible solutions
    bool accepted = false;
    int improvement = 0;
    if (!perturbed_sol.feasible)
    {
      discard_solution(perturbed_sol, solution);
      // std::cout << "Discarded infeasible solution." << std::endl;
    }
    else
    {
      // accept only improving solutions
      perturbed_sol.calculate_cost(instance);
      improvement = solution.sum_of_delays - perturbed_sol.sum_of_delays;

      // find out which destroy strategy was used
      auto destroy_strategy_index_help = magic_enum::enum_index<DESTROY_TYPE>(last_destroy_strategy);
      int destroy_strategy_index = -1;
      if (destroy_strategy_index_help.has_value())
      {
        destroy_strategy_index = destroy_strategy_index_help.value();
      }
      assertm(destroy_strategy_index >= 0 &&
                  destroy_strategy_index < static_cast<int>(magic_enum::enum_count<DESTROY_TYPE>()),
              "Wrong destroy strategy index.");

      if (improvement <= 0)
      {
        // update the weights
        if (settings.destroy_settings.type == DESTROY_TYPE::ADAPTIVE)
        {
          destroy_weights[destroy_strategy_index] =
              (1 - BLOCKED_REACTION_FACTOR) * destroy_weights[destroy_strategy_index];
        }
        if (last_destroy_strategy == DESTROY_TYPE::BLOCKED)
        {
          threshold_blocked = std::max((1 - BLOCKED_REACTION_FACTOR) * threshold_blocked, MIN_BLOCKED_THRESHOLD);
        }
        discard_solution(perturbed_sol, solution);
        // std::cout << "Discarded worse solution, difference: " << perturbed_sol.sum_of_costs - solution.sum_of_costs
        // << std::endl;
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
        else if (last_destroy_strategy == DESTROY_TYPE::BLOCKED)
        {
          threshold_blocked = std::min((1 + reaction_factor) * threshold_blocked, 1.0);
        }
        accepted = true;
        // remember the perturbed solution
        // std::cout << "Solution improved by: " << solution.sum_of_delays - perturbed_sol.sum_of_delays << std::endl;
        solution = std::move(perturbed_sol);
      }
    }
    auto [iteration_time_wall, iteration_time_cpu] = iteration_clock.end();

    // construct LNS iteration info and add to the shared data
    if (!settings.use_psipp && settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      // get the pointer to the original perturbed solution (which might have been moved)
      Solution* sol = &solution;
      if (!accepted)
      {
        sol = &perturbed_sol;
      }
      LNSIterationInfo lns_info(iteration_num, accepted, improvement, sipp_info, *sol,
                                (std::string)magic_enum::enum_name<DESTROY_TYPE>(last_destroy_strategy));
      sipp_info.clear();
      shared_data->update_lns_info(lns_info);
    }
    // log the iteration
    else if (settings.sipp_settings.info_type == INFO_type::experiment)
    {
      log.bsf_solution_cost.push_back(solution.sum_of_costs);
      log.bsf_makespan.push_back(solution.makespan);
      log.used_operator.push_back(last_destroy_strategy);
      log.iteration_time_wall.push_back(iteration_time_wall);
      log.iteration_time_cpu.push_back(iteration_time_cpu);
    }
  }
}

auto LNS::find_initial_solution() -> bool
{
  Clock clock;
  clock.start();

  // create initial solution
  solution = PrioritizedPlanning();
  auto [init_sol_time_wall, init_sol_time_cpu] = clock.get_current_time();
  solution.calculate_cost(instance);

  // construct LNS iteration info and add to the shared data
  if (settings.sipp_settings.info_type == INFO_type::visualisation)
  {
    LNSIterationInfo lns_info(0, solution.feasible, 0, sipp_info, solution, "None");
    sipp_info.clear();
    shared_data->update_lns_info(lns_info);
  }
  // log the experiment data
  else if (settings.sipp_settings.info_type == INFO_type::experiment)
  {
    log.bsf_solution_cost.push_back(solution.sum_of_costs);
    log.bsf_makespan.push_back(solution.makespan);
    log.used_operator.push_back(DESTROY_TYPE::ADAPTIVE);
    log.iteration_time_wall.push_back(init_sol_time_wall);
    log.iteration_time_cpu.push_back(init_sol_time_cpu);
  }

  // std::cout << "Solution has sum of costs: " << solution.sum_of_costs << ", sol has delay: " <<
  // solution.sum_of_delays << std::endl;
  return solution.feasible;
}

auto LNS::PrioritizedPlanning() -> Solution
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
  if (settings.use_psipp)
  {
    if (settings.psipp_settings.info_type == INFO_type::visualisation)
    {
      psipp_info.clear();
      psipp_info.resize(agent_num);
    }
  }
  else
  {
    if (settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      sipp_info.clear();
      sipp_info.resize(agent_num);
    }
  }

  // plan path for each agent according to its priority
  assertm(already_planned.empty(), "Running Prioritized Planning when some agents were already planned.");
  for (int i = 0; i < agent_num; i++)
  {
    // get agent with priority i
    int agent_id = priorities[i];

    TimePointPath tp_path;
    // plan path for agent
    // std::cout << "  Planning path of agent " << agent_id << "." << std::endl;
    if (settings.use_psipp)
    {
      std::unordered_set<int> to_replan = {agent_id};
      std::unordered_set<int> erased_from_to_replan = {};
      while (!to_replan.empty())
      {
        auto it = to_replan.begin();
        // std::cout << "to replan: " << *it << std::endl;
        if (already_planned.find(*it) != already_planned.end())
        {
          // std::cout << "agent " << *it << " was already planned, removing." << std::endl;
          constraint_manager.remove_trajectory(sol.paths[*it], *it);
        }
        std::unordered_set<int> to_replan_new;
        std::tie(tp_path, to_replan_new) = psipp->plan(*it, already_planned);
        // print_timepointpath(tp_path);
        if (tp_path.empty())
        {
          std::cout << "Empty path, fail." << std::endl;
          sol.feasible = false;
          return sol;
        }
        assertm(instance.check_timepointpath_validity(tp_path), "PSIPP planned an invalid timepointpath.");
        for (auto id : to_replan_new)
        {
          // std::cout << "   Added " << id << " into to_replan." << std::endl;
          // remove trajectory
          if (already_planned.find(id) != already_planned.end())
          {
            if (erased_from_to_replan.find(id) != erased_from_to_replan.end())
            {
              // caught in a cycle
              std::cout << "Cycle detected." << std::endl;
              sol.feasible = false;
              return sol;
            }
            // std::cout << "id: " << id << " was already planned, removing." << std::endl;
            constraint_manager.remove_trajectory(sol.paths[id], id);
            sol.paths[id] = TimePointPath();
            already_planned.erase(id);
            erased_from_to_replan.insert(id);
          }
        }
        // std::cout << "Adding to CM agent " << *it << "." << std::endl;
        constraint_manager.add_trajectory(tp_path, *it);
        // add to the solution
        sol.paths[*it] = tp_path;
        already_planned.insert(*it);
        to_replan.erase(it);
        to_replan.insert(to_replan_new.begin(), to_replan_new.end());
      }
    }
    else
    {
      tp_path = planner->plan(agent_id, already_planned);
    }
    // std::cout << "Planned" << std::endl;

    // get the visualization info
    if (!settings.use_psipp && settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      sipp_info[agent_id] = planner->get_iter_info();
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

    if (!settings.use_psipp)
    {
      // update the safe interval table
      constraint_manager.add_trajectory(tp_path, agent_id);

      // add to the solution
      sol.paths[agent_id] = tp_path;

      // add to the already planned agents
      already_planned.insert(agent_id);
    }
  }
  assertm(constraint_manager.is_valid(), "Invalid structure of constraint manager.");
  // add priorities to the visualization info
  if (settings.sipp_settings.info_type == INFO_type::visualisation)
  {
    sol.priorities = std::move(priorities);
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
  sol.destroyed_paths = std::vector(std::make_move_iterator(path_idcs.begin()),
                                    std::make_move_iterator(path_idcs.begin() + num_to_destroy));
  assertm(sol.destroyed_paths.size() > 0, "No paths were destroyed.");
  sol.feasible = false;
}

void LNS::destroy_randomwalk(Solution& sol) const
{
  last_destroy_strategy = DESTROY_TYPE::RANDOMWALK;

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

  int max_delay = 0;
  int most_delayed = -1;
  for (int i = 0; i < instance.get_num_of_agents(); i++)
  {
    // skip agents in tabu list
    if (tabu_list.find(i) != tabu_list.end())
    {
      continue;
    }

    if (sol.delays[i] > max_delay)
    {
      max_delay = sol.delays[i];
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
    std::cout << "Using random destroy" << std::endl;
    destroy_random(sol);
    return;
  }

  assertm(max_delay > 0, "Max delay is invalid.");
  assertm(most_delayed >= 0 && most_delayed < instance.get_num_of_agents() && sol.delays[most_delayed] == max_delay,
          "Invalid id of most delayed agent");

  // update the tabu list
  tabu_list.insert(most_delayed);
  if (static_cast<int>(tabu_list.size()) == instance.get_num_of_agents())
  {
    tabu_list.clear();
  }

  std::set<int> chosen = {most_delayed};  // TODO in the original code they use set, but I think it makes more sense to
                                          // use either unordered set or vector
  int upperbound = sol.paths[most_delayed].back().interval.t_min;
  assertm(upperbound > 0, "Upperbound is too small.");
  // perform the first randomwalk from start - not in pseudocode, but in MAPF-LNS2 implementation
  int chosen_agent = most_delayed;
  if (!random_walk(chosen_agent, settings.destroy_settings.size, sol.paths[chosen_agent][0].location, 0, upperbound,
                   chosen))
  {
    // perform the random walk again, maximum 10 iterations
    for (int i = 0; i < 10; i++)
    {
      // choose a random time from the agents path
      std::uniform_int_distribution<int> dist_time(0, upperbound);
      int chosen_t = dist_time(rnd_generator);

      // choose location
      int rw_start_location = location_at_time(sol.paths[chosen_agent], chosen_t);
      assertm(rw_start_location >= 0, "Invalid start location of randomwalk");

      // perform randomwalk
      if (random_walk(chosen_agent, settings.destroy_settings.size, rw_start_location, chosen_t, upperbound, chosen))
      {
        break;
      }
      assertm(static_cast<int>(chosen.size()) < settings.destroy_settings.size,
              "Enough agents were selected, but false returned.");

      // choose a random agent
      std::uniform_int_distribution<int> dist(0, chosen.size() - 1);
      int idx = dist(rnd_generator);
      assertm(idx >= 0 && idx < static_cast<int>(chosen.size()), "Index out of bounds.");
      // iterator to the first element
      auto it = chosen.begin();
      std::advance(it, idx);
      chosen_agent = *it;
      upperbound = sol.paths[chosen_agent].back().interval.t_min;
    }
  }

  // if not enough agents selected, repeat
  if (static_cast<int>(chosen.size()) <= 1)
  {
    destroy_randomwalk(sol);
    return;
  }
  assertm(static_cast<int>(chosen.size()) > 1, "Not enough agents selected.");

  // but it is implemented like this in MAPF-LNS2
  // sol.destroyed_paths = std::vector(std::make_move_iterator(chosen.begin()), std::make_move_iterator(chosen.end()));
  sol.destroyed_paths.assign(chosen.begin(), chosen.end());

  std::shuffle(sol.destroyed_paths.begin(), sol.destroyed_paths.end(), rnd_generator);
  assertm(sol.destroyed_paths.size() > 1, "Not enough paths were destroyed.");
  sol.feasible = false;
}

auto LNS::random_walk(int agent_num, int neighborhood_size, int start_location, int start_time, int upperbound,
                      std::set<int>& chosen) const -> bool
{
  assertm(static_cast<int>(chosen.size()) < neighborhood_size, "Randomwalk not needed.");
  int curr = start_location;
  for (int t = start_time; t < upperbound; t++)
  {
    std::vector<int> next_locations = instance.get_neighbor_locations(curr);
    next_locations.push_back(curr);  // agent can also stay
    std::shuffle(next_locations.begin(), next_locations.end(), rnd_generator);
    // try all possible neighbors in random order
    bool moved = false;
    for (auto loc : next_locations)
    {
      int next_h_val = instance.get_heuristic_distance(agent_num, loc);
      // if lowerbound on the time to reach goal smaller than limit, accept the move
      if (t + 1 + next_h_val < upperbound)
      {
        // get the agent that is in conflict and add it to the neighborhood
        auto [conflicting_agent_vertex, conflicting_agent_edge] =
            constraint_manager.get_blocking_agent(curr, loc, t + 1);

        // if there is an agent in collision, add it to the neighborhood
        if (conflicting_agent_vertex != -1)
        {
          chosen.insert(conflicting_agent_vertex);
        }

        if (conflicting_agent_edge != -1)
        {
          chosen.insert(conflicting_agent_edge);
        }

        curr = loc;
        moved = true;
        break;
      }
    }
    // test whether enough agents selected
    if (static_cast<int>(chosen.size()) >= neighborhood_size)
    {
      return true;
    }

    // if none of the next locations was valid, return
    if (!moved)
    {
      break;
    }
  }
  assertm(static_cast<int>(chosen.size()) <= neighborhood_size, "Enough agents selected, but false returned.");
  return false;
}

void LNS::destroy_intersection(Solution& sol) const
{
  last_destroy_strategy = DESTROY_TYPE::INTERSECTION;

  // find all intersection vertices
  std::vector<int> intersection_vertices_free;
  intersection_vertices_free.reserve(instance.get_num_free_cells());  // upperbound on number of intersection vertices

  for (int i = 0; i < instance.get_num_free_cells(); i++)
  {
    const auto& agents_counts = constraint_manager.get_agents_counts_free(i);
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
    int idx = dist(rnd_generator);
    int chosen = instance.free_location_to_location(intersection_vertices_free[idx]);
    openlist.push(chosen);
  }

  std::unordered_set<int> neighborhood;  // TODO try set
  std::vector<bool> known(instance.get_num_cells(), false);

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
    auto agents_counts = constraint_manager.get_agents_counts(current);
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
  assertm(static_cast<int>(neighborhood.size()) > 1, "Not enough agents selected for destroy.");

  // convert to vector and shuffle
  sol.destroyed_paths =
      std::vector(std::make_move_iterator(neighborhood.begin()), std::make_move_iterator(neighborhood.end()));
  // shuffle the paths
  std::shuffle(sol.destroyed_paths.begin(), sol.destroyed_paths.end(), rnd_generator);
  assertm(sol.destroyed_paths.size() > 0, "No paths were destroyed.");
  sol.feasible = false;
}

void LNS::get_intersection_agents(std::unordered_set<int>& neighborhood, int current) const
{
  // find maximal T
  int t_max = constraint_manager.get_last_constraint_start(current);

  // choose random time
  std::uniform_int_distribution<int> dist(0, t_max);
  int t = dist(rnd_generator);
  int delta = 0;
  while (static_cast<int>(neighborhood.size()) < settings.destroy_settings.size && t + delta <= t_max && t - delta >= 0)
  {
    // find the agent, that blocks the cell at time t + delta
    auto [blocking_agent_1, blocking_agent_2] = constraint_manager.get_blocking_agent(current, current, t + delta);
    // find the agent, that blocks the cell at time t - delta
    auto [blocking_agent_3, blocking_agent_4] = constraint_manager.get_blocking_agent(current, current, t - delta);

    // add the agents to the neighborhood
    std::array<int, 4> blocking_agents = {blocking_agent_1, blocking_agent_2, blocking_agent_3, blocking_agent_4};
    for (auto blocking_agent : blocking_agents)
    {
      if (blocking_agent >= 0 && static_cast<int>(neighborhood.size()) < settings.destroy_settings.size)
      {
        neighborhood.insert(blocking_agent);
      }
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
  double rand_value = dist(rnd_generator);

  // spin the roulette
  double cummulative_sum = 0.0;
  DESTROY_TYPE chosen_type = DESTROY_TYPE::ADAPTIVE;
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
      throw std::runtime_error(std::string("Unknown DESTROY_TYPE ") +
                               std::string(magic_enum::enum_name<DESTROY_TYPE>(chosen_type)) +
                               std::string(" encountered in adaptive destroy."));
    }
  }
  assertm(chosen_type != DESTROY_TYPE::ADAPTIVE && chosen_type != DESTROY_TYPE::RANDOM_CHOOSE,
          "Did not select any destroy strategy.");

  // if sum of weights too small, increase it
  if (sum_of_weights < 0.1)
  {
    for (double& destroy_weight : destroy_weights)
    {
      destroy_weight *= 10;
    }
  }
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
      throw std::runtime_error(std::string("Unknown DESTROY_TYPE ") +
                               std::string(magic_enum::enum_name<DESTROY_TYPE>(chosen_type)) +
                               std::string(" encountered in adaptive destroy."));
  }
}

void LNS::destroy_blocked(Solution& sol) const
{
  last_destroy_strategy = DESTROY_TYPE::BLOCKED;

  // if a random value is higher, than the threshold, perform randomwalk instead
  std::uniform_real_distribution<float> blocked_dist(0, 1);
  if (blocked_dist(rnd_generator) >= threshold_blocked)
  {
    // std::cout << "Running randomwalk. Threshold: " << threshold_blocked << std::endl;
    destroy_randomwalk(sol);
    return;
  }

  // find all agents whose end is blocked
  std::vector<int> blocked_agents;
  for (int i = 0; i < instance.get_num_of_agents(); i++)
  {
    const int loc = instance.get_goal_locations()[i];
    const int reach_time = sol.paths[i].back().interval.t_min;
    assertm(sol.paths[i].back().location == loc, "The path does not end in the goal location");
    // skip agents that dont have any delay
    if (reach_time == instance.get_heuristic_distance(i, instance.get_start_locations()[i]) - 1)
    {
      continue;
    }
    assertm(reach_time >= instance.get_heuristic_distance(i, instance.get_start_locations()[i]), "Invalid reach time");
    int from = loc;
    if (static_cast<int>(sol.paths[i].size()) > 1)
    {
      from = std::prev(sol.paths[i].end(), 2)->location;
    }
    auto [blocking_agent_1, blocking_agent_2] = constraint_manager.get_blocking_agent(from, loc, reach_time - 1);
    if (blocking_agent_1 != -1 || blocking_agent_2 != -1)
    {
      blocked_agents.push_back(i);
    }
  }

  // if no agent is blocked, use the randomwalk
  if (blocked_agents.empty())
  {
    destroy_randomwalk(sol);
    return;
  }
  // std::cout << "Number of blocked_agents: " << blocked_agents.size() << std::endl;

  // choose a random agent from blocked agents
  std::uniform_int_distribution<int> dist(0, blocked_agents.size() - 1);
  int chosen_agent = -1;
  {
    int idx = dist(rnd_generator);
    chosen_agent = blocked_agents[idx];
  }
  // std::cout << "Chosen agent: " << chosen_agent << std::endl;

  // add the agent to the neighborhood
  std::unordered_set<int> neighborhood = {chosen_agent};
  int idx = -1;
  sol.destroyed_paths = {chosen_agent};
  while (static_cast<int>(sol.destroyed_paths.size()) < settings.destroy_settings.size)
  {
    const int loc = instance.get_goal_locations()[chosen_agent];
    const int min_reach_time =
        instance.get_heuristic_distance(chosen_agent, instance.get_start_locations()[chosen_agent]);
    std::vector<int> blocking_agents_new = constraint_manager.get_blocking_agents(loc, min_reach_time - 1);

    // add all new blocking agents to the neighborhood
    for (auto it : blocking_agents_new)
    {
      if (neighborhood.find(it) == neighborhood.end())
      {
        neighborhood.insert(it);
        sol.destroyed_paths.push_back(it);
        // make sure that the neighborhood size is not exceeded
        // if (static_cast<int>(sol.destroyed_paths.size()) >= settings.destroy_settings.size)
        // {
        //   break;
        // }
      }
    }

    // update index
    idx++;
    if (idx >= static_cast<int>(sol.destroyed_paths.size()))
    {
      break;
    }

    // update chosen agent
    chosen_agent = sol.destroyed_paths[idx];
  }
  // shuffle randomly,  only when threshold is low (we need more variance)
  if (blocked_dist(rnd_generator) >= threshold_blocked)
  {
    // std::cout << "Shuffle. Threshold: " << threshold_blocked << std::endl;
    std::shuffle(sol.destroyed_paths.begin(), sol.destroyed_paths.end(), rnd_generator);
  }
  // dont exceed the neighborhood size
  if (settings.destroy_settings.size < static_cast<int>(sol.destroyed_paths.size()))
  {
    sol.destroyed_paths.resize(settings.destroy_settings.size);
  }
  assertm(settings.destroy_settings.size == 1 || static_cast<int>(sol.destroyed_paths.size()) > 1,
          "At least two paths should be destroyed.");
  sol.feasible = false;
}

void LNS::destroy_random_choose(Solution& sol) const
{
  std::uniform_int_distribution<int> dist(0, 2);
  int rand_value = dist(rnd_generator);
  // apply the destroy operator
  switch (rand_value)
  {
    case 0:
      destroy_random(sol);
      break;
    case 1:
      destroy_randomwalk(sol);
      break;
    case 2:
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
  for (int agent_id : sol.destroyed_paths)
  {
    // std::cout << "Removing path: " << agent_id << std::endl;
    constraint_manager.remove_trajectory(sol.paths[agent_id], agent_id);
    sol.paths[agent_id].clear();
    already_planned.erase(agent_id);
  }
  sol.feasible = true;

  // replan
  for (int i = 0; i < static_cast<int>(sol.destroyed_paths.size()); i++)
  {
    int path_idx = sol.destroyed_paths[i];
    // std::cout << "Replanning path: " << path_idx << std::endl;
    TimePointPath tp_path = planner->plan(path_idx, already_planned);
    if (tp_path.empty())
    {
      sol.feasible = false;
      return;
    }

    // get the visualization info
    if (!settings.use_psipp && settings.sipp_settings.info_type == INFO_type::visualisation)
    {
      sipp_info[i] = planner->get_iter_info();
    }

    constraint_manager.add_trajectory(tp_path, path_idx);
    sol.paths[path_idx] = tp_path;
    already_planned.insert(path_idx);
  }
}

void LNS::discard_solution(const Solution& sol, const Solution& prev_sol) const
{
  for (int agent_id : sol.destroyed_paths)
  {
    // remove the destroyed paths
    if (!sol.paths[agent_id].empty())
    {
      constraint_manager.remove_trajectory(sol.paths[agent_id], agent_id);
    }
  }

  // restore the old paths
  for (int agent_id : sol.destroyed_paths)
  {
    constraint_manager.add_trajectory(prev_sol.paths[agent_id], agent_id);
  }
}

void LNS::load_initial_solution(Solution& sol)
{
  assertm(found_initial_solution == false, "Solver already has an initial solution");
  sol.calculate_cost(instance);
  solution = sol;
  found_initial_solution = true;

  // build the constraints
  constraint_manager.build_sequential(sol.paths);

  // add to the log
  log.bsf_solution_cost.push_back(sol.sum_of_costs);
  log.bsf_makespan.push_back(sol.makespan);
  log.used_operator.push_back(DESTROY_TYPE::ADAPTIVE);
  log.iteration_time_wall.push_back(0);
  log.iteration_time_cpu.push_back(0);
}
