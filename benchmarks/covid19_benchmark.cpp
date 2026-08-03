#include "covid19_helper.hpp"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace
{

    /*
     * R10: Use a smaller fixed batch for performance measurements.
     *
     * Requirement 9 is demonstrated separately using 100 simulations.
     * A batch of 10 simulations keeps the expensive NDK single-core
     * benchmark practical while still allowing a direct comparison
     * between single-core and multi-core execution.
     */
    constexpr auto benchmark_simulation_count =
        std::size_t{10};

    // R7/R9/R10: Population sizes used for the required NNJ
    // and NDK Covid-19 experiments.
    constexpr auto nnj_population =
        std::uint32_t{589'755};

    constexpr auto ndk_population =
        std::uint32_t{5'822'763};

    /*
     * R7/R10: Runs a batch of independent Covid-19 simulations
     * sequentially and returns the average hospitalisation peak.
     */
    auto estimate_average_peak_hospitalized_single_core(
        const std::string &region_name,
        const std::uint32_t population_size,
        const std::size_t simulation_count,
        const std::uint32_t first_seed) -> double
    {
        // R10: An empty batch has no meaningful average.
        if (simulation_count == 0)
        {
            throw std::invalid_argument{
                "simulation_count must be greater than zero"};
        }

        auto total_peak = std::uint64_t{0};

        // R10: Run each simulation sequentially.
        for (std::size_t simulation_index = 0;
             simulation_index < simulation_count;
             ++simulation_index)
        {
            // R10: Use a different deterministic seed for each
            // independent simulation.
            const auto seed =
                first_seed +
                static_cast<std::uint32_t>(
                    simulation_index);

            /*
             * R7/R10: Estimate the peak hospitalisation using
             * the observer-based Covid-19 helper.
             *
             * The observer records the peak during simulation
             * without storing the complete trajectory.
             */
            const auto result =
                covid19_example::estimate_peak_hospitalized(
                    region_name,
                    population_size,
                    seed);

            total_peak += result.peak_hospitalized;
        }

        return static_cast<double>(total_peak) /
               static_cast<double>(simulation_count);
    }

    /*
     * R7/R9/R10: Runs a batch of independent Covid-19 simulations
     * concurrently and returns the average hospitalisation peak.
     */
    auto estimate_average_peak_hospitalized_multi_core(
        const std::string &region_name,
        const std::uint32_t population_size,
        const std::size_t simulation_count,
        const std::uint32_t first_seed) -> double
    {
        // R10: An empty batch has no meaningful average.
        if (simulation_count == 0)
        {
            throw std::invalid_argument{
                "simulation_count must be greater than zero"};
        }

        const auto hardware_threads =
            std::thread::hardware_concurrency();

        // R9: hardware_concurrency() may return zero when the
        // number of hardware threads cannot be determined.
        const auto available_workers =
            hardware_threads == 0
                ? std::size_t{1}
                : static_cast<std::size_t>(
                      hardware_threads);

        /*
         * R9: Bound this batch to no more workers than available
         * hardware threads or simulations.
         *
         * This reduces the risk of local over-subscription without
         * claiming knowledge of all other work running on the system.
         */
        const auto worker_count =
            std::min(
                simulation_count,
                available_workers);

        /*
         * R9: Each asynchronous worker returns its own partial sum.
         *
         * No worker modifies a shared result variable, so no mutex
         * or atomic result counter is required.
         */
        auto tasks =
            std::vector<std::future<std::uint64_t>>{};

        tasks.reserve(worker_count);

        // R9: Start a bounded number of asynchronous worker tasks.
        for (std::size_t worker_index = 0;
             worker_index < worker_count;
             ++worker_index)
        {
            tasks.emplace_back(
                std::async(
                    std::launch::async,

                    /*
                     * R9: Capture all configuration values by value.
                     * Each task therefore owns the values it needs.
                     */
                    [region_name,
                     population_size,
                     simulation_count,
                     first_seed,
                     worker_index,
                     worker_count]()
                    {
                        // R9: This partial sum belongs exclusively
                        // to the current worker.
                        auto partial_sum =
                            std::uint64_t{0};

                        /*
                         * R9: Divide simulation indices between workers
                         * using a strided partition.
                         *
                         * Every simulation index is handled by exactly
                         * one worker, so no shared work counter is needed.
                         */
                        for (std::size_t simulation_index =
                                 worker_index;
                             simulation_index <
                             simulation_count;
                             simulation_index +=
                             worker_count)
                        {
                            // R9/R10: Every simulation receives
                            // a distinct deterministic seed.
                            const auto seed =
                                first_seed +
                                static_cast<std::uint32_t>(
                                    simulation_index);

                            /*
                             * R7/R9/R10: Each worker runs independent
                             * observer-based simulations using separate
                             * simulation state and random generators.
                             */
                            const auto result =
                                covid19_example::
                                    estimate_peak_hospitalized(
                                        region_name,
                                        population_size,
                                        seed);

                            partial_sum +=
                                result.peak_hospitalized;
                        }

                        // R9: Return the local result through the future.
                        return partial_sum;
                    }));
        }

        /*
         * R9: Only the main thread combines the partial sums.
         *
         * future::get() waits for completion and rethrows any
         * exception raised by the worker task.
         */
        auto total_peak = std::uint64_t{0};

        for (auto &task : tasks)
        {
            total_peak += task.get();
        }

        return static_cast<double>(total_peak) /
               static_cast<double>(simulation_count);
    }

    void benchmark_nnj_single_core(
        benchmark::State &state)
    {
        for (auto _ : state)
        {
            // R10: Benchmark a fixed batch of NNJ simulations
            // using sequential execution.
            auto average_peak =
                estimate_average_peak_hospitalized_single_core(
                    "NNJ",
                    nnj_population,
                    benchmark_simulation_count,
                    42U);

            benchmark::DoNotOptimize(average_peak);
        }

        /*
         * R10: Record the number of completed simulations so that
         * Google Benchmark can report simulation throughput.
         */
        state.SetItemsProcessed(
            static_cast<std::int64_t>(
                state.iterations()) *
            static_cast<std::int64_t>(
                benchmark_simulation_count));
    }

    void benchmark_nnj_multi_core(
        benchmark::State &state)
    {
        for (auto _ : state)
        {
            // R10: Benchmark the same NNJ workload using
            // concurrent execution.
            auto average_peak =
                estimate_average_peak_hospitalized_multi_core(
                    "NNJ",
                    nnj_population,
                    benchmark_simulation_count,
                    42U);

            benchmark::DoNotOptimize(average_peak);
        }

        state.SetItemsProcessed(
            static_cast<std::int64_t>(
                state.iterations()) *
            static_cast<std::int64_t>(
                benchmark_simulation_count));
    }

    void benchmark_ndk_single_core(
        benchmark::State &state)
    {
        for (auto _ : state)
        {
            /*
             * R10: Benchmark a reduced but explicitly reported
             * NDK workload using sequential execution.
             *
             * The identical simulation count and seeds are used
             * in the corresponding multi-core benchmark.
             */
            auto average_peak =
                estimate_average_peak_hospitalized_single_core(
                    "NDK",
                    ndk_population,
                    benchmark_simulation_count,
                    10'000U);

            benchmark::DoNotOptimize(average_peak);
        }

        state.SetItemsProcessed(
            static_cast<std::int64_t>(
                state.iterations()) *
            static_cast<std::int64_t>(
                benchmark_simulation_count));
    }

    void benchmark_ndk_multi_core(
        benchmark::State &state)
    {
        for (auto _ : state)
        {
            // R10: Benchmark the same NDK workload using
            // concurrent execution.
            auto average_peak =
                estimate_average_peak_hospitalized_multi_core(
                    "NDK",
                    ndk_population,
                    benchmark_simulation_count,
                    10'000U);

            benchmark::DoNotOptimize(average_peak);
        }

        state.SetItemsProcessed(
            static_cast<std::int64_t>(
                state.iterations()) *
            static_cast<std::int64_t>(
                benchmark_simulation_count));
    }

} // namespace

/*
 * R10: Measure wall-clock time because the multi-core implementation
 * executes work concurrently across asynchronous tasks.
 *
 * Each benchmark performs one deliberately expensive batch. The same
 * simulation count and seed range are used when comparing single-core
 * and multi-core execution.
 */
BENCHMARK(benchmark_nnj_single_core)
    ->Name("Covid19/NNJ/SingleCore/10_simulations")
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

BENCHMARK(benchmark_nnj_multi_core)
    ->Name("Covid19/NNJ/MultiCore/10_simulations")
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

BENCHMARK(benchmark_ndk_single_core)
    ->Name("Covid19/NDK/SingleCore/10_simulations")
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

BENCHMARK(benchmark_ndk_multi_core)
    ->Name("Covid19/NDK/MultiCore/10_simulations")
    ->Unit(benchmark::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

BENCHMARK_MAIN();