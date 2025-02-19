#include <benchmark/benchmark.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "Instance.h"
#include "LNS.h"
#include "test_utils.h"


// Benchmark for SIPP using prioritized planning on map den520d
static void BM_PP_den520(benchmark::State& state)
{
  // setup the instance
  std::string     base_path  = get_base_path_tests();  // path to my_solver
  int             num_agents = state.range(0);
  std::unique_ptr instance   = std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map",
                                                        base_path + "/tests/test_scen/den520d-random-0.scen", num_agents);
  for (auto _ : state)
  {
    // create LNS
    LNS lns(*instance);

    // plan path by Prioritized Planning
    Solution sol = lns.PrioritizedPlanning();
  }
}
BENCHMARK(BM_PP_den520)->Arg(1)->Arg(5)->Arg(10)->Arg(50)->Arg(100)->Arg(200)->Arg(500);

BENCHMARK_MAIN();
