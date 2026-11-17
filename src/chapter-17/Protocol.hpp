#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include "SocketRAII.hpp"
#include <string_view>
#include <bit>

namespace ModernNet {

enum class MessageType : uint16_t {
    Ping         = 0x0001,
    Pong         = 0x0002,
    ExecuteQuery = 0x0010,
    QueryResult  = 0x0011,
    Error        = 0x00FF
};

#pragma pack(push, 1)
struct FrameHeader {
    uint16_t magic{0xCAFE};       // Protocol Identifier
    uint16_t msg_type{0};         // MessageType
    uint32_t payload_length{0};   // Size of payload in bytes
};
#pragma pack(pop)

static_assert(sizeof(FrameHeader) == 8, "FrameHeader must be exactly 8 bytes packed.");

struct Message {
    MessageType type;
    std::string payload;
};

class ProtocolEngine {
public:
    static constexpr uint16_t PROTOCOL_MAGIC = 0xCAFE;

    // Serialise and transmit a message frame across the socket
    static void send_message(const Socket& sock, MessageType type, std::string_view payload) {
        FrameHeader header;
        header.magic = htons(PROTOCOL_MAGIC);
        header.msg_type = htons(static_cast<uint16_t>(type));
        header.payload_length = htonl(static_cast<uint32_t>(payload.size()));

        // Transmit header
        std::span<const std::byte> header_bytes(
            reinterpret_cast<const std::byte*>(&header), sizeof(FrameHeader)
        );
        sock.send_all(header_bytes);

        // Transmit payload
        if (!payload.empty()) {
            std::span<const std::byte> payload_bytes(
                reinterpret_cast<const std::byte*>(payload.data()), payload.size()
            );
            sock.send_all(payload_bytes);
        }
    }

    // Block and reconstruct an incoming framed message
    static Message receive_message(const Socket& sock) {
        FrameHeader raw_header;
        std::span<std::byte> header_dest(
            reinterpret_cast<std::byte*>(&raw_header), sizeof(FrameHeader)
        );
        sock.recv_exact(header_dest);

        // Decode network byte order
        uint16_t magic = ntohs(raw_header.magic);
        uint16_t type_val = ntohs(raw_header.msg_type);
        uint32_t length = ntohl(raw_header.payload_length);

        if (magic != PROTOCOL_MAGIC) {
            throw std::runtime_error("Protocol Error: Invalid header magic bytes 0x" + 
                                     std::to_string(magic));
        }

        // Allocate payload buffer and read exact payload length
        std::string payload;
        payload.resize(length);

        if (length > 0) {
            std::span<std::byte> payload_dest(
                reinterpret_cast<std::byte*>(payload.data()), length
            );
            sock.recv_exact(payload_dest);
        }

        return Message{
            .type = static_cast<MessageType>(type_val),
            .payload = std::move(payload)
        };
    }
};

} // namespace ModernNet

#endif // PROTOCOL_HPP