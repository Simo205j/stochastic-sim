#include "covid19_helper.hpp"

#include "stochastic/reaction.hpp"
#include "stochastic/simulation.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace covid19_example
{

    auto estimate_peak_hospitalized(
        const std::string &population_name,
        const std::uint32_t population_size,
        const std::uint32_t seed) -> PeakHospitalizedResult
    {
        using namespace stochastic;

        // R7: Covid-19 model parameters used for estimating the hospitalized peak.
        constexpr auto eps = 0.0009;
        constexpr auto basic_reproduction_number = 2.4;
        constexpr auto alpha = 1.0 / 5.1;
        constexpr auto gamma = 1.0 / 3.1;
        constexpr auto hospitalization_probability = 0.9e-3;
        constexpr auto tau = 1.0 / 10.12;

        // R7: Initial SEIHR state for the requested population.
        const auto initial_infectious =
            static_cast<std::size_t>(std::round(eps * population_size));

        const auto initial_exposed =
            static_cast<std::size_t>(std::round(eps * population_size * 15.0));

        const auto initial_susceptible =
            static_cast<std::size_t>(population_size) - initial_infectious - initial_exposed;

        // R7: Derived reaction rates for the Covid-19 reaction network.
        const auto beta = basic_reproduction_number * gamma;

        const auto kappa =
            gamma * hospitalization_probability * (1.0 - hospitalization_probability);

        // R4/R7: Reactant identifiers used by the generic stochastic simulation algorithm.
        const auto S = Reactant{0};
        const auto E = Reactant{1};
        const auto I = Reactant{2};
        const auto H = Reactant{3};
        const auto R = Reactant{4};

        // R4/R7: Initial state vector for the Covid-19 stochastic system.
        auto state = State{
            initial_susceptible, // S
            initial_exposed,     // E
            initial_infectious,  // I
            0,                   // H
            0,                   // R
        };

        // R1/R4/R7: Covid-19 reaction rules written using the library's reaction DSL.
        const auto reactions = std::vector<Reaction>{
            (S + I) >> (beta / static_cast<double>(population_size)) >>= E + I,
            E >> alpha >>= I,
            I >> gamma >>= R,
            I >> kappa >>= H,
            H >> tau >>= R,
        };

        // R7: Seeded RNG makes each stochastic Covid-19 simulation reproducible.
        auto random_generator = std::mt19937{seed};

        // R7: Stores only the current peak instead of storing the full trajectory.
        auto peak_hospitalized = std::size_t{0};

        constexpr auto end_time = 100.0; // days

        // R4/R7: Run the generic simulation with a user-supplied observer.
        simulate(
            reactions,
            state,
            end_time,
            random_generator,
            [&peak_hospitalized, H](double, const State &current_state)
            {
                // R7: Observer updates the peak hospitalization value directly from the current state.
                if (current_state[H.id] > peak_hospitalized)
                {
                    peak_hospitalized = current_state[H.id];
                }
            });

        // R7: Return the recorded peak value for reporting NNJ/NDK Covid-19 results.
        return PeakHospitalizedResult{
            .population_size = population_size,
            .population_name = population_name,
            .peak_hospitalized = peak_hospitalized,
        };
    }

} // namespace covid19_example