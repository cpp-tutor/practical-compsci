#ifndef DYNAMIC_ARRAY_HPP
#define DYNAMIC_ARRAY_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <format>
#include <iostream>

namespace sysmem {

template <typename T>
class DynamicArray {
public:
    // =========================================================================
    // Construction and Destruction (RAII)
    // =========================================================================
    
    // Default constructor: creates an empty array with zero initial capacity
    DynamicArray() noexcept : m_data(nullptr), m_size(0), m_capacity(0) {}

    // Sized constructor: allocates capacity up front
    explicit DynamicArray(std::size_t initial_capacity)
        : m_data(initial_capacity > 0 ? new T[initial_capacity] : nullptr),
          m_size(0),
          m_capacity(initial_capacity) {}

    // Destructor: guarantees clean reclamation of heap buffer
    ~DynamicArray() noexcept {
        release();
    }

    // =========================================================================
    // The Rule of Five: Deep Copy and Move Semantics
    // =========================================================================

    // 1. Copy Constructor: Deep-copies elements from other
    DynamicArray(const DynamicArray& other)
        : m_data(other.m_capacity > 0 ? new T[other.m_capacity] : nullptr),
          m_size(other.m_size),
          m_capacity(other.m_capacity) 
    {
        for (std::size_t i = 0; i < m_size; ++i) {
            m_data[i] = other.m_data[i];
        }
    }

    // 2. Copy Assignment Operator: Copy-and-Swap Idiom for strong exception safety
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            DynamicArray temp(other);
            swap(temp);
        }
        return *this;
    }

    // 3. Move Constructor: Steals the pointer from temporary, leaving it empty
    DynamicArray(DynamicArray&& other) noexcept
        : m_data(std::exchange(other.m_data, nullptr)),
          m_size(std::exchange(other.m_size, 0)),
          m_capacity(std::exchange(other.m_capacity, 0)) {}

    // 4. Move Assignment Operator
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            release();
            m_data = std::exchange(other.m_data, nullptr);
            m_size = std::exchange(other.m_size, 0);
            m_capacity = std::exchange(other.m_capacity, 0);
        }
        return *this;
    }

    // =========================================================================
    // Element Access & Pointer Operations
    // =========================================================================

    [[nodiscard]] std::size_t size() const noexcept { return m_size; }
    [[nodiscard]] std::size_t capacity() const noexcept { return m_capacity; }
    [[nodiscard]] bool empty() const noexcept { return m_size == 0; }

    // Direct unchecked indexing via pointer arithmetic
    T& operator[](std::size_t index) noexcept {
        return *(m_data + index);
    }

    const T& operator[](std::size_t index) const noexcept {
        return *(m_data + index);
    }

    // Checked bounds-safe access
    T& at(std::size_t index) {
        if (index >= m_size) {
            throw std::out_of_range(std::format("DynamicArray index {} out of bounds (size: {})", index, m_size));
        }
        return *(m_data + index);
    }

    const T& at(std::size_t index) const {
        if (index >= m_size) {
            throw std::out_of_range(std::format("DynamicArray index {} out of bounds (size: {})", index, m_size));
        }
        return *(m_data + index);
    }

    // Direct access to the raw underlying pointer
    [[nodiscard]] T* data() noexcept { return m_data; }
    [[nodiscard]] const T* data() const noexcept { return m_data; }

    // =========================================================================
    // Growth and Memory Modification
    // =========================================================================

    void push_back(const T& value) {
        if (m_size == m_capacity) {
            grow(m_capacity == 0 ? 2 : m_capacity * 2);
        }
        m_data[m_size++] = value;
    }

    void push_back(T&& value) {
        if (m_size == m_capacity) {
            grow(m_capacity == 0 ? 2 : m_capacity * 2);
        }
        m_data[m_size++] = std::move(value);
    }

    void reserve(std::size_t new_capacity) {
        if (new_capacity > m_capacity) {
            grow(new_capacity);
        }
    }

    void swap(DynamicArray& other) noexcept {
        std::swap(m_data, other.m_data);
        std::swap(m_size, other.m_size);
        std::swap(m_capacity, other.m_capacity);
    }

    // Pointer-based Iterators (Enables range-based for loops: for(auto& x : arr))
    using iterator = T*;
    using const_iterator = const T*;

    iterator begin() noexcept { return m_data; }
    iterator end() noexcept { return m_data + m_size; }
    const_iterator begin() const noexcept { return m_data; }
    const_iterator end() const noexcept { return m_data + m_size; }

private:
    void grow(std::size_t target_capacity) {
        T* new_buffer = new T[target_capacity];
        
        // Move existing elements over to new memory allocation
        for (std::size_t i = 0; i < m_size; ++i) {
            new_buffer[i] = std::move(m_data[i]);
        }

        // Deallocate old heap memory and repoint
        delete[] m_data;
        m_data = new_buffer;
        m_capacity = target_capacity;
    }

    void release() noexcept {
        delete[] m_data;
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
    }

    T*          m_data{nullptr};
    std::size_t m_size{0};
    std::size_t m_capacity{0};
};

} // namespace sysmem

#endif // DYNAMIC_ARRAY_HPP