/*
 * Author: Jan Chleboun
 * Date: 06-04-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */
#include <magic_enum/magic_enum.hpp>

#include "Experiment.h"

#define RUNS_PER_MAP 1
#define EXPERIMENT_NAME "dummy"

#define SHOW_PROGRESS true

#define MAPS                   \
  {                            \
    "empty-8-8", "empty-32-32" \
  }

#define NUMS_AGENTS           \
  {                           \
    {16, 24, 32},             \
    {                         \
      300, 350, 400, 450, 500 \
    }                         \
  }

#define SUBOPTIMALITIES \
  {                     \
    1.0, 1.01           \
  }

static void experiment_function(LNS& lns);

auto main() -> int
{
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
  // create the experiment
  Experiment experiment("dummy", experiment_function, MAPS, NUMS_AGENTS, algorithms, RUNS_PER_MAP, true);

  // run the experiment
  experiment.run();
}

void experiment_function(LNS& lns)
{
  lns.find_initial_solution();
}
