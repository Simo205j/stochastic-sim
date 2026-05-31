#pragma once

#include <cstdint>
#include <cstddef>
#include <string>

namespace covid19_example
{

    struct PeakHospitalizedResult
    {
        std::uint32_t population_size;
        std::string population_name;
        std::size_t peak_hospitalized;
    };

    auto estimate_peak_hospitalized(
        const std::string &population_name,
        std::uint32_t population_size,
        std::uint32_t seed) -> PeakHospitalizedResult;

} // namespace covid19_example