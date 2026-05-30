#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace stochastic
{

    struct SymbolNotFound : std::logic_error
    {
        SymbolNotFound()
            : std::logic_error{"symbol not found"}
        {
        }
    };

    struct SymbolAlreadyDefined : std::logic_error
    {
        SymbolAlreadyDefined()
            : std::logic_error{"symbol already defined"}
        {
        }
    };

    template <typename Key, typename Value, typename Compare = std::less<Key>>
    class SymbolTable
    {
        static_assert(
            std::is_invocable_r_v<bool, Compare, const Key &, const Key &>,
            "SymbolTable requires a key type that can be compared by the chosen comparator");

    public:
        void add(Key key, Value value)
        {
            if (symbols_.find(key) != symbols_.end())
            {
                throw SymbolAlreadyDefined{};
            }

            symbols_.emplace(std::move(key), std::move(value));
        }

        [[nodiscard]] const Value &lookup(const Key &key) const
        {
            const auto it = symbols_.find(key);

            if (it == symbols_.end())
            {
                throw SymbolNotFound{};
            }

            return it->second;
        }

        [[nodiscard]] bool contains(const Key &key) const
        {
            return symbols_.find(key) != symbols_.end();
        }

        [[nodiscard]] std::size_t size() const
        {
            return symbols_.size();
        }

    private:
        std::map<Key, Value, Compare> symbols_{};
    };

} // namespace stochastic