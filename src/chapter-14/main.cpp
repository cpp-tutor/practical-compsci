#include "ThreadPool.hpp"
#include "ParallelAlgorithms.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <format>
#include <random>

int main() {
    constexpr std::size_t DATASET_SIZE = 20'000'000;
    std::cout << std::format("Generating dataset of {} double-precision numbers...\n", DATASET_SIZE);

    std::vector<double> dataset(DATASET_SIZE);
    std::mt19937_64 rng(42);
    std::uniform_real_distribution<double> dist(1.0, 10.0);

    for (std::size_t i = 0; i < DATASET_SIZE; ++i) {
        dataset[i] = dist(rng);
    }

    // Mathematical operation: compute sum of log(sqrt(x))
    auto map_op = [](double x) noexcept -> double {
        return std::log(std::sqrt(x));
    };

    auto reduce_op = [](double a, double b) noexcept -> double {
        return a + b;
    };

    // =========================================================================
    // 1. Single-Threaded Baseline
    // =========================================================================
    std::cout << "\n--- 1. Sequential Single-Threaded Execution ---\n";
    auto t0 = std::chrono::high_resolution_clock::now();

    double sequential_sum = 0.0;
    for (double val : dataset) {
        sequential_sum = reduce_op(sequential_sum, map_op(val));
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto seq_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    std::cout << std::format("Sequential Result   : {:.6f}\n", sequential_sum);
    std::cout << std::format("Sequential Duration : {} ms\n", seq_duration);

    // =========================================================================
    // 2. Multi-Threaded Parallel Execution via ThreadPool
    // =========================================================================
    const auto hardware_cores = std::thread::hardware_concurrency();
    std::cout << std::format("\n--- 2. Multi-Threaded Execution ({} Hardware Threads) ---\n", hardware_cores);

    concurrency::ThreadPool pool(hardware_cores);

    auto t2 = std::chrono::high_resolution_clock::now();

    double parallel_sum = concurrency::parallel_map_reduce(
        pool,
        std::span<const double>(dataset),
        0.0,
        map_op,
        reduce_op
    );

    auto t3 = std::chrono::high_resolution_clock::now();
    auto par_duration = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count();
    std::cout << std::format("Parallel Result     : {:.6f}\n", parallel_sum);
    std::cout << std::format("Parallel Duration   : {} ms\n", par_duration);

    // =========================================================================
    // 3. Verification and Scaling Analysis
    // =========================================================================
    std::cout << "\n=========================================================\n";
    std::cout << std::format("Parallel Speedup Ratio: {:.2f}x faster\n", 
                             static_cast<double>(seq_duration) / par_duration);
    std::cout << std::format("Result Difference     : {:.10e}\n", std::abs(sequential_sum - parallel_sum));
    std::cout << "=========================================================\n";

    return 0;
}