/*
 * Author: Jan Chleboun
 * Date: 17-02-2025
 * Email: chlebja3@fel.cvut.cz
 * Description:
 */

#include <benchmark/benchmark.h>

#include <iostream>

#include "test_utils.h"
#include "utils.h"

Path timepointpath_to_path_resize(const TimePointPath& tp_path)
{
  Path path;
  for (const TimePoint& tp : tp_path)
  {
    // calculate how many copies to insert
    int N = tp.interval.t_max - tp.interval.t_min + 1;
    // insert N copies to the end of the vector
    path.resize(path.size() + N, tp.position);
  }
  return path;
}

Path timepointpath_to_path_insert(const TimePointPath& tp_path)
{
  Path path;
  for (const TimePoint& tp : tp_path)
  {
    // calculate how many copies to insert
    int N = tp.interval.t_max - tp.interval.t_min + 1;
    // insert N copies to the end of the vector
    path.insert(path.end(), N, tp.position);
  }
  return path;
}

Path timepointpath_to_path_pushback(const TimePointPath& tp_path)
{
  Path path;
  int  N = 0;
  // calculate the vector size
  for (const TimePoint& tp : tp_path)
  {
    // calculate how many copies to insert
    N = tp.interval.t_max - tp.interval.t_min + 1;
    // insert N copies to the end of the vector
    path.reserve(path.size() + N);
    for (int i = 0; i < N; i++)
    {
      path.push_back(tp.position);
    }
  }
  return path;
}

Path timepointpath_to_path_emplaceback(const TimePointPath& tp_path)
{
  Path path;
  int  N = 0;
  // calculate the vector size
  for (const TimePoint& tp : tp_path)
  {
    // calculate how many copies to insert
    N = tp.interval.t_max - tp.interval.t_min + 1;
    // insert N copies to the end of the vector
    path.reserve(path.size() + N);
    for (int i = 0; i < N; i++)
    {
      path.emplace_back(tp.position.x, tp.position.y);
    }
  }
  return path;
}

// Helper function for benchmarking
template <typename Func>
static void BenchmarkFunction(benchmark::State& state, Func func)
{
  TimePointPath tp_path = generate_random_timepointpath(state.range(0));  // Ensure same random input for all tests
  for (auto _ : state)
  {
    Path p = func(tp_path);
    assertm(p.size() == state.range(0), "The converted path has wrong size");
  }
}

// benchmark push_back
static void BM_TPPathtoPathPushback(benchmark::State& state)
{
  BenchmarkFunction(state, timepointpath_to_path_pushback);
}
BENCHMARK(BM_TPPathtoPathPushback)->Range(100, 100000)->RangeMultiplier(10);

// benchmark emplace_back
static void BM_TPPathtoPathEmplaceback(benchmark::State& state)
{
  BenchmarkFunction(state, timepointpath_to_path_emplaceback);
}
BENCHMARK(BM_TPPathtoPathEmplaceback)->Range(100, 100000)->RangeMultiplier(10);

// benchmark resize
static void BM_TPPathtoPathResize(benchmark::State& state)
{
  BenchmarkFunction(state, timepointpath_to_path_resize);
}
BENCHMARK(BM_TPPathtoPathResize)->Range(100, 100000)->RangeMultiplier(10);

// benchmark insert
static void BM_TPPathtoPathInsert(benchmark::State& state)
{
  BenchmarkFunction(state, timepointpath_to_path_insert);
}
BENCHMARK(BM_TPPathtoPathInsert)->Range(100, 100000)->RangeMultiplier(10);

BENCHMARK_MAIN();
