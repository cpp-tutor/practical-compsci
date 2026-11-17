#include "DynamicArray.hpp"
#include "ContiguousMatrix.hpp"
#include <iostream>
#include <string>
#include <chrono>

struct Tracer {
    std::string name;
    Tracer() : name("Default") { std::cout << "  [Tracer Constructor: " << name << "]\n"; }
    Tracer(std::string n) : name(std::move(n)) { std::cout << "  [Tracer Constructor: " << name << "]\n"; }
    ~Tracer() { std::cout << "  [Tracer Destructor: " << name << "]\n"; }
    Tracer(const Tracer& o) : name(o.name + " (Copy)") { std::cout << "  [Tracer Copy: " << name << "]\n"; }
    Tracer(Tracer&& o) noexcept : name(std::move(o.name)) { 
        o.name = "Empty (Moved)";
        std::cout << "  [Tracer Move: " << name << "]\n"; 
    }
    Tracer& operator=(const Tracer& o) {
        name = o.name + " (Copy Assign)";
        std::cout << "  [Tracer Copy Assign: " << name << "]\n";
        return *this;
    }
    Tracer& operator=(Tracer&& o) noexcept {
        name = std::move(o.name);
        o.name = "Empty (Moved)";
        std::cout << "  [Tracer Move Assign: " << name << "]\n";
        return *this;
    }
};

int main() {
    std::cout << "========================================================\n";
    std::cout << "1. DynamicArray Lifecycle and RAII Verification\n";
    std::cout << "========================================================\n";
    {
        sysmem::DynamicArray<Tracer> array;
        std::cout << "Pushing Tracer Alpha...\n";
        array.push_back(Tracer("Alpha"));

        std::cout << "Pushing Tracer Beta...\n";
        array.push_back(Tracer("Beta"));

        std::cout << "Array size: " << array.size() << ", Capacity: " << array.capacity() << "\n";
        std::cout << "Exiting inner scope -> All dynamic memory must destruct now:\n";
    }

    std::cout << "\n========================================================\n";
    std::cout << "2. Memory Alignment and Pointer Traversal\n";
    std::cout << "========================================================\n";
    sysmem::DynamicArray<int> numbers;
    for (int i = 1; i <= 5; ++i) {
        numbers.push_back(i * 10);
    }

    std::cout << "Pointer-based iteration via iterators (*p):\n";
    for (int val : numbers) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    std::cout << "\n========================================================\n";
    std::cout << "3. Contiguous Matrix vs Cache Locality\n";
    std::cout << "========================================================\n";
    constexpr std::size_t DIM = 2048;
    std::cout << std::format("Instantiating {}x{} Matrix ({} Megabytes)...\n", 
                             DIM, DIM, (DIM * DIM * sizeof(double)) / (1024 * 1024));

    sysmem::ContiguousMatrix<double> matrix(DIM, DIM, 1.0);

    // Row-major traversal (Cache-Friendly: reading adjacent memory addresses)
    auto t0 = std::chrono::high_resolution_clock::now();
    double row_sum = 0.0;
    for (std::size_t r = 0; r < DIM; ++r) {
        for (std::size_t c = 0; c < DIM; ++c) {
            row_sum += matrix(r, c);
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    auto row_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << std::format("Row-major sum completed in  : {} ms (Sum: {})\n", row_duration, row_sum);

    // Column-major traversal (Cache-Hostile: jumping stride of DIM across memory)
    auto t2 = std::chrono::high_resolution_clock::now();
    double col_sum = 0.0;
    for (std::size_t c = 0; c < DIM; ++c) {
        for (std::size_t r = 0; r < DIM; ++r) {
            col_sum += matrix(r, c);
        }
    }
    auto t3 = std::chrono::high_resolution_clock::now();
    auto col_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    std::cout << std::format("Column-major sum completed in: {} ms (Sum: {})\n", col_duration, col_sum);
    std::cout << std::format("Performance difference: Contiguous row access is {:.2f}x faster!\n", 
                             static_cast<double>(col_duration) / row_duration);

    return 0;
}