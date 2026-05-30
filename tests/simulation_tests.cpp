#include "stochastic/reaction.hpp"
#include "stochastic/simulation.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <random>
#include <vector>

TEST_CASE("simulation consumes inputs and produces products")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};
    const auto C = Reactant{2};

    const auto reaction = (A + C) >> 1.0 >>= (B + C);

    auto reactions = std::vector<Reaction>{reaction};
    auto state = State{1, 0, 1};

    auto random_generator = std::mt19937{42};

    simulate(
        reactions,
        state,
        10.0,
        random_generator,
        [](double, const State &) {});

    CHECK(state[A.id] == 0);
    CHECK(state[B.id] == 1);
    CHECK(state[C.id] == 1);
}

TEST_CASE("simulation does nothing when required inputs are missing")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};
    const auto C = Reactant{2};

    const auto reaction = (A + C) >> 1.0 >>= (B + C);

    auto reactions = std::vector<Reaction>{reaction};
    auto state = State{0, 0, 1};

    auto random_generator = std::mt19937{42};

    simulate(
        reactions,
        state,
        10.0,
        random_generator,
        [](double, const State &) {});

    CHECK(state[A.id] == 0);
    CHECK(state[B.id] == 0);
    CHECK(state[C.id] == 1);
}

TEST_CASE("simulation calls observer with initial and updated states")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    const auto reaction = A >> 1.0 >>= B;

    auto reactions = std::vector<Reaction>{reaction};
    auto state = State{1, 0};

    auto observed_states = std::vector<State>{};
    auto random_generator = std::mt19937{42};

    simulate(
        reactions,
        state,
        10.0,
        random_generator,
        [&observed_states](double, const State &observed_state)
        {
            observed_states.push_back(observed_state);
        });

    REQUIRE(observed_states.size() >= 2);

    CHECK(observed_states.front()[A.id] == 1);
    CHECK(observed_states.front()[B.id] == 0);

    CHECK(observed_states.back()[A.id] == 0);
    CHECK(observed_states.back()[B.id] == 1);
}

TEST_CASE("simulation observer can compute peak amount without storing trajectory")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    const auto reaction = A >> 1.0 >>= B;

    auto reactions = std::vector<Reaction>{reaction};
    auto state = State{5, 0};

    auto random_generator = std::mt19937{42};

    auto observer_call_count = std::size_t{0};
    auto peak_B = std::size_t{0};

    simulate(
        reactions,
        state,
        10.0,
        random_generator,
        [&](double, const State &observed_state)
        {
            ++observer_call_count;

            if (observed_state[B.id] > peak_B)
            {
                peak_B = observed_state[B.id];
            }
        });

    CHECK(observer_call_count >= 2);
    CHECK(peak_B == 5);
    CHECK(state[A.id] == 0);
    CHECK(state[B.id] == 5);
}