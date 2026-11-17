#ifndef CONTRACTED_STACK_HPP
#define CONTRACTED_STACK_HPP

#include "Contracts.hpp"
#include <vector>
#include <cstddef>

namespace proof {

template <typename T>
class ContractedStack {
public:
    explicit ContractedStack(std::size_t max_capacity = 100)
        : m_capacity(max_capacity) 
    {
        CONTRACT_EXPECTS(max_capacity > 0);
        check_invariants();
    }

    void push(T value) {
        check_invariants();
        
        // 1. Precondition
        CONTRACT_EXPECTS(!is_full());
        
        const std::size_t old_size = m_elements.size();

        // 2. State Mutation
        m_elements.push_back(std::move(value));

        // 3. Postcondition
        CONTRACT_ENSURES(m_elements.size() == old_size + 1);
        CONTRACT_ENSURES(m_elements.back() == value);
        
        check_invariants();
    }

    [[nodiscard]] T pop() {
        check_invariants();

        // 1. Precondition
        CONTRACT_EXPECTS(!is_empty());

        const std::size_t old_size = m_elements.size();
        const T old_top = m_elements.back();

        // 2. State Mutation
        T result = std::move(m_elements.back());
        m_elements.pop_back();

        // 3. Postcondition
        CONTRACT_ENSURES(m_elements.size() == old_size - 1);
        CONTRACT_ENSURES(result == old_top);

        check_invariants();
        return result;
    }

    [[nodiscard]] const T& top() const {
        // Precondition
        CONTRACT_EXPECTS(!is_empty());
        return m_elements.back();
    }

    [[nodiscard]] bool is_empty() const noexcept { return m_elements.empty(); }
    [[nodiscard]] bool is_full() const noexcept { return m_elements.size() >= m_capacity; }
    [[nodiscard]] std::size_t size() const noexcept { return m_elements.size(); }
    [[nodiscard]] std::size_t capacity() const noexcept { return m_capacity; }

private:
    void check_invariants() const {
        // Class Invariant 1: Size cannot exceed maximum capacity
        CONTRACT_INVARIANT(m_elements.size() <= m_capacity);
        // Class Invariant 2: Capacity must remain strictly positive
        CONTRACT_INVARIANT(m_capacity > 0);
    }

    std::size_t    m_capacity;
    std::vector<T> m_elements;
};

} // namespace proof

#endif // CONTRACTED_STACK_HPP