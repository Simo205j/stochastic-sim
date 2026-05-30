#include "stochastic/reaction.hpp"
#include "stochastic/simulation.hpp"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace
{

    struct PeakHospitalizedResult
    {
        std::uint32_t population_size;
        std::string population_name;
        std::size_t peak_hospitalized;
    };

    auto estimate_peak_hospitalized(
        const std::string &population_name,
        const std::uint32_t population_size) -> PeakHospitalizedResult
    {
        using namespace stochastic;

        constexpr auto eps = 0.0009;
        constexpr auto basic_reproduction_number = 2.4;
        constexpr auto alpha = 1.0 / 5.1;
        constexpr auto gamma = 1.0 / 3.1;
        constexpr auto hospitalization_probability = 0.9e-3;
        constexpr auto tau = 1.0 / 10.12;

        const auto initial_infectious =
            static_cast<std::size_t>(std::round(eps * population_size));

        const auto initial_exposed =
            static_cast<std::size_t>(std::round(eps * population_size * 15.0));

        const auto initial_susceptible =
            static_cast<std::size_t>(population_size) - initial_infectious - initial_exposed;

        const auto beta = basic_reproduction_number * gamma;
        const auto kappa = gamma * hospitalization_probability * (1.0 - hospitalization_probability);

        const auto S = Reactant{0};
        const auto E = Reactant{1};
        const auto I = Reactant{2};
        const auto H = Reactant{3};
        const auto R = Reactant{4};

        auto state = State{
            initial_susceptible, // S
            initial_exposed,     // E
            initial_infectious,  // I
            0,                   // H
            0,                   // R
        };

        const auto reactions = std::vector<Reaction>{
            (S + I) >> (beta / static_cast<double>(population_size)) >>= E + I,
            E >> alpha >>= I,
            I >> gamma >>= R,
            I >> kappa >>= H,
            H >> tau >>= R,
        };

        auto random_generator = std::mt19937{42U};
        auto peak_hospitalized = std::size_t{0};

        constexpr auto end_time = 100.0; // days

        simulate(
            reactions,
            state,
            end_time,
            random_generator,
            [&peak_hospitalized, H](double, const State &current_state)
            {
                if (current_state[H.id] > peak_hospitalized)
                {
                    peak_hospitalized = current_state[H.id];
                }
            });

        return PeakHospitalizedResult{
            .population_size = population_size,
            .population_name = population_name,
            .peak_hospitalized = peak_hospitalized,
        };
    }

    void print_result(const PeakHospitalizedResult &result)
    {
        std::cout << result.population_name
                  << " (N = "
                  << result.population_size
                  << "): peak hospitalized = "
                  << result.peak_hospitalized
                  << '\n';
    }

} // namespace

int main()
{
    const auto nnj = estimate_peak_hospitalized("NNJ", 589'755);
    const auto ndk = estimate_peak_hospitalized("NDK", 5'822'763);

    print_result(nnj);
    print_result(ndk);
}