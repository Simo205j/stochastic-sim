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

TEST_CASE("simulation with no reactions only observes the initial state")
{
    using namespace stochastic;

    auto reactions = std::vector<Reaction>{};
    auto state = State{3, 4};

    auto random_generator = std::mt19937{42};

    auto observed_states = std::vector<State>{};

    simulate(
        reactions,
        state,
        10.0,
        random_generator,
        [&observed_states](double, const State &observed_state)
        {
            observed_states.push_back(observed_state);
        });

    REQUIRE(observed_states.size() == 1);

    CHECK(observed_states.front()[0] == 3);
    CHECK(observed_states.front()[1] == 4);

    CHECK(state[0] == 3);
    CHECK(state[1] == 4);
}

TEST_CASE("simulation stops before applying reaction after end time")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    const auto reaction = A >> 1.0 >>= B;

    auto reactions = std::vector<Reaction>{reaction};
    auto state = State{1, 0};

    auto random_generator = std::mt19937{42};

    simulate(
        reactions,
        state,
        0.0,
        random_generator,
        [](double, const State &) {});

    CHECK(state[A.id] == 1);
    CHECK(state[B.id] == 0);
}

TEST_CASE("simulation overload without observer still runs")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    const auto reaction = A >> 100.0 >>= B;

    auto reactions = std::vector<Reaction>{reaction};
    auto state = State{1, 0};

    auto random_generator = std::mt19937{42};

    simulate(reactions, state, 10.0, random_generator);

    CHECK(state[A.id] == 0);
    CHECK(state[B.id] == 1);
}

TEST_CASE("reaction rate is zero when an input amount is missing")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    const auto reaction = A >> 2.0 >>= B;

    const auto state = State{0, 0};

    CHECK(detail::reaction_rate(reaction, state) == 0.0);
}

TEST_CASE("reaction rate is zero when reactant id is outside state")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto Missing = Reactant{5};

    const auto reaction = (A + Missing) >> 2.0 >>= A;

    const auto state = State{1};

    CHECK(detail::reaction_rate(reaction, state) == 0.0);
    CHECK_FALSE(detail::has_inputs_available(reaction, state));
}

TEST_CASE("reaction rate is multiplied by available input amounts")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto C = Reactant{1};
    const auto B = Reactant{2};

    const auto reaction = (A + C) >> 0.5 >>= (B + C);

    const auto state = State{3, 4, 0};

    CHECK(detail::reaction_rate(reaction, state) == 6.0);
}

TEST_CASE("apply reaction consumes inputs and produces products")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};
    const auto C = Reactant{2};

    const auto reaction = (A + C) >> 1.0 >>= (B + C);

    auto state = State{2, 0, 1};

    REQUIRE(detail::has_inputs_available(reaction, state));

    detail::apply_reaction(reaction, state);

    CHECK(state[A.id] == 1);
    CHECK(state[B.id] == 1);
    CHECK(state[C.id] == 1);
}

TEST_CASE("simulation stops when no reactions are possible")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    const auto reaction = A >> 1.0 >>= B;

    auto reactions = std::vector<Reaction>{reaction};
    auto state = State{0, 0};

    auto random_generator = std::mt19937{42};

    auto observer_call_count = std::size_t{0};

    simulate(
        reactions,
        state,
        10.0,
        random_generator,
        [&observer_call_count](double, const State &)
        {
            ++observer_call_count;
        });

    CHECK(observer_call_count == 1);
    CHECK(state[A.id] == 0);
    CHECK(state[B.id] == 0);
}

TEST_CASE("reaction requiring same input twice needs enough amount")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    const auto reaction = (A + A) >> 1.0 >>= B;

    auto state = State{1, 0};

    CHECK_FALSE(detail::has_inputs_available(reaction, state));
}

TEST_CASE("reaction requiring same input twice is possible with enough amount")
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    const auto reaction = (A + A) >> 1.0 >>= B;

    auto state = State{2, 0};

    CHECK(detail::has_inputs_available(reaction, state));

    detail::apply_reaction(reaction, state);

    CHECK(state[A.id] == 0);
    CHECK(state[B.id] == 1);
}