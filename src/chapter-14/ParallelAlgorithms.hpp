#ifndef PARALLEL_ALGORITHMS_HPP
#define PARALLEL_ALGORITHMS_HPP

#include "ThreadPool.hpp"
#include <span>
#include <numeric>
#include <vector>

namespace concurrency {

template <typename T, typename MapFunc, typename ReduceFunc>
[[nodiscard]] auto parallel_map_reduce(
    ThreadPool& pool,
    std::span<const T> data,
    auto init_val,
    MapFunc map_fn,
    ReduceFunc reduce_fn)
{
    using ResultType = decltype(init_val);
    const std::size_t total_elements = data.size();

    if (total_elements == 0) {
        return init_val;
    }

    const std::size_t num_chunks = pool.worker_count() * 4; // 4x oversubscription for load balancing
    const std::size_t chunk_size = (total_elements + num_chunks - 1) / num_chunks;

    std::vector<std::future<ResultType>> futures;

    // Partition and dispatch map sub-tasks to the Thread Pool
    for (std::size_t start = 0; start < total_elements; start += chunk_size) {
        const std::size_t end = std::min(start + chunk_size, total_elements);
        std::span<const T> slice = data.subspan(start, end - start);

        futures.push_back(pool.submit([slice, init_val, map_fn, reduce_fn]() -> ResultType {
            ResultType local_acc = init_val;
            for (const auto& item : slice) {
                local_acc = reduce_fn(local_acc, map_fn(item));
            }
            return local_acc;
        }));
    }

    // Reduce Phase: Collect all sub-task results
    ResultType final_result = init_val;
    for (auto& fut : futures) {
        final_result = reduce_fn(final_result, fut.get());
    }

    return final_result;
}

} // namespace concurrency

#endif // PARALLEL_ALGORITHMS_HPP