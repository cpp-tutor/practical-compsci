#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP

#include "ThreadSafeQueue.hpp"
#include <vector>
#include <thread>
#include <future>
#include <functional>
#include <type_traits>
#include <memory>
#include <stdexcept>
#include <format>

namespace concurrency {

class ThreadPool {
public:
    explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency()) {
        const std::size_t actual_threads = (thread_count == 0) ? 4 : thread_count;
        m_workers.reserve(actual_threads);

        for (std::size_t i = 0; i < actual_threads; ++i) {
            // Emplace jthread instances with cooperative cancellation tokens
            m_workers.emplace_back([this](std::stop_token stop_tok) {
                worker_loop(stop_tok);
            });
        }
    }

    ~ThreadPool() {
        shutdown();
    }

    // Non-copyable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // Enqueue an arbitrary callable function returning a std::future
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>>
    {
        using ReturnType = std::invoke_result_t<F, Args...>;

        if (m_stop_requested.load(std::memory_order_acquire)) {
            throw std::runtime_error("ThreadPool: Cannot submit tasks to a terminated pool.");
        }

        // Package the task into a heap-allocated shared promise
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [func = std::forward<F>(f), ...captured_args = std::forward<Args>(args)]() mutable {
                return std::invoke(func, captured_args...);
            }
        );

        std::future<ReturnType> future_result = task->get_future();

        // Type-erase the packaged task into a void() function
        m_task_queue.push([task]() {
            (*task)();
        });

        return future_result;
    }

    void shutdown() noexcept {
        if (!m_stop_requested.exchange(true, std::memory_order_acq_rel)) {
            m_task_queue.shutdown();
            // jthread automatically signals stop_token and joins on destruction!
        }
    }

    [[nodiscard]] std::size_t worker_count() const noexcept {
        return m_workers.size();
    }

private:
    void worker_loop(std::stop_token stop_tok) {
        while (!stop_tok.stop_requested()) {
            auto task_opt = m_task_queue.pop_blocking();
            if (!task_opt.has_value()) {
                break; // Queue shut down and drained
            }
            // Execute the submitted task
            (*task_opt)();
        }
    }

    using Task = std::function<void()>;

    std::vector<std::jthread>  m_workers;
    ThreadSafeQueue<Task>      m_task_queue;
    std::atomic<bool>          m_stop_requested{false};
};

} // namespace concurrency

#endif // THREAD_POOL_HPP