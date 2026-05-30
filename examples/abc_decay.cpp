#include "stochastic/reaction.hpp"
#include "stochastic/simulation.hpp"

#include "trajectory_writer.hpp"

#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace
{

    void run_abc_decay_example(
        const std::string &output_path,
        stochastic::State initial_state,
        const double end_time,
        const unsigned int seed)
    {
        using namespace stochastic;

        constexpr auto lambda = 0.001;

        const auto A = Reactant{0};
        const auto B = Reactant{1};
        const auto C = Reactant{2};

        const auto reaction = (A + C) >> lambda >>= (B + C);
        const auto reactions = std::vector<Reaction>{reaction};

        auto random_generator = std::mt19937{seed};

        auto writer = stochastic::examples::TrajectoryWriter{
            output_path,
            {"time", "A", "B", "C"},
        };

        simulate(
            reactions,
            initial_state,
            end_time,
            random_generator,
            [&writer](const double time, const State &state)
            {
                writer.write_row(time, {state[0], state[1], state[2]});
            });
    }

} // namespace

int main()
{
    constexpr auto end_time = 2'000.0;
    constexpr auto seed = 42U;

    run_abc_decay_example(
        "abc_decay_A100_B0_C1.csv",
        stochastic::State{100, 0, 1},
        end_time,
        seed);

    run_abc_decay_example(
        "abc_decay_A100_B0_C2.csv",
        stochastic::State{100, 0, 2},
        end_time,
        seed);

    run_abc_decay_example(
        "abc_decay_A50_B50_C1.csv",
        stochastic::State{50, 50, 1},
        end_time,
        seed);

    std::cout << "Wrote ABC decay trajectories.\n";
}