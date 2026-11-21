// Headless anytime benchmark of the full LNS solver (initial solution by PP with restarts, then LNS).
// Uses the final configuration of the thesis (BLOCKED destroy, neighborhood 8, SIPP_suboptimal_ap w=1 p=5); the
// initial solution is planned either by SIPP or by PSIPP. The psipp_rp* variants also repair by PSIPP (destroyed agents
// may push agents outside the neighborhood): psipp_rp tries SIPP first for every destroyed agent, psipp_rpd uses PSIPP
// directly, both with infinite push cost; psipp_rpdc<cost> uses PSIPP directly with a finite push cost.
// Saves the final paths in the MAPF-LNS2 format and prints
// one JSON line with the anytime curve (time from the start of the preprocessing, best sum of costs).
#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <string>

#include "Instance.h"
#include "LNS.h"

auto main(int argc, char** argv) -> int
{
  if (argc != 8)
  {
    std::cerr << "usage: anytime_bench <map> <scen> <k> <init: sipp|psipp|psipp_rp|psipp_rpd|psipp_rpdc<cost>> <seed> <time_limit> <paths_out>\n";
    return 2;
  }
  const std::string map = argv[1];
  const std::string scen = argv[2];
  const int k = std::stoi(argv[3]);
  const std::string init = argv[4];
  const int seed = std::stoi(argv[5]);
  const double time_limit = std::stod(argv[6]);
  const std::string paths_out = argv[7];
  const std::string rpdc = "psipp_rpdc";
  const bool finite_cost = init.rfind(rpdc, 0) == 0;
  if (init != "sipp" && init != "psipp" && init != "psipp_rp" && init != "psipp_rpd" && !finite_cost)
  {
    std::cerr << "unknown init: " << init << "\n";
    return 2;
  }
  const bool use_psipp = init != "sipp";
  const bool psipp_repair = init.rfind("psipp_rp", 0) == 0;
  const bool repair_sipp_first = init == "psipp_rp";
  const double repair_push_cost =
      finite_cost ? std::stod(init.substr(rpdc.size())) : std::numeric_limits<double>::infinity();

  Instance instance(map, scen, k);
  std::mt19937 rng(seed);

  // thesis configuration; the PSIPP variant adds PSIPP (SIPP first, max 5 pushes) and longest-first first attempt
  SIPP_settings sipp_settings(SIPP_implementation::SIPP_suboptimal_ap, INFO_type::experiment, 1.0, 5);
  PSIPP_settings psipp_settings(PSIPP_implementation::PSIPP_mine_ap, INFO_type::experiment, 1.0, 1, 5,
                                std::numeric_limits<double>::infinity(), false);
  Destroy_settings destroy_settings(DESTROY_TYPE::BLOCKED, std::min(8, k));
  LNS_settings settings(std::numeric_limits<int>::max(), time_limit, destroy_settings, sipp_settings, psipp_settings,
                        true, use_psipp, true, use_psipp ? PP_ORDER::LONGEST_FIRST : PP_ORDER::RANDOM, true,
                        psipp_repair, repair_sipp_first, repair_push_cost);

  // the time includes the preprocessing done in the LNS constructor, as in MAPF-LNS2
  const auto t0 = std::chrono::steady_clock::now();
  const auto elapsed = [&t0]() { return std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count(); };
  LNS lns(instance, rng, nullptr, settings);
  const double t_pre = elapsed();

  // initial solution: PP with restarts, timed here so that the resets between the attempts are included
  int init_attempts = 0;
  while (!lns.found_initial_solution && elapsed() < time_limit)
  {
    lns.found_initial_solution = lns.find_initial_solution();
    init_attempts++;
    if (!lns.found_initial_solution)
    {
      lns.get_constraint_manager().reset();
      lns.already_planned.clear();
    }
  }
  const double t_first = lns.found_initial_solution ? elapsed() : -1;

  // LNS improvement for the rest of the time limit (solve() skips the initial solution, it is already found)
  if (lns.found_initial_solution && t_first < time_limit)
  {
    settings.time_limit = time_limit - t_first;
    lns.solve();
  }
  const double t_total = elapsed();

  // anytime curve: the initial solution and every improvement, times of the LNS iterations accumulated
  const Logger& log = lns.log;
  double t = t_first;
  int best = -1;
  std::string curve;
  for (size_t i = init_attempts; i < log.bsf_solution_cost.size(); i++)
  {
    if (i > static_cast<size_t>(init_attempts))
    {
      t += log.iteration_time_wall[i];
    }
    const int cost = log.bsf_solution_cost[i];
    if (best >= 0 && cost >= best)
    {
      continue;
    }
    best = cost;
    curve += std::string(curve.empty() ? "" : ",") + "[" + std::to_string(t) + "," + std::to_string(cost) + "]";
  }
  // a first solution found after the limit (one PP attempt is not interrupted) is a failure
  const bool ok = lns.found_initial_solution && t_first <= time_limit;
  const bool valid = ok && lns.solution.is_valid(instance);
  if (ok)
  {
    lns.solution.save(paths_out, instance);
  }
  const int iterations = static_cast<int>(log.bsf_solution_cost.size()) - init_attempts;
  std::cout << "{\"solver\":\"ours_" << init << "\",\"map\":\"" << map.substr(map.find_last_of('/') + 1)
            << "\",\"scen\":\"" << scen.substr(scen.find_last_of('/') + 1) << "\",\"k\":" << k << ",\"seed\":" << seed
            << ",\"time_limit\":" << time_limit << ",\"success\":" << (ok ? 1 : 0) << ",\"valid\":" << (valid ? 1 : 0)
            << ",\"t_pre\":" << t_pre << ",\"t_first\":" << t_first << ",\"t_logged\":" << t << ",\"t_total\":" << t_total
            << ",\"init_attempts\":" << init_attempts << ",\"iterations\":" << iterations
            << ",\"repair_pushes\":" << log.repair_pushes << ",\"repair_pushed_agents\":" << log.repair_pushed_agents
            << ",\"repair_fail_no_path\":" << log.repair_fail_no_path << ",\"repair_fail_cycle\":" << log.repair_fail_cycle
            << ",\"soc\":" << (ok ? lns.solution.sum_of_costs : -1)
            << ",\"delays\":" << (ok ? lns.solution.sum_of_delays : -1) << ",\"curve\":[" << curve << "]}" << std::endl;
  return 0;
}
