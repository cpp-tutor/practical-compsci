#ifndef DATABASE_HPP
#define DATABASE_HPP

#include "CoreDB.hpp"
#include "BSTIndex.hpp"
#include "Table.hpp"
#include "Lexer.hpp"

namespace CoreDB {

enum class ComparisonOp {
    Equal,
    NotEqual,
    LessThan,
    LessOrEqual,
    GreaterThan,
    GreaterOrEqual
};

struct Predicate {
    std::string column_name;
    ComparisonOp op;
    Value value;

    [[nodiscard]] bool evaluate(const Value& cell) const {
        switch (op) {
            case ComparisonOp::Equal:          return cell == value;
            case ComparisonOp::NotEqual:       return cell != value;
            case ComparisonOp::LessThan:       return cell < value;
            case ComparisonOp::LessOrEqual:    return cell <= value;
            case ComparisonOp::GreaterThan:    return cell > value;
            case ComparisonOp::GreaterOrEqual: return cell >= value;
        }
        return false;
    }
};

// -----------------------------------------------------------------------------
// Relational Algebra Execution Engine (Volcano Iterator Pattern)
// -----------------------------------------------------------------------------

class ExecutionNode {
public:
    virtual ~ExecutionNode() = default;
    virtual void open() = 0;
    virtual std::optional<Row> next() = 0;
    virtual const Schema& output_schema() const = 0;
};

// 1. Scan Node with Index Acceleration
class PhysicalScanNode : public ExecutionNode {
public:
    PhysicalScanNode(const Table& table, std::optional<Predicate> pred = std::nullopt)
        : m_table(table), m_predicate(std::move(pred)) {}

    void open() override {
        m_cursor = 0;
        m_matching_ids.clear();

        // Check if an index can accelerate this predicate
        if (m_predicate.has_value()) {
            auto col_idx = m_table.schema().index_of(m_predicate->column_name);
            if (col_idx.has_value()) {
                const BSTIndex* index = m_table.get_index(*col_idx);
                if (index && m_predicate->op == ComparisonOp::Equal) {
                    // Logarithmic O(log N) lookup
                    m_matching_ids = index->find_exact(m_predicate->value);
                    m_uses_index = true;
                    return;
                }
            }
        }

        // Fallback: Full Table Scan O(N)
        m_matching_ids = m_table.all_row_ids();
        m_uses_index = false;
    }

    std::optional<Row> next() override {
        while (m_cursor < m_matching_ids.size()) {
            RowID id = m_matching_ids[m_cursor++];
            const Row& candidate = m_table.get_row(id);

            // If we used an exact index, the predicate is guaranteed to match
            if (m_uses_index) {
                return candidate;
            }

            // Otherwise, evaluate the selection predicate manually
            if (!m_predicate.has_value()) {
                return candidate;
            }

            auto col_idx = *m_table.schema().index_of(m_predicate->column_name);
            if (m_predicate->evaluate(candidate[col_idx])) {
                return candidate;
            }
        }
        return std::nullopt;
    }

    const Schema& output_schema() const override {
        return m_table.schema();
    }

    [[nodiscard]] bool used_index() const noexcept { return m_uses_index; }

private:
    const Table& m_table;
    std::optional<Predicate> m_predicate;
    std::vector<RowID> m_matching_ids;
    size_t m_cursor{0};
    bool m_uses_index{false};
};

// 2. Projection Node (π)
class ProjectionNode : public ExecutionNode {
public:
    ProjectionNode(std::unique_ptr<ExecutionNode> child, std::vector<std::string> projected_cols)
        : m_child(std::move(child)), m_requested_names(std::move(projected_cols)) {
        
        const auto& in_schema = m_child->output_schema();
        std::vector<Column> out_cols;

        if (m_requested_names.size() == 1 && m_requested_names[0] == "*") {
            output_m_schema = in_schema;
            for (size_t i = 0; i < in_schema.arity(); ++i) {
                project_m_indices.push_back(i);
            }
        } else {
            for (const auto& name : m_requested_names) {
                auto idx = in_schema.index_of(name);
                if (!idx) throw std::runtime_error("Column '" + name + "' does not exist in child relation.");
                project_m_indices.push_back(*idx);
                out_cols.push_back(in_schema.get_column(*idx));
            }
            output_m_schema = Schema(out_cols);
        }
    }

    void open() override {
        m_child->open();
    }

    std::optional<Row> next() override {
        auto child_row = m_child->next();
        if (!child_row) return std::nullopt;

        Row projected;
        projected.reserve(project_m_indices.size());
        for (size_t idx : project_m_indices) {
            projected.push_back((*child_row)[idx]);
        }
        return projected;
    }

