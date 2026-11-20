// Headless benchmark of the initial-solution phase (prioritized planning with restarts).
// Prints one JSON line.
#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <string>

#include "Instance.h"
#include "LNS.h"

auto main(int argc, char** argv) -> int
{
  if (argc != 11)
  {
    std::cerr << "usage: psipp_init_bench <map> <scen> <k> <planner: sipp|psipp> <max_pushes> <seed> <time_limit> "
                 "<push_cost: inf|number> <closed_per_count: 0|1> <sipp_first: 0|1>\n";
    return 2;
  }
  const std::string map = argv[1];
  const std::string scen = argv[2];
  const int k = std::stoi(argv[3]);
  const std::string planner = argv[4];
  const int max_pushes = std::stoi(argv[5]);
  const int seed = std::stoi(argv[6]);
  const double time_limit = std::stod(argv[7]);
  const std::string push_cost_arg = argv[8];
  const double push_cost = push_cost_arg == "inf" ? std::numeric_limits<double>::infinity() : std::stod(push_cost_arg);
  const bool closed_per_count = std::stoi(argv[9]) != 0;
  const bool sipp_first = std::stoi(argv[10]) != 0;

  const std::string variant = planner == "sipp" ? "sipp"
                                                 : "psipp_c" + push_cost_arg + "_cl" + (closed_per_count ? "1" : "0") +
                                                       "_sf" + (sipp_first ? "1" : "0");

  Instance instance(map, scen, k);
  std::mt19937 rng(seed);

  const bool use_psipp = planner != "sipp";
  SIPP_settings sipp_settings(SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0);
  PSIPP_settings psipp_settings(PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, max_pushes, push_cost,
                                closed_per_count);
  Destroy_settings destroy_settings(DESTROY_TYPE::RANDOM, std::min(10, k));
  LNS_settings settings(0, time_limit, destroy_settings, sipp_settings, psipp_settings, true, use_psipp, sipp_first);

  LNS lns(instance, rng, nullptr, settings);

  const auto t0 = std::chrono::steady_clock::now();
  int attempts = 0;
  bool ok = false;
  while (!ok)
  {
    const double el = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    if (el >= time_limit) break;
    ok = lns.find_initial_solution();
    attempts++;
    if (!ok)
    {
      lns.get_constraint_manager().reset();
      lns.already_planned.clear();
    }
  }
  const double el = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  // One planning attempt is not interrupted by the clock, so a solution found after the limit is a failure.
  if (el > time_limit) ok = false;
  const bool valid = ok && lns.solution.is_valid(instance);
  std::cout << "{\"map\":\"" << map.substr(map.find_last_of('/') + 1) << "\",\"scen\":\""
            << scen.substr(scen.find_last_of('/') + 1) << "\",\"k\":" << k << ",\"planner\":\"" << planner
            << "\",\"variant\":\"" << variant << "\",\"max_pushes\":" << max_pushes << ",\"push_cost\":\"" << push_cost_arg
            << "\",\"closed_per_count\":" << (closed_per_count ? 1 : 0) << ",\"sipp_first\":" << (sipp_first ? 1 : 0)
            << ",\"time_limit\":" << time_limit << ",\"seed\":" << seed << ",\"success\":" << (ok ? 1 : 0) << ",\"valid\":" << (valid ? 1 : 0)
            << ",\"attempts\":" << attempts << ",\"time\":" << el
            << ",\"soc\":" << (ok ? lns.solution.sum_of_costs : -1)
            << ",\"delays\":" << (ok ? lns.solution.sum_of_delays : -1)
            << ",\"fail_no_path\":" << lns.log.pp_fail_no_path << ",\"fail_cycle\":" << lns.log.pp_fail_cycle
            << ",\"pushed_replanned\":" << lns.log.pp_pushed_replanned << "}" << std::endl;
  return 0;
}
