#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace covid19_example
{

    // R7: Result type for reporting the Covid-19 peak hospitalization estimate.
    struct PeakHospitalizedResult
    {
        std::uint32_t population_size;
        std::string population_name;
        std::size_t peak_hospitalized;

        // Evidence that the observer was repeatedly invoked.
        std::size_t observation_count;
        double last_observed_time;
    };

    // R7: Runs one Covid-19 stochastic simulation and estimates
    // the hospitalized peak.
    auto estimate_peak_hospitalized(
        const std::string &population_name,
        std::uint32_t population_size,
        std::uint32_t seed) -> PeakHospitalizedResult;

} // namespace covid19_example