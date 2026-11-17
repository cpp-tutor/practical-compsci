#ifndef TABLE_HPP
#define TABLE_HPP

#include "CoreDB.hpp"
#include "BSTIndex.hpp"

namespace CoreDB {

class Table {
public:
    Table(std::string name, Schema schema)
        : m_name(std::move(name)), m_schema(std::move(schema)) {
        // Automatically index the primary key column
        for (size_t i = 0; i < m_schema.arity(); ++i) {
            if (m_schema.get_column(i).is_primary_key) {
                m_pk_index_col = i;
                m_indices[i] = std::make_unique<BSTIndex>();
            }
        }
    }

    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const Schema& schema() const noexcept { return m_schema; }
    [[nodiscard]] size_t size() const noexcept { return m_active_row_count; }

    void create_index_on(const std::string& column_name) {
        auto col_idx = m_schema.index_of(column_name);
        if (!col_idx) {
            throw std::runtime_error("Column '" + column_name + "' does not exist.");
        }
        size_t idx = *col_idx;
        if (m_indices.contains(idx)) return; // Already indexed

        auto new_index = std::make_unique<BSTIndex>();
        for (size_t r = 0; r < m_rows.size(); ++r) {
            if (m_row_active[r]) {
                new_index->insert(m_rows[r][idx], r);
            }
        }
        m_indices[idx] = std::move(new_index);
    }

    [[nodiscard]] const BSTIndex* get_index(size_t col_idx) const {
        auto it = m_indices.find(col_idx);
        if (it != m_indices.end()) return it->second.get();
        return nullptr;
    }

    // -------------------------------------------------------------------------
    // Transactional Mutations
    // -------------------------------------------------------------------------

    void begin_transaction() {
        m_in_transaction = true;
        m_undo_log.clear();
    }

    void commit() {
        if (!m_in_transaction) return;
        m_undo_log.clear();
        m_in_transaction = false;
    }

    void rollback() {
        if (!m_in_transaction) return;
        
        // Revert mutations in reverse order
        for (auto it = m_undo_log.rbegin(); it != m_undo_log.rend(); ++it) {
            RowID id = it->row_id;
            if (it->type == UndoType::Insert) {
                m_row_active[id] = false;
                --m_active_row_count;
            }
        }
        
        // Rebuild indices to reflect restored state
        for (auto& [col_idx, index_ptr] : m_indices) {
            auto fresh_index = std::make_unique<BSTIndex>();
            for (size_t r = 0; r < m_rows.size(); ++r) {
                if (m_row_active[r]) {
                    fresh_index->insert(m_rows[r][col_idx], r);
                }
            }
            index_ptr = std::move(fresh_index);
        }

        m_undo_log.clear();
        m_in_transaction = false;
    }

    void insert_record(Row row) {
        validate_row(row);

        // Check primary key uniqueness
        if (m_pk_index_col.has_value()) {
            size_t pk_col = *m_pk_index_col;
            const Value& pk_val = row[pk_col];
            auto existing = m_indices[pk_col]->find_exact(pk_val);
            if (!existing.empty()) {
                throw std::runtime_error("Constraint Violation: Duplicate Primary Key value " + 
                                         [pk_val]() { 
                                             std::ostringstream ss; 
                                             ss << pk_val; 
                                             return ss.str(); 
                                         }());
            }
        }

        RowID new_id = m_rows.size();
        m_rows.push_back(std::move(row));
        m_row_active.push_back(true);
        ++m_active_row_count;

        // Update all associated indices
        for (auto& [col_idx, index_ptr] : m_indices) {
            index_ptr->insert(m_rows[new_id][col_idx], new_id);
        }

        if (m_in_transaction) {
            m_undo_log.push_back({UndoType::Insert, new_id});
        }
    }

    [[nodiscard]] const Row& get_row(RowID id) const {
        if (id >= m_rows.size() || !m_row_active[id]) {
            throw std::out_of_range("Invalid RowID.");
        }
        return m_rows[id];
    }

    [[nodiscard]] std::vector<RowID> all_row_ids() const {
        std::vector<RowID> ids;
        ids.reserve(m_active_row_count);
        for (size_t i = 0; i < m_rows.size(); ++i) {
            if (m_row_active[i]) ids.push_back(i);
        }
        return ids;
    }

private:
    enum class UndoType { Insert };
    struct UndoEntry {
        UndoType type;
        RowID row_id;
    };

    std::string m_name;
    Schema m_schema;
    std::vector<Row> m_rows;
    std::vector<bool> m_row_active;
    size_t m_active_row_count{0};

    std::optional<size_t> m_pk_index_col;
    std::map<size_t, std::unique_ptr<BSTIndex>> m_indices;

    bool m_in_transaction{false};
    std::vector<UndoEntry> m_undo_log;

    void validate_row(const Row& row) const {
        if (row.size() != m_schema.arity()) {
            throw std::runtime_error("Schema Mismatch: Expected " + 
                                     std::to_string(m_schema.arity()) + 
                                     " attributes, got " + std::to_string(row.size()));
        }
        for (size_t i = 0; i < m_schema.arity(); ++i) {
            const auto& col = m_schema.get_column(i);
            const auto& val = row[i];

            if (col.is_not_null && val.is_null()) {
                throw std::runtime_error("Constraint Violation: Column '" + 
                                         col.name + "' cannot be NULL.");
            }
            if (!val.is_null() && val.type() != col.type) {
                // Allow safe int-to-float promotion
                if (col.type == DataType::Float && val.type() == DataType::Integer) {
                    continue;
                }
                throw std::runtime_error("Type Mismatch on Column '" + col.name + "'.");
            }
        }
    }
};

} // namespace CoreDB

#endif // TABLE_HPP