#include <catch2/catch_test_macros.hpp>

#include <stochastic/reaction.hpp>
#include <stochastic/symbol_table.hpp>

#include <functional>
#include <string>

TEST_CASE("symbol table starts empty")
{
    using namespace stochastic;

    const auto symbols = SymbolTable<std::string, Reactant>{};

    CHECK(symbols.size() == 0);
    CHECK_FALSE(symbols.contains("A"));
}

TEST_CASE("symbol table can store and lookup reactants")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};

    const auto A = Reactant{0};
    const auto B = Reactant{1};

    symbols.add("A", A);
    symbols.add("B", B);

    CHECK(symbols.size() == 2);
    CHECK(symbols.contains("A"));
    CHECK(symbols.contains("B"));

    CHECK(symbols.lookup("A").id == A.id);
    CHECK(symbols.lookup("B").id == B.id);
}

TEST_CASE("symbol table fails when looking up missing symbol")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};

    CHECK_THROWS_AS(symbols.lookup("A"), SymbolNotFound);
}

TEST_CASE("symbol table lookup failure does not modify table")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};

    symbols.add("A", Reactant{0});

    CHECK_THROWS_AS(symbols.lookup("B"), SymbolNotFound);

    CHECK(symbols.size() == 1);
    CHECK(symbols.contains("A"));
    CHECK_FALSE(symbols.contains("B"));
}

TEST_CASE("symbol table fails when adding duplicate symbol")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};

    symbols.add("A", Reactant{0});

    CHECK_THROWS_AS(symbols.add("A", Reactant{1}), SymbolAlreadyDefined);
}

TEST_CASE("symbol table duplicate add does not replace existing value")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};

    symbols.add("A", Reactant{0});

    CHECK_THROWS_AS(symbols.add("A", Reactant{42}), SymbolAlreadyDefined);

    CHECK(symbols.size() == 1);
    CHECK(symbols.lookup("A").id == 0);
}

TEST_CASE("symbol table lookup works through const reference")
{
    using namespace stochastic;

    auto symbols = SymbolTable<std::string, Reactant>{};
    symbols.add("A", Reactant{0});

    const auto &const_symbols = symbols;

    CHECK(const_symbols.lookup("A").id == 0);
    CHECK(const_symbols.contains("A"));
    CHECK(const_symbols.size() == 1);
}

TEST_CASE("symbol table is generic over key and value types")
{
    using namespace stochastic;

    auto symbols = SymbolTable<int, std::string>{};

    symbols.add(1, "A");
    symbols.add(2, "B");

    CHECK(symbols.lookup(1) == "A");
    CHECK(symbols.lookup(2) == "B");
    CHECK(symbols.size() == 2);
}

TEST_CASE("symbol table supports a custom comparator")
{
    using namespace stochastic;

    auto symbols = SymbolTable<int, std::string, std::greater<int>>{};

    symbols.add(2, "B");
    symbols.add(1, "A");

    CHECK(symbols.lookup(1) == "A");
    CHECK(symbols.lookup(2) == "B");
    CHECK(symbols.contains(1));
    CHECK(symbols.contains(2));
}