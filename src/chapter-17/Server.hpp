#ifndef SERVER_HPP
#define SERVER_HPP

#include "SocketRAII.hpp"
#include "Protocol.hpp"
#include <thread>
#include <vector>
#include <atomic>
#include <functional>
#include <mutex>

namespace ModernNet {

class TcpServer {
public:
    using MessageHandler = std::function<void(const Socket& client_sock, const Message& msg)>;

    explicit TcpServer(uint16_t port) : m_port(port), m_is_running(false) {}

    ~TcpServer() {
        stop();
    }

    void start(MessageHandler handler) {
        m_handler = std::move(handler);
        
        int raw_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (raw_fd < 0) {
            throw std::runtime_error("Failed to create server socket: " + std::string(strerror(errno)));
        }
        m_listen_socket = Socket(raw_fd);

        // Set SO_REUSEADDR to prevent "Address already in use" binding errors during restarts
        int opt = 1;
        if (::setsockopt(m_listen_socket.native_handle(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            throw std::runtime_error("setsockopt(SO_REUSEADDR) failed.");
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
        server_addr.sin_port = htons(m_port);

        if (::bind(m_listen_socket.native_handle(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            throw std::runtime_error("Bind failed on port " + std::to_string(m_port) + ": " + strerror(errno));
        }

        if (::listen(m_listen_socket.native_handle(), SOMAXCONN) < 0) {
            throw std::runtime_error("Listen failed: " + std::string(strerror(errno)));
        }

        m_is_running = true;
        std::cout << "[Server] Listening on 0.0.0.0:" << m_port << "...\n";

        // Spawn accept loop thread
        m_accept_thread = std::jthread([this](std::stop_token st) {
            accept_loop(st);
        });
    }

    void stop() {
        if (!m_is_running.exchange(false)) return;

        std::cout << "[Server] Shutting down...\n";
        m_listen_socket.shutdown();
        m_listen_socket.close_socket();

        if (m_accept_thread.joinable()) {
            m_accept_thread.request_stop();
        }

        // Join active client threads
        std::lock_guard<std::mutex> lock(m_threads_mutex);
        m_worker_threads.clear();
    }

private:
    uint16_t m_port;
    Socket m_listen_socket;
    std::atomic<bool> m_is_running{false};
    MessageHandler m_handler;
    std::jthread m_accept_thread;

    std::mutex m_threads_mutex;
    std::vector<std::jthread> m_worker_threads;

    void accept_loop(std::stop_token st) {
        while (!st.stop_requested() && m_is_running) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);

            int client_fd = ::accept(m_listen_socket.native_handle(), 
                                     reinterpret_cast<sockaddr*>(&client_addr), 
                                     &client_len);

            if (client_fd < 0) {
                if (!m_is_running) break; // Normal socket shutdown
                std::cerr << "[Server] Accept error: " << strerror(errno) << "\n";
                continue;
            }

            char ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), ip_str, INET_ADDRSTRLEN);
            std::cout << "[Server] Accepted connection from " << ip_str << ":" 
                      << ntohs(client_addr.sin_port) << "\n";

            Socket client_sock(client_fd);

            // Dispatch to worker thread
            std::lock_guard<std::mutex> lock(m_threads_mutex);
            m_worker_threads.emplace_back([this, sock = std::move(client_sock)](std::stop_token) mutable {
                handle_client(std::move(sock));
            });
        }
    }

    void handle_client(Socket sock) {
        try {
            while (m_is_running) {
                Message msg = ProtocolEngine::receive_message(sock);
                m_handler(sock, msg);
            }
        } catch (const std::exception& e) {
            // Client disconnect or socket error
            // (Standard EOF terminates loop cleanly)
        }
    }
};

} // namespace ModernNet

#endif // SERVER_HPP