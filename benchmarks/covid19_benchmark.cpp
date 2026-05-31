#include "covid19_helper.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdint>
#include <future>
#include <string>
#include <thread>
#include <vector>

namespace
{
    constexpr auto simulation_count = 100U;

    constexpr auto nnj_population = 589'755U;
    constexpr auto ndk_population = 5'822'763U;

    auto estimate_average_peak_hospitalized_single_core(
        const std::string &region_name,
        const std::uint32_t population_size,
        const std::uint32_t simulations,
        const std::uint32_t seed) -> double
    {
        auto sum = 0.0;

        for (std::uint32_t i = 0; i < simulations; ++i)
        {
            const auto result = covid19_example::estimate_peak_hospitalized(
                region_name,
                population_size,
                seed + i);

            sum += static_cast<double>(result.peak_hospitalized);
        }

        return sum / static_cast<double>(simulations);
    }

    auto estimate_average_peak_hospitalized_multi_core(
        const std::string &region_name,
        const std::uint32_t population_size,
        const std::uint32_t simulations,
        const std::uint32_t seed) -> double
    {
        const auto hardware_threads = std::thread::hardware_concurrency();

        const auto worker_count = std::max(
            1U,
            std::min(
                simulations,
                hardware_threads == 0U ? 1U : hardware_threads));

        auto futures = std::vector<std::future<double>>{};
        futures.reserve(worker_count);

        for (std::uint32_t worker = 0; worker < worker_count; ++worker)
        {
            futures.push_back(std::async(
                std::launch::async,
                [=, &region_name]
                {
                    auto partial_sum = 0.0;

                    for (std::uint32_t i = worker; i < simulations; i += worker_count)
                    {
                        const auto result = covid19_example::estimate_peak_hospitalized(
                            region_name,
                            population_size,
                            seed + i);

                        partial_sum += static_cast<double>(result.peak_hospitalized);
                    }

                    return partial_sum;
                }));
        }

        auto sum = 0.0;

        for (auto &future : futures)
        {
            sum += future.get();
        }

        return sum / static_cast<double>(simulations);
    }

    void benchmark_nnj_single_core(benchmark::State &state)
    {
        for (auto _ : state)
        {
            auto average_peak = estimate_average_peak_hospitalized_single_core(
                "NNJ",
                nnj_population,
                simulation_count,
                42U);

            benchmark::DoNotOptimize(average_peak);
        }
    }

    void benchmark_nnj_multi_core(benchmark::State &state)
    {
        for (auto _ : state)
        {
            auto average_peak = estimate_average_peak_hospitalized_multi_core(
                "NNJ",
                nnj_population,
                simulation_count,
                42U);

            benchmark::DoNotOptimize(average_peak);
        }
    }

    void benchmark_ndk_single_core(benchmark::State &state)
    {
        for (auto _ : state)
        {
            auto average_peak = estimate_average_peak_hospitalized_single_core(
                "NDK",
                ndk_population,
                simulation_count,
                10'000U);

            benchmark::DoNotOptimize(average_peak);
        }
    }

    void benchmark_ndk_multi_core(benchmark::State &state)
    {
        for (auto _ : state)
        {
            auto average_peak = estimate_average_peak_hospitalized_multi_core(
                "NDK",
                ndk_population,
                simulation_count,
                10'000U);

            benchmark::DoNotOptimize(average_peak);
        }
    }
}

BENCHMARK(benchmark_nnj_single_core)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

BENCHMARK(benchmark_nnj_multi_core)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

BENCHMARK(benchmark_ndk_single_core)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

BENCHMARK(benchmark_ndk_multi_core)
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

BENCHMARK_MAIN();