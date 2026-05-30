#include "stochastic/reaction.hpp"
#include "stochastic/simulation.hpp"

#include "trajectory_writer.hpp"

#include <iostream>
#include <random>
#include <vector>

namespace
{

    void run_circadian_rhythm_example()
    {
        using namespace stochastic;

        constexpr auto alphaA = 50.0;
        constexpr auto alpha_A = 500.0;
        constexpr auto alphaR = 0.01;
        constexpr auto alpha_R = 50.0;
        constexpr auto betaA = 50.0;
        constexpr auto betaR = 5.0;
        constexpr auto gammaA = 1.0;
        constexpr auto gammaR = 1.0;
        constexpr auto gammaC = 2.0;
        constexpr auto deltaA = 1.0;
        constexpr auto deltaR = 0.2;
        constexpr auto deltaMA = 10.0;
        constexpr auto deltaMR = 0.5;
        constexpr auto thetaA = 50.0;
        constexpr auto thetaR = 100.0;

        const auto DA = Reactant{0};
        const auto D_A = Reactant{1};
        const auto DR = Reactant{2};
        const auto D_R = Reactant{3};
        const auto MA = Reactant{4};
        const auto MR = Reactant{5};
        const auto A = Reactant{6};
        const auto R = Reactant{7};
        const auto C = Reactant{8};

        const auto environment = ReactionSide{};

        auto state = State{
            1, // DA
            0, // D_A
            1, // DR
            0, // D_R
            0, // MA
            0, // MR
            0, // A
            0, // R
            0, // C
        };

        const auto reactions = std::vector<Reaction>{
            (A + DA) >> gammaA >>= D_A,
            D_A >> thetaA >>= DA + A,

            (A + DR) >> gammaR >>= D_R,
            D_R >> thetaR >>= DR + A,

            D_A >> alpha_A >>= MA + D_A,
            DA >> alphaA >>= MA + DA,

            D_R >> alpha_R >>= MR + D_R,
            DR >> alphaR >>= MR + DR,

            MA >> betaA >>= MA + A,
            MR >> betaR >>= MR + R,

            (A + R) >> gammaC >>= C,
            C >> deltaA >>= R,

            A >> deltaA >>= environment,
            R >> deltaR >>= environment,
            MA >> deltaMA >>= environment,
            MR >> deltaMR >>= environment,
        };

        auto random_generator = std::mt19937{42U};

        auto writer = stochastic::examples::TrajectoryWriter{
            "circadian_rhythm.csv",
            {"time", "DA", "D_A", "DR", "D_R", "MA", "MR", "A", "R", "C"},
        };

        constexpr auto end_time = 72.0; // hours

        simulate(
            reactions,
            state,
            end_time,
            random_generator,
            [&writer](const double time, const State &current_state)
            {
                writer.write_row(time, current_state);
            });
    }

} // namespace

int main()
{
    run_circadian_rhythm_example();

    std::cout << "Wrote circadian rhythm trajectory.\n";
}