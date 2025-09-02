#include <benchmark/benchmark.h>
#include <memory>

// Memory allocation benchmarks
static void BM_MemoryAllocation(benchmark::State& state) {
    for (auto _ : state) {
        auto ptr = std::make_unique<int>(42);
        benchmark::DoNotOptimize(ptr);
    }
}
BENCHMARK(BM_MemoryAllocation);

// Memory pool simulation benchmark
static void BM_MemoryPoolSimulation(benchmark::State& state) {
    for (auto _ : state) {
        // TODO: Implement memory pool benchmarking when memory pool is implemented
        volatile int dummy = 0;
        benchmark::DoNotOptimize(dummy);
    }
}
BENCHMARK(BM_MemoryPoolSimulation);

// Main is in core_benchmarks.cpp
