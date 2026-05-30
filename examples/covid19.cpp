#include "stochastic/reaction.hpp"
#include "stochastic/simulation.hpp"

#include "trajectory_writer.hpp"

#include <cmath>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace
{

    void run_covid19_example(const std::uint32_t population_size)
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

        auto writer = stochastic::examples::TrajectoryWriter{
            "covid19_N" + std::to_string(population_size) + ".csv",
            {"time", "S", "E", "I", "H", "R"},
        };

        auto peak_hospitalized = std::size_t{0};

        constexpr auto end_time = 100.0; // days

        simulate(
            reactions,
            state,
            end_time,
            random_generator,
            [&writer, &peak_hospitalized](const double time, const State &current_state)
            {
                writer.write_row(time, current_state);

                if (current_state[3] > peak_hospitalized)
                {
                    peak_hospitalized = current_state[3];
                }
            });

        std::cout << "Wrote Covid-19 trajectory for N = "
                  << population_size
                  << ". Peak hospitalized = "
                  << peak_hospitalized
                  << '\n';
    }

} // namespace

int main()
{
    run_covid19_example(10'000);
}