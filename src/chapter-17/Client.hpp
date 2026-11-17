#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "SocketRAII.hpp"
#include "Protocol.hpp"

namespace ModernNet {

class TcpClient {
public:
    TcpClient() = default;

    void connect_to(std::string_view host, uint16_t port) {
        int raw_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (raw_fd < 0) {
            throw std::runtime_error("Failed to create client socket: " + std::string(strerror(errno)));
        }
        m_socket = Socket(raw_fd);

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (::inet_pton(AF_INET, host.data(), &server_addr.sin_addr) <= 0) {
            throw std::runtime_error("Invalid address / Address not supported: " + std::string(host));
        }

        if (::connect(m_socket.native_handle(), reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr)) < 0) {
            throw std::runtime_error("Connection to " + std::string(host) + ":" + 
                                     std::to_string(port) + " failed: " + strerror(errno));
        }

        std::cout << "[Client] Successfully connected to " << host << ":" << port << "\n";
    }

    void send_request(MessageType type, std::string_view payload) {
        ProtocolEngine::send_message(m_socket, type, payload);
    }

    Message receive_response() {
        return ProtocolEngine::receive_message(m_socket);
    }

    void disconnect() {
        m_socket.close_socket();
    }

private:
    Socket m_socket;
};

} // namespace ModernNet

#endif // CLIENT_HPP