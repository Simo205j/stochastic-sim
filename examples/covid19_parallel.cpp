#include "covid19_helper.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <future>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace
{

    // R9: Result type for reporting the average Covid-19 peak
    // over many independent simulations.
    struct AveragePeakHospitalizedResult
    {
        std::uint32_t population_size;
        std::string population_name;
        std::size_t simulation_count;
        std::size_t worker_count;
        std::size_t minimum_peak;
        std::size_t maximum_peak;
        double average_peak;
    };

    // R9: Partial result produced independently by one worker task.
    //
    // Each worker owns and modifies its own summary. Consequently,
    // no worker writes to shared mutable result data.
    struct WorkerPeakSummary
    {
        std::uint64_t total_peak{0};
        std::size_t simulation_count{0};

        std::size_t minimum_peak{
            std::numeric_limits<std::size_t>::max()};

        std::size_t maximum_peak{0};
    };

    // R9: Chooses a bounded number of worker tasks based on the
    // number of simulations and the available hardware concurrency.
    auto choose_worker_count(
        const std::size_t simulation_count) -> std::size_t
    {
        const auto hardware_threads =
            std::thread::hardware_concurrency();

        // R9: hardware_concurrency() may return zero when the number
        // of available hardware threads cannot be determined.
        const auto available_workers =
            hardware_threads == 0
                ? std::size_t{1}
                : static_cast<std::size_t>(hardware_threads);

        // R9: Limit this simulation batch to no more workers than
        // available hardware threads or simulations.
        return std::min(simulation_count, available_workers);
    }

    // R9: Runs many independent Covid-19 simulations concurrently
    // and calculates their average, minimum and maximum peaks.
    auto estimate_average_peak_hospitalized(
        const std::string &population_name,
        const std::uint32_t population_size,
        const std::size_t simulation_count,
        const std::uint32_t first_seed)
        -> AveragePeakHospitalizedResult
    {
        // R9: Reject an empty simulation batch because no meaningful
        // average, minimum or maximum could be calculated.
        if (simulation_count == 0)
        {
            throw std::invalid_argument{
                "simulation_count must be greater than zero"};
        }

        // R9: Bound the amount of parallel work to reduce the risk
        // of local over-subscription.
        const auto worker_count =
            choose_worker_count(simulation_count);

        // R9: Each future represents one independently executing
        // worker task and eventually contains its partial result.
        auto tasks =
            std::vector<std::future<WorkerPeakSummary>>{};

        tasks.reserve(worker_count);

        // R9: Start a bounded number of asynchronous worker tasks.
        for (std::size_t worker_index = 0;
             worker_index < worker_count;
             ++worker_index)
        {
            tasks.emplace_back(
                std::async(
                    std::launch::async,

                    // R9: Capture the configuration by value so that
                    // each worker has its own safe copies.
                    [population_name,
                     population_size,
                     simulation_count,
                     first_seed,
                     worker_index,
                     worker_count]()
                    {
                        // R9: This summary belongs exclusively to the
                        // current worker and is never shared.
                        auto summary = WorkerPeakSummary{};

                        /*
                         * R9: Divide simulation indices between workers
                         * using a strided partition.
                         *
                         * For four workers:
                         *
                         * Worker 0: 0, 4, 8, ...
                         * Worker 1: 1, 5, 9, ...
                         * Worker 2: 2, 6, 10, ...
                         * Worker 3: 3, 7, 11, ...
                         *
                         * Every simulation index belongs to exactly one
                         * worker. Therefore, no shared atomic work counter
                         * or mutex is required.
                         */
                        for (std::size_t simulation_index = worker_index;
                             simulation_index < simulation_count;
                             simulation_index += worker_count)
                        {
                            // R9: Use a different deterministic seed
                            // for every independent simulation.
                            const auto seed =
                                first_seed +
                                static_cast<std::uint32_t>(
                                    simulation_index);

                            /*
                             * R7/R9: Calculate the peak hospitalisation
                             * using the observer-based Covid-19 helper.
                             *
                             * The helper observes the peak during the
                             * simulation without storing the complete
                             * trajectory.
                             *
                             * Each invocation creates its own simulation
                             * state and random-number generator.
                             */
                            const auto result =
                                covid19_example::
                                    estimate_peak_hospitalized(
                                        population_name,
                                        population_size,
                                        seed);

                            const auto peak =
                                result.peak_hospitalized;

                            // R9: Update only this worker's local summary.
                            summary.total_peak += peak;
                            ++summary.simulation_count;

                            summary.minimum_peak =
                                std::min(
                                    summary.minimum_peak,
                                    peak);

                            summary.maximum_peak =
                                std::max(
                                    summary.maximum_peak,
                                    peak);
                        }

                        // R9: Return the partial result through the future
                        // rather than modifying shared result storage.
                        return summary;
                    }));
        }

        /*
         * R9: Aggregate the partial worker results sequentially.
         *
         * Only the main thread modifies these variables, so the final
         * reduction does not require atomics or mutexes.
         */
        auto total_peak = std::uint64_t{0};

        auto minimum_peak =
            std::numeric_limits<std::size_t>::max();

        auto maximum_peak = std::size_t{0};
        auto completed_simulations = std::size_t{0};

        for (auto &task : tasks)
        {
            /*
             * R9: future::get() waits for the task to complete and
             * retrieves its result.
             *
             * If a simulation task throws an exception, get() rethrows
             * it here in the main thread.
             */
            const auto summary = task.get();

            total_peak += summary.total_peak;
            completed_simulations += summary.simulation_count;

            minimum_peak =
                std::min(
                    minimum_peak,
                    summary.minimum_peak);

            maximum_peak =
                std::max(
                    maximum_peak,
                    summary.maximum_peak);
        }

        // R9: Return the combined statistics from all independent
        // Covid-19 simulations.
        return AveragePeakHospitalizedResult{
            .population_size = population_size,
            .population_name = population_name,
            .simulation_count = completed_simulations,
            .worker_count = worker_count,
            .minimum_peak = minimum_peak,
            .maximum_peak = maximum_peak,
            .average_peak =
                static_cast<double>(total_peak) /
                static_cast<double>(completed_simulations),
        };
    }

    // R9: Prints the multi-core average peak result for the report.
    void print_result(
        const AveragePeakHospitalizedResult &result)
    {
        std::cout << result.population_name
                  << " (N = "
                  << result.population_size
                  << ", simulations = "
                  << result.simulation_count
                  << ", workers = "
                  << result.worker_count
                  << "): average peak hospitalized = "
                  << result.average_peak
                  << " [min = "
                  << result.minimum_peak
                  << ", max = "
                  << result.maximum_peak
                  << "]\n";
    }

} // namespace

int main()
{
    // R9: Demonstrate multi-core computation over 100 simulations,
    // as required by the assignment.
    constexpr auto simulation_count =
        std::size_t{100};

    // R7/R9: Estimate the average hospitalisation peak for NNJ
    // using 100 independent observer-based simulations.
    const auto nnj =
        estimate_average_peak_hospitalized(
            "NNJ",
            589'755,
            simulation_count,
            42U);

    // R7/R9: Estimate the average hospitalisation peak for NDK
    // using a separate deterministic seed range.
    const auto ndk =
        estimate_average_peak_hospitalized(
            "NDK",
            5'822'763,
            simulation_count,
            10'000U);

    // R9: Print the recorded average, minimum and maximum peaks.
    print_result(nnj);
    print_result(ndk);
}