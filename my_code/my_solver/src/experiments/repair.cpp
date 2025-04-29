/*
 * Author: Jan Chleboun
 * Date: 04-03-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <nlohmann/json.hpp>
#include <random>

#include "Experiment.h"
#include "Instance.h"
#include "LNS.h"
#include "indicators/indicators.hpp"
#include "magic_enum/magic_enum.hpp"
#include "utils.h"

#define RUNS_PER_MAP 40
#define EXPERIMENT_NAME "repair"

#define SHOW_PROGRESS

#define MAPS                                                                         \
  {                                                                                  \
    "empty-32-32", "random-32-32-20", "warehouse-10-20-10-2-1", "ost003d", "den520d" \
  }

#define NUMS_AGENTS             \
  {                             \
    {200}, {100}, {150}, {400}, \
    {                           \
      500                       \
    }                           \
  }

#define SUBOPTIMALITIES  \
  {                      \
    1.0, 1.02, 1.05, 1.1 \
  }

#define P_VALUES \
  {              \
    1, 5         \
  }

#define NEIGHBORHOOD_SIZE 8

#define MAX_ITER 100000
#define TIME_LIMIT 20

static void experiment_function(LNS& lns);

auto main(int argc, char** argv) -> int
{
  // assertm(false, "Experiments should be compiled in Release mode to have reliable results!");

  // prepare the algorithms
  std::vector<Algorithm> algorithms;
  std::vector<double>    suboptimalities(SUBOPTIMALITIES);

  // optimal version
  {
    LNS_settings lns_settings(MAX_ITER, TIME_LIMIT, {DESTROY_TYPE::RANDOMWALK, NEIGHBORHOOD_SIZE},
                              {SIPP_implementation::SIPP_mine, INFO_type::experiment, 1.0});
    algorithms.emplace_back(SIPP_implementation::SIPP_mine, lns_settings);
  }
  {
    LNS_settings lns_settings(MAX_ITER, TIME_LIMIT, {DESTROY_TYPE::RANDOMWALK, NEIGHBORHOOD_SIZE},
                              {SIPP_implementation::SIPP_mine_ap, INFO_type::experiment, 1.0});
    algorithms.emplace_back(SIPP_implementation::SIPP_mine_ap, lns_settings);
  }
  for (int p : P_VALUES)
  {
    for (auto w : suboptimalities)
    {
      LNS_settings lns_settings(MAX_ITER, TIME_LIMIT, {DESTROY_TYPE::RANDOMWALK, NEIGHBORHOOD_SIZE},
                                {SIPP_implementation::SIPP_suboptimal_ap, INFO_type::experiment, w, p});
      algorithms.emplace_back(SIPP_implementation::SIPP_suboptimal_ap, lns_settings, std::make_pair("w", w), std::make_pair("p", p));
    }
  }
#ifdef SHOW_PROGRESS
  const bool show_progress = true;
#else
  const bool show_progress = false;
#endif

  // create the experiment
  Experiment experiment("repair", experiment_function, MAPS, NUMS_AGENTS, algorithms, RUNS_PER_MAP, show_progress);

  // run the experiment
  experiment.run();
}

void experiment_function(LNS& lns)
{
  LNS_settings lns_settings_orig = lns.settings;
  // find initial solution by the same algorithm for all experiments
  lns.settings = LNS_settings(0, TIME_LIMIT, {DESTROY_TYPE::RANDOM_CHOOSE, NEIGHBORHOOD_SIZE},
                              {SIPP_implementation::SIPP_suboptimal, INFO_type::experiment, 1.0}, true);

  lns.solve();
  if (lns.solution.feasible)
  {
    // switch settings to the original ones and run the LNS
    lns.settings = lns_settings_orig;
    lns.solve();
  }
}
