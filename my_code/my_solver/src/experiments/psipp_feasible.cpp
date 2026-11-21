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

#define RUNS_PER_MAP 100
#define EXPERIMENT_NAME "psipp_feasible"

#define SHOW_PROGRESS

#define MAPS                                                              \
  {                                                                       \
    "empty-32-32", "random-32-32-20", "warehouse-10-20-10-2-1", "ost003d" \
  }

#define NUMS_AGENTS                                                                                                   \
  {                                                                                                                   \
    {350, 400, 450, 500}, {200, 225, 250, 275, 300}, {350, 400, 450, 500, 550, 600, 650}, {600, 700, 800, 900, 1000}, \
  }

#define TIME_LIMITS \
  {                 \
    20, 20, 20, 20  \
  }

static void experiment_function(LNS& lns);

auto main(int argc, char** argv) -> int
{
  // assertm(false, "Experiments should be compiled in Release mode to have reliable results!");

  // prepare the algorithms
  std::vector<Algorithm> algorithms;

  // SIPP mine ap
  {
    SIPP_implementation algo_type = SIPP_implementation::SIPP_mine_ap;
    LNS_settings lns_settings(0, 20, {DESTROY_TYPE::RANDOM, 10}, {algo_type, INFO_type::experiment, 1.0},
                              {PSIPP_implementation::PSIPP_mine_ap, INFO_type::experiment, 1.0}, true, false);
    algorithms.emplace_back(algo_type, lns_settings);
  }

  // PSIPP mine max pushes 1
  {
    SIPP_implementation algo_type = SIPP_implementation::SIPP_mine_ap;
    LNS_settings lns_settings(0, 20, {DESTROY_TYPE::RANDOM, 10}, {algo_type, INFO_type::experiment, 1.0},
                              {PSIPP_implementation::PSIPP_mine_ap, INFO_type::experiment, 1.0, 1, 1}, true, true);
    algorithms.emplace_back(PSIPP_implementation::PSIPP_mine_ap, lns_settings, std::make_pair("max_push", 1));
  }

  // PSIPP mine max pushes 5
  {
    SIPP_implementation algo_type = SIPP_implementation::SIPP_mine_ap;
    LNS_settings lns_settings(0, 20, {DESTROY_TYPE::RANDOM, 10}, {algo_type, INFO_type::experiment, 1.0},
                              {PSIPP_implementation::PSIPP_mine_ap, INFO_type::experiment, 1.0, 1, 5}, true, true);
    algorithms.emplace_back(PSIPP_implementation::PSIPP_mine_ap, lns_settings, std::make_pair("max_push", 5));
  }

  // PSIPP suboptimal
  {
    SIPP_implementation algo_type = SIPP_implementation::SIPP_mine_ap;
    LNS_settings lns_settings(0, 20, {DESTROY_TYPE::RANDOM, 10}, {algo_type, INFO_type::experiment, 1.0},
                              {PSIPP_implementation::PSIPP_suboptimal_ap, INFO_type::experiment, 1.0, 1, 5}, true,
                              true);
    algorithms.emplace_back(PSIPP_implementation::PSIPP_suboptimal_ap, lns_settings, std::make_pair("max_push", 5));
  }

#ifdef SHOW_PROGRESS
  const bool show_progress = true;
#else
  const bool show_progress = false;
#endif

  // create the experiment
  Experiment experiment(EXPERIMENT_NAME, experiment_function, MAPS, NUMS_AGENTS, TIME_LIMITS, algorithms, RUNS_PER_MAP,
                        show_progress);

  // run the experiment
  experiment.run();
}

void experiment_function(LNS& lns)
{
  lns.solve();
}
