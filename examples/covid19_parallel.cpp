#include "covid19_helper.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <thread>
#include <vector>

namespace
{

    // R9: Result type for reporting the average Covid-19 peak over many simulations.
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

    // R9: Chooses a safe number of worker threads based on available hardware.
    auto choose_worker_count(const std::size_t simulation_count) -> std::size_t
    {
        const auto hardware_threads = std::thread::hardware_concurrency();

        const auto available_workers =
            hardware_threads == 0
                ? std::size_t{1}
                : static_cast<std::size_t>(hardware_threads);

        // R9: Avoid over-subscription by never creating more workers than simulations
        // or available hardware threads.
        return std::min(simulation_count, available_workers);
    }

    // R9: Runs many independent Covid-19 simulations in parallel and averages the peak.
    auto estimate_average_peak_hospitalized(
        const std::string &population_name,
        const std::uint32_t population_size,
        const std::size_t simulation_count,
        const std::uint32_t first_seed) -> AveragePeakHospitalizedResult
    {
        // R9: Each simulation writes its peak to a unique index, avoiding data races.
        auto peaks = std::vector<std::size_t>(simulation_count);

        // R9: Atomic work counter lets each worker claim a unique simulation index.
        // This avoids data races without creating one thread per simulation.
        std::atomic<std::size_t> next_simulation{0};

        // R9: Limit the number of worker threads to avoid over-subscription.
        const auto worker_count = choose_worker_count(simulation_count);

        auto workers = std::vector<std::thread>{};
        workers.reserve(worker_count);

        // R9: Start a bounded number of workers for the parallel simulation batch.
        for (std::size_t worker_index = 0; worker_index < worker_count; ++worker_index)
        {
            workers.emplace_back(
                [&population_name,
                 population_size,
                 simulation_count,
                 first_seed,
                 &next_simulation,
                 &peaks]()
                {
                    while (true)
                    {
                        // R9: Atomically claim the next simulation index.
                        const auto simulation_index =
                            next_simulation.fetch_add(1, std::memory_order_relaxed);

                        if (simulation_index >= simulation_count)
                        {
                            break;
                        }

                        // R9: Use a different deterministic seed for each simulation.
                        const auto seed =
                            first_seed + static_cast<std::uint32_t>(simulation_index);

                        // R7/R9: Estimate the peak using the observer-based Covid-19 helper.
                        const auto result =
                            covid19_example::estimate_peak_hospitalized(
                                population_name,
                                population_size,
                                seed);

                        // R9: Safe write because each simulation_index is claimed once.
                        peaks[simulation_index] = result.peak_hospitalized;
                    }
                });
        }

        // R9: Join all workers before reading the collected peak values.
        for (auto &worker : workers)
        {
            worker.join();
        }

        // R9: Aggregate the results from the 100 independent simulations.
        const auto total_peak =
            std::accumulate(peaks.begin(), peaks.end(), std::uint64_t{0});

        const auto [minimum_peak, maximum_peak] =
            std::minmax_element(peaks.begin(), peaks.end());

        return AveragePeakHospitalizedResult{
            .population_size = population_size,
            .population_name = population_name,
            .simulation_count = simulation_count,
            .worker_count = worker_count,
            .minimum_peak = *minimum_peak,
            .maximum_peak = *maximum_peak,
            .average_peak = static_cast<double>(total_peak) /
                            static_cast<double>(simulation_count),
        };
    }

    // R9: Prints the multi-core average peak result for the report.
    void print_result(const AveragePeakHospitalizedResult &result)
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
    // R9: Demonstrate multi-core computation over 100 simulations.
    constexpr auto simulation_count = std::size_t{100};

    // R7/R9: Estimate the average hospitalized peak for NNJ.
    const auto nnj =
        estimate_average_peak_hospitalized("NNJ", 589'755, simulation_count, 42U);

    // R7/R9: Estimate the average hospitalized peak for NDK.
    const auto ndk =
        estimate_average_peak_hospitalized("NDK", 5'822'763, simulation_count, 10'000U);

    print_result(nnj);
    print_result(ndk);
}