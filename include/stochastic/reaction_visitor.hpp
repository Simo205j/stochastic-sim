#pragma once

namespace stochastic {

struct Reaction;

struct ReactionVisitor {
    virtual ~ReactionVisitor() noexcept = default;

    virtual void visit(const Reaction& reaction) = 0;
};

} // namespace stochastic