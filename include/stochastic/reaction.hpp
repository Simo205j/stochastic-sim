#pragma once

#include <cstddef>
#include <utility>
#include <vector>
#include "reaction_visitor.hpp"

namespace stochastic
{

  using ReactantId = std::size_t;

  struct Reactant
  {
    ReactantId id{};
  };

  struct Term
  {
    Reactant reactant;
    std::size_t count{1};
  };

  class ReactionSide
  {
  public:
    ReactionSide() = default;

    explicit ReactionSide(Reactant reactant) { add(reactant); }

    void add(Reactant reactant)
    {
      terms_.push_back(Term{.reactant = reactant, .count = 1});
    }

    [[nodiscard]] const std::vector<Term> &terms() const { return terms_; }

  private:
    std::vector<Term> terms_{};
  };

  struct ReactionBuilder
  {
    ReactionSide inputs;
    double rate{};
  };

  struct Reaction
  {
    ReactionSide inputs;
    double rate{};
    ReactionSide products;

    // R2: Enables the visitor pattern for operations such as printing
    // the reaction network in DOT graph format.
    void accept(ReactionVisitor &visitor) const
    {
      visitor.visit(*this);
    }
  };

  // R1: Operator overloads for writing reaction rules directly in C++ code.
  // Example: A + C >> lambda >>= B + C.
  inline ReactionSide operator+(Reactant lhs, Reactant rhs)
  {
    auto side = ReactionSide{lhs};
    side.add(rhs);
    return side;
  }

  inline ReactionSide operator+(ReactionSide lhs, Reactant rhs)
  {
    lhs.add(rhs);
    return lhs;
  }

  inline ReactionSide operator+(Reactant lhs, ReactionSide rhs)
  {
    auto side = ReactionSide{lhs};

    for (const auto &term : rhs.terms())
    {
      for (std::size_t i = 0; i < term.count; ++i)
      {
        side.add(term.reactant);
      }
    }

    return side;
  }

  // R1: Stores the input side and reaction rate before the product side
  // is attached by operator>>=.
  inline ReactionBuilder operator>>(ReactionSide inputs, double rate)
  {
    return ReactionBuilder{
        .inputs = std::move(inputs),
        .rate = rate,
    };
  }

  inline ReactionBuilder operator>>(Reactant input, double rate)
  {
    return ReactionSide{input} >> rate;
  }

  // R1: Completes a reaction rule by connecting inputs, rate, and products.
  inline Reaction operator>>=(ReactionBuilder builder, ReactionSide products)
  {
    return Reaction{
        .inputs = std::move(builder.inputs),
        .rate = builder.rate,
        .products = std::move(products),
    };
  }

  inline Reaction operator>>=(ReactionBuilder builder, Reactant product)
  {
    return std::move(builder) >>= ReactionSide{product};
  }

} // namespace stochastic