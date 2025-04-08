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

#define RUNS_PER_MAP 104
#define EXPERIMENT_NAME "sipp_pp"

#define SHOW_PROGRESS

//#define MAPS                                                                                      \
//  {                                                                                               \
//    "empty-8-8", "empty-32-32", "random-32-32-20", "warehouse-10-20-10-2-1", "ost003d", "den520d" \
//  }
//
//#define NUMS_AGENTS                                                                                                          \
//  {                                                                                                                          \
//    {16, 24, 32}, {300, 350, 400, 450, 500}, {50, 100, 150, 200, 250}, {150, 200, 250, 300, 350}, {100, 200, 300, 400, 500}, \
//    {                                                                                                                        \
//      100, 200, 300, 400, 500                                                                                                \
//    }                                                                                                                        \
//  }
//
#define MAPS    \
  {             \
    "empty-8-8" \
  }

#define NUMS_AGENTS \
  {                 \
    {               \
      16            \
    }               \
  }

#define SUBOPTIMALITIES        \
  {                            \
    1.0, 1.01, 1.02, 1.05, 1.1 \
  }

static void experiment_function(LNS& lns);

auto main(int argc, char** argv) -> int
{
  // assertm(false, "Experiments should be compiled in Release mode to have reliable results!");

  // prepare the algorithms
  std::vector<Algorithm> algorithms;

  for (const auto& algo_type : magic_enum::enum_values<SIPP_implementation>())
  {
    if (algo_type == SIPP_implementation::SIPP_suboptimal)
    {
      for (double w : SUBOPTIMALITIES)
      {
        LNS_settings lns_settings(1, 1, {DESTROY_TYPE::RANDOM, 10}, {algo_type, INFO_type::experiment, w});
        algorithms.emplace_back(algo_type, lns_settings, std::make_pair("w", w));
      }
    }
    else
    {
      LNS_settings lns_settings(1, 1, {DESTROY_TYPE::RANDOM, 10}, {algo_type, INFO_type::experiment, 1.0});
      algorithms.emplace_back(algo_type, lns_settings);
    }
  }
#ifdef SHOW_PROGRESS
  const bool show_progress = true;
#else
  const bool show_progress = false;
#endif

  // create the experiment
  Experiment experiment("sipp_pp", experiment_function, MAPS, NUMS_AGENTS, algorithms, RUNS_PER_MAP, show_progress);

  // run the experiment
  experiment.run();
}

void experiment_function(LNS& lns)
{
  lns.find_initial_solution();
}
