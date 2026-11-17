#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include "Type.hpp"
#include "../chapter-18/SourceLocation.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <optional>
#include <stdexcept>

namespace Faberge {

struct Symbol {
    std::string name;
    Type type;
    SourceLocation location;
    bool is_function{false};
};

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
        : m_parent(std::move(parent)) {}

    static std::shared_ptr<Environment> create(std::shared_ptr<Environment> parent = nullptr) {
        return std::make_shared<Environment>(std::move(parent));
    }

    [[nodiscard]] std::shared_ptr<Environment> parent() const noexcept { return m_parent; }

    bool define(std::string name, Type type, SourceLocation loc, bool is_fn = false) {
        if (m_bindings.contains(name)) {
            return false; // Duplicate symbol declaration in the EXACT same scope
        }
        m_bindings.emplace(name, Symbol{name, std::move(type), std::move(loc), is_fn});
        return true;
    }

    [[nodiscard]] std::optional<Symbol> lookup(const std::string& name) const {
        auto it = m_bindings.find(name);
        if (it != m_bindings.end()) {
            return it->second;
        }
        if (m_parent) {
            return m_parent->lookup(name);
        }
        return std::nullopt;
    }

private:
    std::shared_ptr<Environment> m_parent;
    std::unordered_map<std::string, Symbol> m_bindings;
};

} // namespace Faberge

#endif // SYMBOL_TABLE_HPP