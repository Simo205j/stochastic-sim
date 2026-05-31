#pragma once

#include "stochastic/reaction.hpp"

#include <cstddef>
#include <limits>
#include <random>
#include <vector>

namespace stochastic
{

    using State = std::vector<std::size_t>;

    namespace detail
    {

        inline auto has_inputs_available(const Reaction &reaction, const State &state) -> bool
        {
            auto required = std::vector<std::size_t>(state.size(), 0);

            for (const auto &term : reaction.inputs.terms())
            {
                if (term.reactant.id >= state.size())
                {
                    return false;
                }

                ++required[term.reactant.id];

                if (state[term.reactant.id] < required[term.reactant.id])
                {
                    return false;
                }
            }

            return true;
        }

        // R4: Applies one stochastic reaction step by consuming input
        // reactants and producing output reactants.
        inline void apply_reaction(const Reaction &reaction, State &state)
        {
            for (const auto &term : reaction.inputs.terms())
            {
                --state[term.reactant.id];
            }

            for (const auto &term : reaction.products.terms())
            {
                ++state[term.reactant.id];
            }
        }

        // R4: Computes the reaction rate from the base rate and the
        // current quantities of all input reactants.
        inline auto reaction_rate(const Reaction &reaction, const State &state) -> double
        {
            if (!has_inputs_available(reaction, state))
            {
                return 0.0;
            }

            auto rate = reaction.rate;

            for (const auto &term : reaction.inputs.terms())
            {
                rate *= static_cast<double>(state[term.reactant.id]);
            }

            return rate;
        }

        struct NoObserver
        {
            void operator()(double, const State &) const
            {
            }
        };

    } // namespace detail

    // R4: Implements the stochastic simulation algorithm by repeatedly
    // sampling exponential delays and executing the reaction with the
    // shortest delay.
    //
    // R7: Supports a user-supplied observer that receives each trajectory
    // state during simulation, allowing clients to compute quantities such
    // as peak hospitalisation without storing the full trajectory.
    template <typename RandomGenerator, typename Observer>
    void simulate(
        const std::vector<Reaction> &reactions,
        State &state,
        const double end_time,
        RandomGenerator &random_generator,
        Observer observer)
    {
        auto time = 0.0;

        observer(time, state);

        while (time <= end_time)
        {
            auto best_delay = std::numeric_limits<double>::infinity();
            const Reaction *next_reaction = nullptr;

            for (const auto &reaction : reactions)
            {
                const auto rate = detail::reaction_rate(reaction, state);

                if (rate <= 0.0)
                {
                    continue;
                }

                auto distribution = std::exponential_distribution<double>{rate};
                const auto delay = distribution(random_generator);

                if (delay < best_delay)
                {
                    best_delay = delay;
                    next_reaction = &reaction;
                }
            }

            if (next_reaction == nullptr)
            {
                break;
            }

            time += best_delay;

            if (time > end_time)
            {
                break;
            }

            if (detail::has_inputs_available(*next_reaction, state))
            {
                detail::apply_reaction(*next_reaction, state);
            }

            observer(time, state);
        }
    }

    // R4: Convenience overload for running the simulator when no trajectory
    // observer is needed.
    template <typename RandomGenerator>
    void simulate(
        const std::vector<Reaction> &reactions,
        State &state,
        const double end_time,
        RandomGenerator &random_generator)
    {
        simulate(reactions, state, end_time, random_generator, detail::NoObserver{});
    }

} // namespace stochastic