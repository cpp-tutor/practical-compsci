#ifndef THREAD_SAFE_QUEUE_HPP
#define THREAD_SAFE_QUEUE_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace concurrency {

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() {
        shutdown();
    }

    // Non-copyable, non-movable synchronisation barrier
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push(std::move(value));
        }
        m_cv.notify_one();
    }

    // Blocking pop with graceful shutdown support
    [[nodiscard]] std::optional<T> pop_blocking() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this]() {
            return !m_queue.empty() || m_shutdown.load(std::memory_order_acquire);
        });

        if (m_queue.empty() && m_shutdown.load(std::memory_order_acquire)) {
            return std::nullopt;
        }

        T item = std::move(m_queue.front());
        m_queue.pop();
        return item;
    }

    void shutdown() noexcept {
        m_shutdown.store(true, std::memory_order_release);
        m_cv.notify_all();
    }

    [[nodiscard]] bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    [[nodiscard]] std::size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    mutable std::mutex       m_mutex;
    std::queue<T>            m_queue;
    std::condition_variable  m_cv;
    std::atomic<bool>        m_shutdown{false};
};

} // namespace concurrency

#endif // THREAD_SAFE_QUEUE_HPP