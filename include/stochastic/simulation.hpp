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
            for (const auto &term : reaction.inputs.terms())
            {
                if (term.reactant.id >= state.size())
                {
                    return false;
                }

                if (state[term.reactant.id] == 0)
                {
                    return false;
                }
            }

            return true;
        }

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

        inline auto reaction_rate(const Reaction &reaction, const State &state) -> double
        {
            auto rate = reaction.rate;

            for (const auto &term : reaction.inputs.terms())
            {
                if (term.reactant.id >= state.size())
                {
                    return 0.0;
                }

                const auto amount = state[term.reactant.id];

                if (amount == 0)
                {
                    return 0.0;
                }

                rate *= static_cast<double>(amount);
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