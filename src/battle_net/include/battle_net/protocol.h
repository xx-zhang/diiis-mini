"""#ifndef D3SERVER_BATTLE_NET_PROTOCOL_H
#define D3SERVER_BATTLE_NET_PROTOCOL_H

#include <cstdint>
#include <vector>

namespace d3server {
namespace battle_net {
namespace protocol {

// Example Service IDs (these are specific to the D3 protocol)
// These would be discovered through protocol analysis.
constexpr uint16_t SERVICE_ID_AUTHENTICATION = 0x0001; // Example
constexpr uint16_t SERVICE_ID_CONNECTION = 0x0002;     // Example
constexpr uint16_t SERVICE_ID_FRIENDS = 0x0003;        // Example
constexpr uint16_t SERVICE_ID_GAME_UTILITIES = 0x0004; // Example
// ... and so on for all services

// Example Method IDs for SERVICE_ID_AUTHENTICATION
constexpr uint16_t METHOD_ID_AUTH_CHALLENGE_REQUEST = 0x0001;  // Client -> Server
constexpr uint16_t METHOD_ID_AUTH_CHALLENGE_RESPONSE = 0x0002; // Server -> Client
constexpr uint16_t METHOD_ID_AUTH_SESSION_REQUEST = 0x0003;    // Client -> Server
constexpr uint16_t METHOD_ID_AUTH_SESSION_RESPONSE = 0x0004;   // Server -> Client
constexpr uint16_t METHOD_ID_LOGON_QUEUE_UPDATE = 0x0005;    // Server -> Client (Queue position)
constexpr uint16_t METHOD_ID_LOGON_SUCCESS = 0x0006;         // Server -> Client (Authentication complete)

// A very simplified generic packet header structure
// The actual D3 header is more complex and might not directly map like this.
// It typically includes service hash, method id, request id, and size.
struct PacketHeader {
    uint16_t service_id;  // Or service_hash for BNet 2.0
    uint16_t method_id;
    uint32_t request_id;  // Or sequence number, token
    uint32_t body_length; // Size of the payload following the header

    static constexpr size_t SIZE = sizeof(service_id) + sizeof(method_id) + sizeof(request_id) + sizeof(body_length);

    // Basic serialization (example, network byte order (big-endian) is typical)
    // For simplicity, this example doesn't handle byte order conversion.
    // In a real implementation, use functions like htonl/ntohl or Boost.Endian.
    void serialize(std::vector<unsigned char>& buffer) const {
        buffer.resize(SIZE);
        uint32_t offset = 0;
        memcpy(buffer.data() + offset, &service_id, sizeof(service_id)); offset += sizeof(service_id);
        memcpy(buffer.data() + offset, &method_id, sizeof(method_id));   offset += sizeof(method_id);
        memcpy(buffer.data() + offset, &request_id, sizeof(request_id)); offset += sizeof(request_id);
        memcpy(buffer.data() + offset, &body_length, sizeof(body_length));
    }

    bool deserialize(const std::vector<unsigned char>& buffer) {
        if (buffer.size() < SIZE) return false;
        uint32_t offset = 0;
        memcpy(&service_id, buffer.data() + offset, sizeof(service_id)); offset += sizeof(service_id);
        memcpy(&method_id, buffer.data() + offset, sizeof(method_id));   offset += sizeof(method_id);
        memcpy(&request_id, buffer.data() + offset, sizeof(request_id)); offset += sizeof(request_id);
        memcpy(&body_length, buffer.data() + offset, sizeof(body_length));
        return true;
    }
};

// Maximum expected packet size to prevent buffer overflows from malicious packets
constexpr size_t MAX_PACKET_BODY_SIZE = 1024 * 64; // 64 KB, adjust as needed
constexpr size_t MAX_PACKET_FULL_SIZE = PacketHeader::SIZE + MAX_PACKET_BODY_SIZE;


// Placeholder for Protobuf message definitions if used
// e.g., #include "bnet/authentication_service.pb.h"

} // namespace protocol
} // namespace battle_net
} // namespace d3server

#endif // D3SERVER_BATTLE_NET_PROTOCOL_H 