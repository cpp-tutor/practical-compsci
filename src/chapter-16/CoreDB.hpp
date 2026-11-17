#ifndef COREDB_HPP
#define COREDB_HPP

#include <iostream>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <memory>
#include <stdexcept>
#include <functional>
#include <sstream>
#include <map>
#include <algorithm>
#include <cstdint>
#include <string_view>
#include <iomanip>

namespace CoreDB {

// -----------------------------------------------------------------------------
// 1. Primitive Value System
// -----------------------------------------------------------------------------

enum class DataType {
    Integer,
    Float,
    Text,
    Null
};

// Represents a concrete relational value
class Value {
public:
    using VariantType = std::variant<std::monostate, int64_t, double, std::string>;

    Value() : m_data(std::monostate{}) {}
    Value(int64_t v) : m_data(v) {}
    Value(int v) : m_data(static_cast<int64_t>(v)) {}
    Value(double v) : m_data(v) {}
    Value(std::string v) : m_data(std::move(v)) {}
    Value(const char* v) : m_data(std::string(v)) {}

    [[nodiscard]] DataType type() const noexcept {
        return std::visit([](const auto& val) -> DataType {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::monostate>) return DataType::Null;
            else if constexpr (std::is_same_v<T, int64_t>)    return DataType::Integer;
            else if constexpr (std::is_same_v<T, double>)     return DataType::Float;
            else if constexpr (std::is_same_v<T, std::string>)return DataType::Text;
        }, m_data);
    }

    [[nodiscard]] bool is_null() const noexcept {
        return std::holds_alternative<std::monostate>(m_data);
    }

    [[nodiscard]] int64_t as_int() const {
        if (auto p = std::get_if<int64_t>(&m_data)) return *p;
        throw std::runtime_error("Type error: Value is not an Integer.");
    }

    [[nodiscard]] double as_float() const {
        if (auto p = std::get_if<double>(&m_data)) return *p;
        if (auto p = std::get_if<int64_t>(&m_data)) return static_cast<double>(*p);
        throw std::runtime_error("Type error: Value is not numeric.");
    }

    [[nodiscard]] const std::string& as_text() const {
        if (auto p = std::get_if<std::string>(&m_data)) return *p;
        throw std::runtime_error("Type error: Value is not Text.");
    }

    // Equality operator
    bool operator==(const Value& other) const {
        if (type() != other.type()) {
            // Support transparent float-to-int comparison
            if ((type() == DataType::Integer && other.type() == DataType::Float) ||
                (type() == DataType::Float && other.type() == DataType::Integer)) {
                return as_float() == other.as_float();
            }
            return false;
        }
        return m_data == other.m_data;
    }

    // Ordering operator for BST indexing and sorting
    bool operator<(const Value& other) const {
        if (type() != other.type()) {
            if ((type() == DataType::Integer && other.type() == DataType::Float) ||
                (type() == DataType::Float && other.type() == DataType::Integer)) {
                return as_float() < other.as_float();
            }
            return type() < other.type();
        }
        return m_data < other.m_data;
    }

    bool operator<=(const Value& other) const { return *this < other || *this == other; }
    bool operator>(const Value& other) const { return !(*this <= other); }
    bool operator>=(const Value& other) const { return !(*this < other); }
    bool operator!=(const Value& other) const { return !(*this == other); }

    friend std::ostream& operator<<(std::ostream& os, const Value& val) {
        std::visit([&os](const auto& v) {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                os << "NULL";
            } else if constexpr (std::is_same_v<T, std::string>) {
                os << "'" << v << "'";
            } else {
                os << v;
            }
        }, val.m_data);
        return os;
    }

private:
    VariantType m_data;
};

// -----------------------------------------------------------------------------
// 2. Schema Definition and Constraints
// -----------------------------------------------------------------------------

struct Column {
    std::string name;
    DataType type;
    bool is_primary_key{false};
    bool is_not_null{false};
};

class Schema {
public:
    Schema() = default;
    Schema(std::initializer_list<Column> cols) : m_columns(cols) {}
    explicit Schema(std::vector<Column> cols) : m_columns(std::move(cols)) {}

    [[nodiscard]] const std::vector<Column>& columns() const noexcept { return m_columns; }

    [[nodiscard]] std::optional<size_t> index_of(std::string_view column_name) const noexcept {
        for (size_t i = 0; i < m_columns.size(); ++i) {
            if (m_columns[i].name == column_name) return i;
        }
        return std::nullopt;
    }

    [[nodiscard]] const Column& get_column(size_t idx) const {
        if (idx >= m_columns.size()) throw std::out_of_range("Column index out of bounds.");
        return m_columns[idx];
    }

    [[nodiscard]] size_t arity() const noexcept { return m_columns.size(); }

private:
    std::vector<Column> m_columns;
};

using Row = std::vector<Value>;
using RowID = size_t;

} // namespace CoreDB

#endif // COREDB_HPP