    const Schema& output_schema() const override {
        return output_m_schema;
    }

private:
    std::unique_ptr<ExecutionNode> m_child;
    std::vector<std::string> m_requested_names;
    std::vector<size_t> project_m_indices;
    Schema output_m_schema;
};

// -----------------------------------------------------------------------------
// Database Instance: Catalog and Engine Coordination
// -----------------------------------------------------------------------------

class Database {
public:
    void execute_ddl(std::string_view sql) {
        Lexer lexer{std::string(sql)};
        auto tokens = lexer.tokenise();
        size_t pos = 0;

        if (match(tokens, pos, TokenType::Keyword_CREATE)) {
            consume(tokens, pos, TokenType::Keyword_TABLE, "Expected 'TABLE'");
            Token m_nametok = consume(tokens, pos, TokenType::Identifier, "Expected table name");
            consume(tokens, pos, TokenType::LeftParen, "Expected '('");

            std::vector<Column> cols;
            while (true) {
                Token col_name = consume(tokens, pos, TokenType::Identifier, "Expected column name");
                Token col_type = advance_tok(tokens, pos);
                DataType dtype;
                if (col_type.type == TokenType::Type_INT) dtype = DataType::Integer;
                else if (col_type.type == TokenType::Type_FLOAT) dtype = DataType::Float;
                else if (col_type.type == TokenType::Type_TEXT) dtype = DataType::Text;
                else throw std::runtime_error("Invalid data type for column " + col_name.text);

                bool is_pk = false;
                bool is_not_null = false;

                while (pos < tokens.size() && 
                       (tokens[pos].type == TokenType::Keyword_PRIMARY || 
                        tokens[pos].type == TokenType::Keyword_NOT)) {
                    if (match(tokens, pos, TokenType::Keyword_PRIMARY)) {
                        consume(tokens, pos, TokenType::Keyword_KEY, "Expected 'KEY' after 'PRIMARY'");
                        is_pk = true;
                        is_not_null = true;
                    } else if (match(tokens, pos, TokenType::Keyword_NOT)) {
                        consume(tokens, pos, TokenType::Keyword_NULL, "Expected 'NULL' after 'NOT'");
                        is_not_null = true;
                    }
                }

                cols.push_back({col_name.text, dtype, is_pk, is_not_null});

                if (match(tokens, pos, TokenType::Comma)) continue;
                if (match(tokens, pos, TokenType::RightParen)) break;
                throw std::runtime_error("Expected ',' or ')' in column definitions.");
            }

            m_tables.emplace(m_nametok.text, std::make_unique<Table>(m_nametok.text, Schema(cols)));
        } else {
            throw std::runtime_error("Unsupported DDL command.");
        }
    }

    void execute_insert(std::string_view sql) {
        Lexer lexer{std::string(sql)};
        auto tokens = lexer.tokenise();
        size_t pos = 0;

        consume(tokens, pos, TokenType::Keyword_INSERT, "Expected 'INSERT'");
        consume(tokens, pos, TokenType::Keyword_INTO, "Expected 'INTO'");
        Token tbl_tok = consume(tokens, pos, TokenType::Identifier, "Expected table name");
        consume(tokens, pos, TokenType::Keyword_VALUES, "Expected 'VALUES'");
        consume(tokens, pos, TokenType::LeftParen, "Expected '('");

        auto it = m_tables.find(tbl_tok.text);
        if (it == m_tables.end()) {
            throw std::runtime_error("Table '" + tbl_tok.text + "' does not exist.");
        }

        Row row;
        while (true) {
            Token val_tok = advance_tok(tokens, pos);
            if (val_tok.type == TokenType::Literal_Integer) {
                row.emplace_back(static_cast<int64_t>(std::stoll(val_tok.text)));
            } else if (val_tok.type == TokenType::Literal_Float) {
                row.emplace_back(std::stod(val_tok.text));
            } else if (val_tok.type == TokenType::Literal_String) {
                row.emplace_back(val_tok.text);
            } else if (val_tok.type == TokenType::Keyword_NULL) {
                row.emplace_back(Value());
            } else {
                throw std::runtime_error("Unexpected token in VALUES clause: " + val_tok.text);
            }

            if (match(tokens, pos, TokenType::Comma)) continue;
            if (match(tokens, pos, TokenType::RightParen)) break;
            throw std::runtime_error("Expected ',' or ')' in VALUES clause.");
        }

        it->second->insert_record(std::move(row));
    }

