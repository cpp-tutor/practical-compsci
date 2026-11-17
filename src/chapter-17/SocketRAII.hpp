#ifndef SOCKET_RAII_HPP
#define SOCKET_RAII_HPP

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <span>
#include <cstdint>
#include <cstring>

// POSIX Networking Headers
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

namespace ModernNet {

// RAII Wrapper around a POSIX file descriptor
class Socket {
public:
    Socket() : m_fd(-1) {}
    explicit Socket(int fd) : m_fd(fd) {}

    ~Socket() {
        close_socket();
    }

    // Move-only semantics (no accidental descriptor duplication)
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    Socket(Socket&& other) noexcept : m_fd(other.m_fd) {
        other.m_fd = -1;
    }

    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) {
            close_socket();
            m_fd = other.m_fd;
            other.m_fd = -1;
        }
        return *this;
    }

    [[nodiscard]] int native_handle() const noexcept { return m_fd; }
    [[nodiscard]] bool is_valid() const noexcept { return m_fd >= 0; }

    void shutdown(int how = SHUT_RDWR) noexcept {
        if (m_fd >= 0) {
            ::shutdown(m_fd, how);
        }
    }

    void close_socket() noexcept {
        if (m_fd >= 0) {
            ::close(m_fd);
            m_fd = -1;
        }
    }

    // Guaranteed complete send over stream
    void send_all(std::span<const std::byte> data) const {
        size_t total_sent = 0;
        const auto* raw_ptr = reinterpret_cast<const char*>(data.data());

        while (total_sent < data.size()) {
            ssize_t bytes_sent = ::send(m_fd, raw_ptr + total_sent, data.size() - total_sent, 0);
            if (bytes_sent <= 0) {
                throw std::runtime_error("Socket write failure: " + std::string(strerror(errno)));
            }
            total_sent += static_cast<size_t>(bytes_sent);
        }
    }

    // Guaranteed complete receive over stream
    void recv_exact(std::span<std::byte> destination) const {
        size_t total_read = 0;
        auto* raw_ptr = reinterpret_cast<char*>(destination.data());

        while (total_read < destination.size()) {
            ssize_t bytes_read = ::recv(m_fd, raw_ptr + total_read, destination.size() - total_read, 0);
            if (bytes_read == 0) {
                throw std::runtime_error("Connection closed by peer (EOF).");
            }
            if (bytes_read < 0) {
                throw std::runtime_error("Socket read failure: " + std::string(strerror(errno)));
            }
            total_read += static_cast<size_t>(bytes_read);
        }
    }

private:
    int m_fd;
};

} // namespace ModernNet

#endif // SOCKET_RAII_HPP