#include "stochastic/dot_printer.hpp"
#include "stochastic/reaction.hpp"

#include <iostream>

int main()
{
    using namespace stochastic;

    const auto A = Reactant{0};
    const auto B = Reactant{1};
    const auto C = Reactant{2};

    const auto reaction1 = A >> 0.5 >>= B;
    const auto reaction2 = A + C >> 0.001 >>= B + C;

    {
        auto printer = DotPrinter{
            std::cout,
            {
                {A.id, "A"},
                {B.id, "B"},
                {C.id, "C"},
            },
        };

        reaction1.accept(printer);
        reaction2.accept(printer);
    }

    return 0;
}