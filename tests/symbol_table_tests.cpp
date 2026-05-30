#include <catch2/catch_test_macros.hpp>

#include <stochastic/reaction.hpp>
#include <stochastic/symbol_table.hpp>

#include <string>

TEST_CASE("symbol table can store and lookup reactants")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    symbols.add("A", A);
    symbols.add("B", B);

    CHECK(symbols.lookup("A").id == A.id);
    CHECK(symbols.lookup("B").id == B.id);
}

TEST_CASE("symbol table fails when looking up missing symbol")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};

    CHECK_THROWS_AS(symbols.lookup("A"), SymbolNotFound);
}

TEST_CASE("symbol table fails when adding duplicate symbol")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};

    symbols.add("A", Reactant{0});

    CHECK_THROWS_AS(symbols.add("A", Reactant{1}), SymbolAlreadyDefined);
}