#ifndef D3SERVER_BATTLE_NET_CLIENT_H
#define D3SERVER_BATTLE_NET_CLIENT_H

#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include "d3core/logger.h"
#include "utils/debug.h"
#include "database/database_manager.h"
#include "battle_net/protocol.h"

// Forward declare Message if it's used in sendPacket signature
namespace google {
namespace protobuf {
    class Message;
}
}

namespace d3server {
namespace battle_net {

class BattleNetServer; // Forward declaration

class BattleNetClient : public std::enable_shared_from_this<BattleNetClient> {
public:
    BattleNetClient(boost::asio::ip::tcp::socket socket,
                    BattleNetServer& server, // server reference
                    std::shared_ptr<database::DatabaseManager> db_manager); // Config is not directly used by client logic
    ~BattleNetClient();

    void start();
    void close();

    void sendPacket(uint16_t serviceId, uint16_t methodId, uint32_t requestId, const std::vector<unsigned char>& body_payload);
    // void sendPacket(uint16_t serviceId, uint16_t methodId, uint32_t requestId, const google::protobuf::Message& message); // Placeholder

private:
    void doReadHeader();
    void handleReadHeader(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void doReadBody(uint32_t body_length);
    void handleReadBody(const boost::system::error_code& ec, std::size_t bytes_transferred, uint32_t body_length);
    void processPacket(const std::vector<unsigned char>& packet_data);
    void doWrite(const std::vector<unsigned char>& data);
    void handleWrite(const boost::system::error_code& ec, std::size_t bytes_transferred, size_t expected_bytes);

    // Packet processing handlers
    void handleAuthChallenge();
    void handleAuthSession(const std::vector<unsigned char>& request_data);

    boost::asio::ip::tcp::socket m_socket;
    BattleNetServer& m_server; // Reference to the managing server
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    std::vector<unsigned char> m_readBuffer; // Main buffer for reads
    protocol::PacketHeader m_currentHeader;  // Stores the currently processed packet's header

    // Client state (consistent with the new .cpp)
    std::string m_clientIpAddress;
    bool m_isAuthenticated;
    std::string m_accountLogin; // Login of the authenticated account
};

} // namespace battle_net
} // namespace d3server

#endif // D3SERVER_BATTLE_NET_CLIENT_H 