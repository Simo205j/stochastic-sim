#include "covid19_helper.hpp"

#include "stochastic/reaction.hpp"
#include "stochastic/simulation.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace covid19_example
{

    namespace
    {

        // R7: This observer belongs to the user/example program rather than
        // the stochastic simulation library.
        //
        // It calculates the peak number of hospitalized agents online,
        // without storing the complete trajectory.
        //
        // It is deliberately non-copyable. This proves that simulate()
        // operates on the caller's observer instead of silently creating
        // and updating a discarded copy.
        class PeakHospitalizedObserver
        {
        public:
            explicit PeakHospitalizedObserver(
                const std::size_t hospitalized_id)
                : hospitalized_id_{hospitalized_id}
            {
            }

            PeakHospitalizedObserver(
                const PeakHospitalizedObserver &) = delete;

            auto operator=(
                const PeakHospitalizedObserver &)
                -> PeakHospitalizedObserver & = delete;

            PeakHospitalizedObserver(
                PeakHospitalizedObserver &&) = default;

            auto operator=(
                PeakHospitalizedObserver &&)
                -> PeakHospitalizedObserver & = default;

            void operator()(
                const double time,
                const stochastic::State &state)
            {
                ++observation_count_;
                last_observed_time_ = time;

                peak_hospitalized_ = std::max(
                    peak_hospitalized_,
                    state[hospitalized_id_]);
            }

            [[nodiscard]]
            auto peak_hospitalized() const noexcept
                -> std::size_t
            {
                return peak_hospitalized_;
            }

            [[nodiscard]]
            auto observation_count() const noexcept
                -> std::size_t
            {
                return observation_count_;
            }

            [[nodiscard]]
            auto last_observed_time() const noexcept
                -> double
            {
                return last_observed_time_;
            }

        private:
            std::size_t hospitalized_id_;

            std::size_t peak_hospitalized_{0};
            std::size_t observation_count_{0};
            double last_observed_time_{0.0};
        };

    } // namespace

    auto estimate_peak_hospitalized(
        const std::string &population_name,
        const std::uint32_t population_size,
        const std::uint32_t seed) -> PeakHospitalizedResult
    {
        using namespace stochastic;

        // R7: Covid-19 model parameters used for estimating
        // the hospitalized peak.
        constexpr auto eps = 0.0009;
        constexpr auto basic_reproduction_number = 2.4;
        constexpr auto alpha = 1.0 / 5.1;
        constexpr auto gamma = 1.0 / 3.1;
        constexpr auto hospitalization_probability = 0.9e-3;
        constexpr auto tau = 1.0 / 10.12;

        // R7: Initial SEIHR state for the requested population.
        const auto initial_infectious =
            static_cast<std::size_t>(
                std::round(eps * population_size));

        const auto initial_exposed =
            static_cast<std::size_t>(
                std::round(
                    eps *
                    population_size *
                    15.0));

        const auto initial_susceptible =
            static_cast<std::size_t>(population_size) -
            initial_infectious -
            initial_exposed;

        // R7: Derived reaction rates for the Covid-19 reaction network.
        const auto beta =
            basic_reproduction_number * gamma;

        const auto kappa =
            gamma *
            hospitalization_probability *
            (1.0 - hospitalization_probability);

        // R4/R7: Reactant identifiers used by the generic
        // stochastic simulation algorithm.
        const auto S = Reactant{0};
        const auto E = Reactant{1};
        const auto I = Reactant{2};
        const auto H = Reactant{3};
        const auto R = Reactant{4};

        // R4/R7: Initial state vector for the Covid-19
        // stochastic system.
        auto state = State{
            initial_susceptible, // S
            initial_exposed,     // E
            initial_infectious,  // I
            0,                   // H
            0,                   // R
        };

        // R1/R4/R7: Covid-19 reaction rules written using
        // the library's reaction DSL.
        const auto reactions = std::vector<Reaction>{
            (S + I) >>
                (beta / static_cast<double>(population_size)) >>= E + I,

            E >> alpha >>= I,
            I >> gamma >>= R,
            I >> kappa >>= H,
            H >> tau >>= R,
        };

        // R7: Seeded RNG makes each stochastic Covid-19
        // simulation reproducible.
        auto random_generator = std::mt19937{seed};

        constexpr auto end_time = 100.0; // days

        // R7: The observer belongs to the example program,
        // not to the library.
        //
        // Because it is passed as an lvalue, simulate() uses
        // this exact object rather than copying it.
        auto observer =
            PeakHospitalizedObserver{H.id};

        simulate(
            reactions,
            state,
            end_time,
            random_generator,
            observer);

        // The observer retains its state after simulate() returns.
        // No vector of trajectory states was created.
        return PeakHospitalizedResult{
            .population_size = population_size,
            .population_name = population_name,
            .peak_hospitalized =
                observer.peak_hospitalized(),
            .observation_count =
                observer.observation_count(),
            .last_observed_time =
                observer.last_observed_time(),
        };
    }

} // namespace covid19_example