    void print_query(std::string_view sql) {
        Lexer lexer{std::string(sql)};
        auto tokens = lexer.tokenise();
        size_t pos = 0;

        consume(tokens, pos, TokenType::Keyword_SELECT, "Expected 'SELECT'");

        std::vector<std::string> projections;
        if (match(tokens, pos, TokenType::Asterisk)) {
            projections.push_back("*");
        } else {
            while (true) {
                Token col = consume(tokens, pos, TokenType::Identifier, "Expected column name");
                projections.push_back(col.text);
                if (!match(tokens, pos, TokenType::Comma)) break;
            }
        }

        consume(tokens, pos, TokenType::Keyword_FROM, "Expected 'FROM'");
        Token tbl_tok = consume(tokens, pos, TokenType::Identifier, "Expected table name");

        auto it = m_tables.find(tbl_tok.text);
        if (it == m_tables.end()) {
            throw std::runtime_error("Table '" + tbl_tok.text + "' does not exist.");
        }
        Table& table = *(it->second);

        std::optional<Predicate> predicate;
        if (match(tokens, pos, TokenType::Keyword_WHERE)) {
            Token col = consume(tokens, pos, TokenType::Identifier, "Expected column name in WHERE");
            Token op_tok = advance_tok(tokens, pos);
            Token val_tok = advance_tok(tokens, pos);

            ComparisonOp op;
            if (op_tok.type == TokenType::Op_Equal) op = ComparisonOp::Equal;
            else if (op_tok.type == TokenType::Op_NotEqual) op = ComparisonOp::NotEqual;
            else if (op_tok.type == TokenType::Op_Less) op = ComparisonOp::LessThan;
            else if (op_tok.type == TokenType::Op_LessEqual) op = ComparisonOp::LessOrEqual;
            else if (op_tok.type == TokenType::Op_Greater) op = ComparisonOp::GreaterThan;
            else if (op_tok.type == TokenType::Op_GreaterEqual) op = ComparisonOp::GreaterOrEqual;
            else throw std::runtime_error("Unsupported comparison operator: " + op_tok.text);

            Value rhs;
            if (val_tok.type == TokenType::Literal_Integer) rhs = Value(static_cast<int64_t>(std::stoll(val_tok.text)));
            else if (val_tok.type == TokenType::Literal_Float) rhs = Value(std::stod(val_tok.text));
            else if (val_tok.type == TokenType::Literal_String) rhs = Value(val_tok.text);
            else throw std::runtime_error("Invalid literal in WHERE predicate.");

            predicate = Predicate{col.text, op, rhs};
        }

        // Construct Relational Algebra Pipeline
        auto scan_node = std::make_unique<PhysicalScanNode>(table, predicate);
        auto proj_node = std::make_unique<ProjectionNode>(std::move(scan_node), projections);

        // Execute Plan
        proj_node->open();
        const auto& out_schema = proj_node->output_schema();

        // Print Pretty Table Header
        std::cout << "\n+";
        for (const auto& col : out_schema.columns()) {
            std::cout << "----------------------+";
        }
        std::cout << "\n|";
        for (const auto& col : out_schema.columns()) {
            std::cout << " " << std::setw(20) << std::left << col.name << " |";
        }
        std::cout << "\n+";
        for (const auto& col : out_schema.columns()) {
            std::cout << "----------------------+";
        }
        std::cout << "\n";

        size_t row_count = 0;
        while (auto row = proj_node->next()) {
            ++row_count;
            std::cout << "|";
            for (const auto& val : *row) {
                std::ostringstream ss;
                ss << val;
                std::cout << " " << std::setw(20) << std::left << ss.str() << " |";
            }
            std::cout << "\n";
        }

        std::cout << "+";
        for (const auto& col : out_schema.columns()) {
            std::cout << "----------------------+";
        }
        std::cout << "\n(" << row_count << " rows returned)\n\n";
    }

    [[nodiscard]] Table& get_table(const std::string& name) {
        auto it = m_tables.find(name);
        if (it == m_tables.end()) throw std::runtime_error("Table not found: " + name);
        return *(it->second);
    }

private:
    std::map<std::string, std::unique_ptr<Table>> m_tables;

    bool match(const std::vector<Token>& tokens, size_t& pos, TokenType type) {
        if (pos < tokens.size() && tokens[pos].type == type) {
            ++pos;
            return true;
        }
        return false;
    }

    Token advance_tok(const std::vector<Token>& tokens, size_t& pos) {
        if (pos >= tokens.size()) throw std::runtime_error("Unexpected end of tokens.");
        return tokens[pos++];
    }

    Token consume(const std::vector<Token>& tokens, size_t& pos, TokenType expected, std::string_view err) {
        if (pos < tokens.size() && tokens[pos].type == expected) {
            return tokens[pos++];
        }
        throw std::runtime_error(std::string(err) + " at token '" + 
                                 (pos < tokens.size() ? tokens[pos].text : "EOF") + "'");
    }
};

} // namespace CoreDB

#endif // DATABASE_HPP