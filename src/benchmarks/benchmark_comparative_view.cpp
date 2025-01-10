//
// Created by Vyxs on 09/01/2025.
//

#include <chrono>
#include <benchmark/benchmark.h>
#include <entt/entt.hpp>
#include "vecs/ECS.h"
#include "vecs/View.h"

namespace config {
    struct BenchmarkConfig {
        static constexpr size_t entityCount = 10'000;
        static constexpr double minTime = 4.0;
        static constexpr size_t repetitions = 10;

        static constexpr const char* benchmarkName = "Entity Component Iteration";
        static constexpr const char* vecsLabel = "Vecs Framework";
        static constexpr const char* enttLabel = "EnTT Framework";
    };
}

struct alignas(16) Position {
    float x{}, y{}, z{};
    float padding{};
};

struct alignas(16) Velocity {
    float dx{}, dy{}, dz{};
    float padding{};
};

struct PerformanceMetrics {
    double elapsedTimeSeconds;      // Total elapsed time in seconds
    size_t totalIterations;         // Number of benchmark iterations
    size_t totalEntitiesProcessed;  // Total number of entities processed

    void report(benchmark::State& state) const {
        // Calculate per-second metrics
        const double entitiesPerSecond = static_cast<double>(totalEntitiesProcessed) / elapsedTimeSeconds;
        const double componentsPerSecond = entitiesPerSecond * 2; // Position + Velocity
        const double bytesPerSecond = componentsPerSecond * (sizeof(Position) + sizeof(Velocity));
        const double averageLatencyUs = (elapsedTimeSeconds * 1e6) / totalIterations;

        // Report counters
        state.counters["Entities/s"] =
            benchmark::Counter(entitiesPerSecond);
        state.counters["Components/s"] =
            benchmark::Counter(componentsPerSecond);
        state.counters["Bytes/s"] =
            benchmark::Counter(bytesPerSecond);
        state.counters["Avg Latency (us)"] =
            benchmark::Counter(averageLatencyUs);
        state.counters["Cache Lines Touched/s"] =
            benchmark::Counter(bytesPerSecond / 64.0); // Assuming 64-byte cache lines
    }
};

void setupVecs(vecs::ECS& ecs) {
    ecs.clear();
    for (size_t i = 0; i < config::BenchmarkConfig::entityCount; ++i) {
        auto entity = ecs.createEntity();
        ecs.emplaceComponent<Position>(entity,
            static_cast<float>(i),
            static_cast<float>(i * 2),
            static_cast<float>(i * 3)
        );
        ecs.emplaceComponent<Velocity>(entity, 1.0f, 2.0f, 3.0f);
    }
}

void setupEnTT(entt::registry& registry) {
    registry.clear();
    for (size_t i = 0; i < config::BenchmarkConfig::entityCount; ++i) {
        auto entity = registry.create();
        registry.emplace<Position>(entity,
            static_cast<float>(i),
            static_cast<float>(i * 2),
            static_cast<float>(i * 3)
        );
        registry.emplace<Velocity>(entity, 1.0f, 2.0f, 3.0f);
    }
}

static void BM_VecsIteration(benchmark::State& state) {
    vecs::ECS ecs;
    setupVecs(ecs);

    PerformanceMetrics metrics{};
    const auto startTime = std::chrono::high_resolution_clock::now();

    for (auto _ : state) {
        auto view = ecs.view<Position, Velocity>();
        float accumulator = 0.0f;

        view.each([&accumulator](const Position& pos, const Velocity& vel) {
            benchmark::DoNotOptimize(accumulator += pos.x * vel.dx + pos.y * vel.dy + pos.z * vel.dz);
        });

        benchmark::ClobberMemory();
        metrics.totalEntitiesProcessed += config::BenchmarkConfig::entityCount;
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    metrics.elapsedTimeSeconds = std::chrono::duration<double>(endTime - startTime).count();
    metrics.totalIterations = state.iterations();

    metrics.report(state);
}

static void BM_EnTTIteration(benchmark::State& state) {
    entt::registry registry;
    setupEnTT(registry);

    PerformanceMetrics metrics{};
    const auto startTime = std::chrono::high_resolution_clock::now();

    for (auto _ : state) {
        auto view = registry.view<const Position, const Velocity>();
        float accumulator = 0.0f;

        view.each([&accumulator](const Position& pos, const Velocity& vel) {
            benchmark::DoNotOptimize(accumulator += pos.x * vel.dx + pos.y * vel.dy + pos.z * vel.dz);
        });

        benchmark::ClobberMemory();
        metrics.totalEntitiesProcessed += config::BenchmarkConfig::entityCount;
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    metrics.elapsedTimeSeconds = std::chrono::duration<double>(endTime - startTime).count();
    metrics.totalIterations = state.iterations();

    metrics.report(state);
}

BENCHMARK(BM_VecsIteration)
    ->Name(config::BenchmarkConfig::vecsLabel)
    ->Unit(benchmark::kMicrosecond)
    ->MinTime(config::BenchmarkConfig::minTime)
    ->Repetitions(config::BenchmarkConfig::repetitions)
    ->DisplayAggregatesOnly(true)
    ->ComputeStatistics("cv", [](const std::vector<double>& v) -> double {
        double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
        double variance = (sq_sum - mean * mean * v.size()) / (v.size() - 1);
        return std::sqrt(variance) / mean * 100.0;
    });

BENCHMARK(BM_EnTTIteration)
    ->Name(config::BenchmarkConfig::enttLabel)
    ->Unit(benchmark::kMicrosecond)
    ->MinTime(config::BenchmarkConfig::minTime)
    ->Repetitions(config::BenchmarkConfig::repetitions)
    ->DisplayAggregatesOnly(true)
    ->ComputeStatistics("cv", [](const std::vector<double>& v) -> double {
        double mean = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
        double sq_sum = std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
        double variance = (sq_sum - mean * mean * v.size()) / (v.size() - 1);
        return std::sqrt(variance) / mean * 100.0;
    });