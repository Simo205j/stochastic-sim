#include <sstream>
#include <catch2/catch_test_macros.hpp>

#include <stochastic/reaction.hpp>
#include "stochastic/dot_printer.hpp"

TEST_CASE("reaction rule can be typeset with operators")
{
  using namespace stochastic;

  const auto A = Reactant{0};
  const auto B = Reactant{1};
  const auto C = Reactant{2};

  const auto reaction = (A + C) >> 0.001 >>= (B + C);

  REQUIRE(reaction.rate == 0.001);

  REQUIRE(reaction.inputs.terms().size() == 2);
  CHECK(reaction.inputs.terms()[0].reactant.id == A.id);
  CHECK(reaction.inputs.terms()[1].reactant.id == C.id);

  REQUIRE(reaction.products.terms().size() == 2);
  CHECK(reaction.products.terms()[0].reactant.id == B.id);
  CHECK(reaction.products.terms()[1].reactant.id == C.id);
}

TEST_CASE("reaction can be printed as dot graph")
{
  using namespace stochastic;

  const auto A = Reactant{0};
  const auto B = Reactant{1};

  const auto reaction = A >> 0.5 >>= B;

  auto out = std::ostringstream{};

  {
    auto printer = DotPrinter{out};
    reaction.accept(printer);
  }

  CHECK(out.str() ==
        "digraph reaction_network {\n"
        "  r0 [label=\"0.5\", shape=oval];\n"
        "  x0 [label=\"x0\", shape=box];\n"
        "  x0 -> r0;\n"
        "  x1 [label=\"x1\", shape=box];\n"
        "  r0 -> x1;\n"
        "}\n");
}

TEST_CASE("reaction side can be extended with another reactant")
{
  using namespace stochastic;

  const auto A = Reactant{0};
  const auto B = Reactant{1};
  const auto C = Reactant{2};

  const auto side = A + B + C;

  REQUIRE(side.terms().size() == 3);

  CHECK(side.terms()[0].reactant.id == A.id);
  CHECK(side.terms()[1].reactant.id == B.id);
  CHECK(side.terms()[2].reactant.id == C.id);
}

TEST_CASE("reactant can be prepended to reaction side")
{
  using namespace stochastic;

  const auto A = Reactant{0};
  const auto B = Reactant{1};
  const auto C = Reactant{2};

  const auto side = A + (B + C);

  REQUIRE(side.terms().size() == 3);

  CHECK(side.terms()[0].reactant.id == A.id);
  CHECK(side.terms()[1].reactant.id == B.id);
  CHECK(side.terms()[2].reactant.id == C.id);
}

TEST_CASE("reaction side preserves duplicate reactants")
{
  using namespace stochastic;

  const auto A = Reactant{0};

  const auto side = A + A;

  REQUIRE(side.terms().size() == 2);

  CHECK(side.terms()[0].reactant.id == A.id);
  CHECK(side.terms()[1].reactant.id == A.id);
}

TEST_CASE("reaction rule supports multiple inputs and single product")
{
  using namespace stochastic;

  const auto A = Reactant{0};
  const auto B = Reactant{1};
  const auto C = Reactant{2};

  const auto reaction = (A + B) >> 2.0 >>= C;

  CHECK(reaction.rate == 2.0);

  REQUIRE(reaction.inputs.terms().size() == 2);
  CHECK(reaction.inputs.terms()[0].reactant.id == A.id);
  CHECK(reaction.inputs.terms()[1].reactant.id == B.id);

  REQUIRE(reaction.products.terms().size() == 1);
  CHECK(reaction.products.terms()[0].reactant.id == C.id);
}

TEST_CASE("reaction rule supports single input and multiple products")
{
  using namespace stochastic;

  const auto A = Reactant{0};
  const auto B = Reactant{1};
  const auto C = Reactant{2};

  const auto reaction = A >> 2.0 >>= (B + C);

  CHECK(reaction.rate == 2.0);

  REQUIRE(reaction.inputs.terms().size() == 1);
  CHECK(reaction.inputs.terms()[0].reactant.id == A.id);

  REQUIRE(reaction.products.terms().size() == 2);
  CHECK(reaction.products.terms()[0].reactant.id == B.id);
  CHECK(reaction.products.terms()[1].reactant.id == C.id);
}