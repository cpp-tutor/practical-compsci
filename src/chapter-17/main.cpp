#include "SocketRAII.hpp"
#include "Protocol.hpp"
#include "Server.hpp"
#include "Client.hpp"

#include <iostream>
#include <sstream>
#include <chrono>

int main() {
    using namespace ModernNet;

    const uint16_t SERVER_PORT = 8080;

    std::cout << "====================================================\n";
    std::cout << "     Modern C++ Network Socket Framework Demo       \n";
    std::cout << "====================================================\n\n";

    // 1. Initialise and Start the Multi-threaded Server
    TcpServer server(SERVER_PORT);

    server.start([](const Socket& client_sock, const Message& msg) {
        switch (msg.type) {
            case MessageType::Ping: {
                std::cout << "[Server] Received PING. Responding with PONG...\n";
                ProtocolEngine::send_message(client_sock, MessageType::Pong, "PONG_ACK");
                break;
            }
            case MessageType::ExecuteQuery: {
                std::cout << "[Server] Received SQL Request: " << msg.payload << "\n";
                
                // Simulated Database Response
                std::string response_data;
                if (msg.payload.find("SELECT") != std::string::npos) {
                    response_data = 
                        "+-----+-------------------+------------+\n"
                        "| ID  | NAME              | SALARY     |\n"
                        "+-----+-------------------+------------+\n"
                        "| 101 | Ada Lovelace      | 8500.50    |\n"
                        "| 102 | Alan Turing       | 9200.00    |\n"
                        "+-----+-------------------+------------+\n"
                        "(2 rows returned across network stream)";
                } else {
                    response_data = "OK: Statement executed successfully.";
                }

                ProtocolEngine::send_message(client_sock, MessageType::QueryResult, response_data);
                break;
            }
            default: {
                std::cerr << "[Server] Unknown message type received.\n";
                ProtocolEngine::send_message(client_sock, MessageType::Error, "UNKNOWN_COMMAND");
                break;
            }
        }
    });

    // Give server thread a moment to bind and enter listen state
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 2. Initialise Client and Connect
    try {
        TcpClient client;
        client.connect_to("127.0.0.1", SERVER_PORT);

        // Test 1: Send Ping Frame
        std::cout << "\n[Client] Sending PING frame...\n";
        client.send_request(MessageType::Ping, "");
        Message ping_resp = client.receive_response();
        std::cout << "[Client] Server reply -> Type: " 
                  << (ping_resp.type == MessageType::Pong ? "PONG" : "OTHER") 
                  << ", Payload: " << ping_resp.payload << "\n";

        // Test 2: Send Networked SQL Query Frame
        std::string sql_query = "SELECT id, name, salary FROM engineers WHERE salary > 8000.0;";
        std::cout << "\n[Client] Transmitting SQL Query: " << sql_query << "\n";
        client.send_request(MessageType::ExecuteQuery, sql_query);

        Message query_resp = client.receive_response();
        std::cout << "[Client] Query Response Received:\n\n" << query_resp.payload << "\n\n";

        client.disconnect();
    } catch (const std::exception& e) {
        std::cerr << "[Client Error]: " << e.what() << "\n";
    }

    // 3. Graceful Server Shutdown
    server.stop();
    std::cout << "[System] Network demonstration finished cleanly.\n";

    return 0;
}