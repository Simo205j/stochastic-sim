#include "stochastic/dot_printer.hpp"
#include "stochastic/reaction.hpp"

#include <iostream>

int main()
{
    using namespace stochastic;

    // R1/R2: Define reactants used to demonstrate reaction typesetting and graph printing.
    const auto A = Reactant{0};
    const auto B = Reactant{1};
    const auto C = Reactant{2};

    // R1: Reaction rules are written directly in C++ using the overloaded operators.
    const auto reaction1 = A >> 0.5 >>= B;
    const auto reaction2 = A + C >> 0.001 >>= B + C;

    {
        // R2: DotPrinter implements the visitor used to print the reaction network.
        auto printer = DotPrinter{
            std::cout,
            {
                {A.id, "A"},
                {B.id, "B"},
                {C.id, "C"},
            },
        };

        // R2: Visiting each reaction emits the graph representation.
        reaction1.accept(printer);
        reaction2.accept(printer);
    }

    return 0;
}