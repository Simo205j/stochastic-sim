#include <catch2/catch_test_macros.hpp>

#include <stochastic/reaction.hpp>

TEST_CASE("reaction rule can be typeset with operators") {
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
