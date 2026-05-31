#pragma once

#include "reaction.hpp"
#include "reaction_visitor.hpp"

#include <map>
#include <ostream>
#include <set>
#include <string>
#include <utility>

namespace stochastic
{

    // R2: Visitor implementation that prints a reaction network in DOT graph
    // format, which can be rendered by Graphviz.
    struct DotPrinter : ReactionVisitor
    {
        explicit DotPrinter(std::ostream &out, std::map<int, std::string> labels = {})
            : out{out}, labels{std::move(labels)}
        {
            out << "digraph reaction_network {\n";
        }

        ~DotPrinter() noexcept override
        {
            out << "}\n";
        }

        // R2: Each visited reaction is emitted as a graph node with edges from
        // input reactants to the reaction and from the reaction to its products.
        void visit(const Reaction &reaction) override
        {
            const auto reaction_id = next_reaction_id++;

            out << "  r" << reaction_id
                << " [label=\"" << reaction.rate << "\", shape=oval];\n";

            for (const auto &term : reaction.inputs.terms())
            {
                print_reactant(term.reactant.id);
                out << "  x" << term.reactant.id << " -> r" << reaction_id << ";\n";
            }

            for (const auto &term : reaction.products.terms())
            {
                print_reactant(term.reactant.id);
                out << "  r" << reaction_id << " -> x" << term.reactant.id << ";\n";
            }
        }

    private:
        void print_reactant(int id)
        {
            if (!printed_reactants.insert(id).second)
            {
                return;
            }

            const auto it = labels.find(id);
            const auto label = it == labels.end()
                                   ? "x" + std::to_string(id)
                                   : it->second;

            out << "  x" << id
                << " [label=\"" << label << "\", shape=box];\n";
        }

        std::ostream &out;
        std::map<int, std::string> labels{};
        int next_reaction_id{0};
        std::set<int> printed_reactants{};
    };

} // namespace stochastic