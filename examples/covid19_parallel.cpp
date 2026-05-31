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

    auto choose_worker_count(const std::size_t simulation_count) -> std::size_t
    {
        const auto hardware_threads = std::thread::hardware_concurrency();

        const auto available_workers =
            hardware_threads == 0
                ? std::size_t{1}
                : static_cast<std::size_t>(hardware_threads);

        return std::min(simulation_count, available_workers);
    }

    auto estimate_average_peak_hospitalized(
        const std::string &population_name,
        const std::uint32_t population_size,
        const std::size_t simulation_count,
        const std::uint32_t first_seed) -> AveragePeakHospitalizedResult
    {
        auto peaks = std::vector<std::size_t>(simulation_count);
        // Atomic work counter: each worker claims a unique simulation index.
        // This avoids data races without creating one thread per simulation.
        std::atomic<std::size_t> next_simulation{0};

        // Limit the number of worker threads to avoid over-subscription.
        const auto worker_count = choose_worker_count(simulation_count);

        auto workers = std::vector<std::thread>{};
        workers.reserve(worker_count);

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
                        const auto simulation_index =
                            next_simulation.fetch_add(1, std::memory_order_relaxed);

                        if (simulation_index >= simulation_count)
                        {
                            break;
                        }

                        const auto seed =
                            first_seed + static_cast<std::uint32_t>(simulation_index);

                        const auto result =
                            covid19_example::estimate_peak_hospitalized(
                                population_name,
                                population_size,
                                seed);

                        peaks[simulation_index] = result.peak_hospitalized;
                    }
                });
        }

        for (auto &worker : workers)
        {
            worker.join();
        }

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
    constexpr auto simulation_count = std::size_t{100};

    const auto nnj =
        estimate_average_peak_hospitalized("NNJ", 589'755, simulation_count, 42U);

    const auto ndk =
        estimate_average_peak_hospitalized("NDK", 5'822'763, simulation_count, 10'000U);

    print_result(nnj);
    print_result(ndk);
}