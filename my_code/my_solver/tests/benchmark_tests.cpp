#include <benchmark/benchmark.h>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

#include "Instance.h"

// Function to get the directory of the running executable
std::string get_base_path()
{
  return std::filesystem::canonical("/proc/self/exe").parent_path().parent_path().parent_path().string();  // Linux only
}

// Benchmark for Sequential A*
static void BM_AStar_Sequential(benchmark::State& state)
{
  // setup the instance
  std::string base_path = get_base_path();  // path to my_solver
  for (auto _ : state)
  {
    int             num_agents = state.range(0);
    std::unique_ptr instance =
        std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", num_agents);
    instance->calculate_optimal_paths();
  }
}
BENCHMARK(BM_AStar_Sequential)->Arg(5)->Arg(10)->Arg(20)->Arg(50)->Arg(100)->Arg(200)->Arg(500);

// Benchmark for Parallel A*
static void BM_AStar_Parallel(benchmark::State& state)
{
  // setup the instance
  std::string base_path = get_base_path();
  for (auto _ : state)
  {
    int             num_agents = state.range(0);
    std::unique_ptr instance =
        std::make_unique<Instance>(base_path + "/tests/test_maps/den520d.map", base_path + "/tests/test_scen/den520d-random-0.scen", num_agents);
    instance->calculate_optimal_paths_parallel();
  }
}
BENCHMARK(BM_AStar_Parallel)->Arg(5)->Arg(10)->Arg(20)->Arg(50)->Arg(100)->Arg(200)->Arg(500);

BENCHMARK_MAIN();
