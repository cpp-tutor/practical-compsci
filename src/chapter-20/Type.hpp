#ifndef TYPE_HPP
#define TYPE_HPP

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <algorithm>

namespace Faberge {

enum class PrimitiveKind {
    Int,
    Float,
    String,
    Bool,
    Void,
    Error // Sentinel for poisonous error propagation
};

class Type {
public:
    enum class Kind { Primitive, Function };

    Type() : m_kind(Kind::Primitive), m_primitive_kind(PrimitiveKind::Void) {}
    explicit Type(PrimitiveKind pk) : m_kind(Kind::Primitive), m_primitive_kind(pk) {}

    static Type make_int()    { return Type(PrimitiveKind::Int); }
    static Type make_float()  { return Type(PrimitiveKind::Float); }
    static Type make_string() { return Type(PrimitiveKind::String); }
    static Type make_bool()   { return Type(PrimitiveKind::Bool); }
    static Type make_void()   { return Type(PrimitiveKind::Void); }
    static Type make_error()  { return Type(PrimitiveKind::Error); }

    static Type make_function(std::vector<Type> param_types, Type return_type) {
        Type t(Kind::Function);
        t.m_param_types = std::move(param_types);
        t.m_return_type = std::make_shared<Type>(std::move(return_type));
        return t;
    }

    [[nodiscard]] Kind kind() const noexcept { return m_kind; }
    [[nodiscard]] PrimitiveKind primitive_kind() const noexcept { return m_primitive_kind; }
    [[nodiscard]] bool is_error() const noexcept { 
        return m_kind == Kind::Primitive && m_primitive_kind == PrimitiveKind::Error; 
    }
    [[nodiscard]] bool is_numeric() const noexcept {
        return m_kind == Kind::Primitive && 
               (m_primitive_kind == PrimitiveKind::Int || m_primitive_kind == PrimitiveKind::Float);
    }

    [[nodiscard]] const std::vector<Type>& param_types() const noexcept { return m_param_types; }
    [[nodiscard]] const Type& return_type() const { return *m_return_type; }

    // Strict structural type equality
    bool operator==(const Type& other) const {
        if (is_error() || other.is_error()) return true; // Suppress cascading error spam
        if (m_kind != other.m_kind) return false;

        if (m_kind == Kind::Primitive) {
            return m_primitive_kind == other.m_primitive_kind;
        }

        if (m_param_types.size() != other.m_param_types.size()) return false;
        for (size_t i = 0; i < m_param_types.size(); ++i) {
            if (m_param_types[i] != other.m_param_types[i]) return false;
        }
        return *m_return_type == *other.m_return_type;
    }

    bool operator!=(const Type& other) const { return !(*this == other); }

    [[nodiscard]] std::string to_string() const {
        if (m_kind == Kind::Primitive) {
            switch (m_primitive_kind) {
                case PrimitiveKind::Int:    return "Int";
                case PrimitiveKind::Float:  return "Float";
                case PrimitiveKind::String: return "String";
                case PrimitiveKind::Bool:   return "Bool";
                case PrimitiveKind::Void:   return "Void";
                case PrimitiveKind::Error:  return "<type-error>";
            }
        }

        std::ostringstream ss;
        ss << "(";
        for (size_t i = 0; i < m_param_types.size(); ++i) {
            ss << m_param_types[i].to_string();
            if (i + 1 < m_param_types.size()) ss << ", ";
        }
        ss << " -> " << m_return_type->to_string() << ")";
        return ss.str();
    }

private:
    explicit Type(Kind k) : m_kind(k) {}
    Kind m_kind;
    PrimitiveKind m_primitive_kind{PrimitiveKind::Void};
    std::vector<Type> m_param_types;
    std::shared_ptr<Type> m_return_type;
};

} // namespace Faberge

#endif // TYPE_HPP