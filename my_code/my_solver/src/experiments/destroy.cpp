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

#define RUNS_PER_MAP 20
#define EXPERIMENT_NAME "sipp_pp"

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

#define NEIGHBORHOOD_SIZES \
  {                        \
    4, 8, 16, 32           \
  }

#define MAX_ITER 500
#define TIME_LIMIT 30

static void experiment_function(LNS& lns);

auto main(int argc, char** argv) -> int
{
  // assertm(false, "Experiments should be compiled in Release mode to have reliable results!");

  // prepare the algorithms
  std::vector<Algorithm> algorithms;
  std::vector<int>       neighborhood_sizes(NEIGHBORHOOD_SIZES);
  for (const auto& destroy : magic_enum::enum_values<DESTROY_TYPE>())
  {
    for (const auto& neighborhood_size : neighborhood_sizes)
    {
      LNS_settings lns_settings(MAX_ITER, TIME_LIMIT, {destroy, neighborhood_size},
                                {SIPP_implementation::SIPP_suboptimal, INFO_type::experiment, 1.0}, false);
      algorithms.emplace_back(SIPP_implementation::SIPP_suboptimal, lns_settings, std::make_pair("w", 1.0),
                              std::make_pair("destroy_type", magic_enum::enum_name<DESTROY_TYPE>(destroy)),
                              std::make_pair("neighborhood_size", neighborhood_size));
    }
  }
#ifdef SHOW_PROGRESS
  const bool show_progress = true;
#else
  const bool show_progress = false;
#endif

  // create the experiment
  Experiment experiment("destroy", experiment_function, MAPS, NUMS_AGENTS, algorithms, RUNS_PER_MAP, show_progress);

  // run the experiment
  experiment.run();
}

void experiment_function(LNS& lns)
{
  // load the initial solution
  std::string filename;  // TODO
  lns.solution.load(filename, lns.instance);
  lns.found_initial_solution = true;
  lns.solve();
}
