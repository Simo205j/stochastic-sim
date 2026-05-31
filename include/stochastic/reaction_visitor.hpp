#pragma once

namespace stochastic
{

    struct Reaction;

    // R2: Visitor interface used by reaction network printers and other
    // operations that should be separated from the Reaction data structure.
    struct ReactionVisitor
    {
        virtual ~ReactionVisitor() noexcept = default;

        virtual void visit(const Reaction &reaction) = 0;
    };

} // namespace stochastic