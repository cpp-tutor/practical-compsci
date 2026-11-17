#ifndef BST_INDEX_HPP
#define BST_INDEX_HPP

#include "CoreDB.hpp"

namespace CoreDB {

class BSTIndex {
private:
    struct Node {
        Value key;
        std::vector<RowID> row_ids;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        int height{1};

        explicit Node(Value k, RowID id) : key(std::move(k)) {
            row_ids.push_back(id);
        }
    };

    std::unique_ptr<Node> m_root;

    int get_height(const Node* n) const noexcept {
        return n ? n->height : 0;
    }

    int balance_factor(const Node* n) const noexcept {
        return n ? get_height(n->left.get()) - get_height(n->right.get()) : 0;
    }

    void update_height(Node* n) noexcept {
        if (n) {
            n->height = 1 + std::max(get_height(n->left.get()), get_height(n->right.get()));
        }
    }

    std::unique_ptr<Node> rotate_right(std::unique_ptr<Node> y) {
        auto x = std::move(y->left);
        y->left = std::move(x->right);
        update_height(y.get());
        x->right = std::move(y);
        update_height(x.get());
        return x;
    }

    std::unique_ptr<Node> rotate_left(std::unique_ptr<Node> x) {
        auto y = std::move(x->right);
        x->right = std::move(y->left);
        update_height(x.get());
        y->left = std::move(x);
        update_height(y.get());
        return y;
    }

    std::unique_ptr<Node> insert_impl(std::unique_ptr<Node> node, const Value& key, RowID id) {
        if (!node) {
            return std::make_unique<Node>(key, id);
        }

        if (key == node->key) {
            node->row_ids.push_back(id);
            return node;
        }

        if (key < node->key) {
            node->left = insert_impl(std::move(node->left), key, id);
        } else {
            node->right = insert_impl(std::move(node->right), key, id);
        }

        update_height(node.get());
        int balance = balance_factor(node.get());

        // AVL Rebalancing
        if (balance > 1 && key < node->left->key) {
            return rotate_right(std::move(node));
        }
        if (balance < -1 && key > node->right->key) {
            return rotate_left(std::move(node));
        }
        if (balance > 1 && key > node->left->key) {
            node->left = rotate_left(std::move(node->left));
            return rotate_right(std::move(node));
        }
        if (balance < -1 && key < node->right->key) {
            node->right = rotate_right(std::move(node->right));
            return rotate_left(std::move(node));
        }

        return node;
    }

    void find_range_impl(const Node* node, const Value& low, const Value& high, 
                         std::vector<RowID>& results) const {
        if (!node) return;

        if (low < node->key) {
            find_range_impl(node->left.get(), low, high, results);
        }
        if (node->key >= low && node->key <= high) {
            results.insert(results.end(), node->row_ids.begin(), node->row_ids.end());
        }
        if (high > node->key) {
            find_range_impl(node->right.get(), low, high, results);
        }
    }

public:
    BSTIndex() = default;

    void insert(const Value& key, RowID id) {
        m_root = insert_impl(std::move(m_root), key, id);
    }

    [[nodiscard]] std::vector<RowID> find_exact(const Value& key) const {
        const Node* curr = m_root.get();
        while (curr) {
            if (key == curr->key) {
                return curr->row_ids;
            }
            if (key < curr->key) {
                curr = curr->left.get();
            } else {
                curr = curr->right.get();
            }
        }
        return {};
    }

    [[nodiscard]] std::vector<RowID> find_range(const Value& low, const Value& high) const {
        std::vector<RowID> results;
        find_range_impl(m_root.get(), low, high, results);
        return results;
    }
};

} // namespace CoreDB

#endif // BST_INDEX_HPP