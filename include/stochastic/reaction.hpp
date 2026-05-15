#pragma once

#include <cstddef>
#include <utility>
#include <vector>

namespace stochastic {

using ReactantId = std::size_t;

struct Reactant {
  ReactantId id{};
};

struct Term {
  Reactant reactant;
  std::size_t count{1};
};

class ReactionSide {
public:
  ReactionSide() = default;

  explicit ReactionSide(Reactant reactant) { add(reactant); }

  void add(Reactant reactant) {
    terms_.push_back(Term{.reactant = reactant, .count = 1});
  }

  [[nodiscard]] const std::vector<Term> &terms() const { return terms_; }

private:
  std::vector<Term> terms_{};
};

struct ReactionBuilder {
  ReactionSide inputs;
  double rate{};
};

struct Reaction {
  ReactionSide inputs;
  double rate{};
  ReactionSide products;
};

inline ReactionSide operator+(Reactant lhs, Reactant rhs) {
  auto side = ReactionSide{lhs};
  side.add(rhs);
  return side;
}

inline ReactionSide operator+(ReactionSide lhs, Reactant rhs) {
  lhs.add(rhs);
  return lhs;
}

inline ReactionSide operator+(Reactant lhs, ReactionSide rhs) {
  auto side = ReactionSide{lhs};

  for (const auto &term : rhs.terms()) {
    for (std::size_t i = 0; i < term.count; ++i) {
      side.add(term.reactant);
    }
  }

  return side;
}

inline ReactionBuilder operator>>(ReactionSide inputs, double rate) {
  return ReactionBuilder{
      .inputs = std::move(inputs),
      .rate = rate,
  };
}

inline ReactionBuilder operator>>(Reactant input, double rate) {
  return ReactionSide{input} >> rate;
}

inline Reaction operator>>=(ReactionBuilder builder, ReactionSide products) {
  return Reaction{
      .inputs = std::move(builder.inputs),
      .rate = builder.rate,
      .products = std::move(products),
  };
}

inline Reaction operator>>=(ReactionBuilder builder, Reactant product) {
  return std::move(builder) >>= ReactionSide{product};
}

} // namespace stochastic
