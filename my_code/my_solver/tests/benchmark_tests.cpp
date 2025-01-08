#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>

// Example: Benchmark sorting a vector
static void BM_SortVector(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> v(state.range(0));
        std::generate(v.begin(), v.end(), rand);
        std::sort(v.begin(), v.end());
    }
}

// Run with different vector sizes
BENCHMARK(BM_SortVector)->Range(8, 8<<10);

BENCHMARK_MAIN();
