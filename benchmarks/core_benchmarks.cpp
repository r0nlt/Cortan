#include <benchmark/benchmark.h>
#include <cortan/core/event_system.hpp>

static void BM_EventCreation(benchmark::State& state) {
    for (auto _ : state) {
        // TODO: Benchmark event creation
        benchmark::DoNotOptimize(state.iterations());
    }
}
BENCHMARK(BM_EventCreation);

BENCHMARK_MAIN();
