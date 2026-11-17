#ifndef CONTIGUOUS_MATRIX_HPP
#define CONTIGUOUS_MATRIX_HPP

#include <cstddef>
#include <vector>
#include <memory>
#include <stdexcept>
#include <format>
#include <span>

namespace sysmem {

template <typename T>
class ContiguousMatrix {
public:
    ContiguousMatrix(std::size_t rows, std::size_t cols, const T& initial_val = T{})
        : m_rows(rows),
          m_cols(cols),
          m_data(std::make_unique<T[]>(rows * cols))
    {
        if (rows == 0 || cols == 0) {
            throw std::invalid_argument("Matrix dimensions must be non-zero.");
        }
        for (std::size_t i = 0; i < (m_rows * m_cols); ++i) {
            m_data[i] = initial_val;
        }
    }

    // High-speed 2D indexing operator (Row-Major Ordering)
    [[nodiscard]] T& operator()(std::size_t row, std::size_t col) noexcept {
        return m_data[(row * m_cols) + col];
    }

    [[nodiscard]] const T& operator()(std::size_t row, std::size_t col) const noexcept {
        return m_data[(row * m_cols) + col];
    }

    [[nodiscard]] std::size_t rows() const noexcept { return m_rows; }
    [[nodiscard]] std::size_t cols() const noexcept { return m_cols; }

    // Modern C++20 View of a single row via std::span
    [[nodiscard]] std::span<T> get_row_span(std::size_t row) {
        if (row >= m_rows) {
            throw std::out_of_range("Row index out of range.");
        }
        return std::span<T>(&m_data[row * m_cols], m_cols);
    }

    // Direct access to contiguous buffer
    [[nodiscard]] T* raw_data() noexcept { return m_data.get(); }
    [[nodiscard]] const T* raw_data() const noexcept { return m_data.get(); }

private:
    std::size_t           m_rows;
    std::size_t           m_cols;
    std::unique_ptr<T[]>  m_data; // Pure RAII ownership of flat heap buffer
};

} // namespace sysmem

#endif // CONTIGUOUS_MATRIX_HPP