#include "battle_net/battle_net_client.h"
#include "battle_net/battle_net_server.h"
#include "battle_net/protocol.h"
#include "utils/crypto_utils.h"
#include "utils/string_utils.h"
#include "d3core/logger.h"
#include "utils/debug.h"

#include <iostream> // For temporary logging
#include <vector>

// Required for ntohl, htonl if not using Boost.Endian or similar for PacketHeader serialize/deserialize
// However, protocol.h currently uses memcpy, so these are not strictly needed by BattleNetClient itself
// unless direct byte order manipulation is done here.
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace d3server {
namespace battle_net {

BattleNetClient::BattleNetClient(
    boost::asio::ip::tcp::socket socket,
    BattleNetServer& server,
    std::shared_ptr<database::DatabaseManager> db_manager)
    : m_socket(std::move(socket)), 
      m_server(server),
      m_dbManager(db_manager),
      m_readBuffer(protocol::PacketHeader::SIZE), // Initialize for header
      m_isAuthenticated(false) {
    DEBUG_FUNCTION_ENTER();
    try {
        m_clientIpAddress = m_socket.remote_endpoint().address().to_string();
        LOG_INFO("Battle.net client connected: " + m_clientIpAddress);
    } catch (const boost::system::system_error& e) {
        LOG_ERROR("Failed to get client remote endpoint: " + std::string(e.what()));
        m_clientIpAddress = "unknown";
    }
    DEBUG_FUNCTION_EXIT();
}

BattleNetClient::~BattleNetClient() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Battle.net client disconnected: " + m_clientIpAddress + (m_accountLogin.empty() ? "" : " (User: " + m_accountLogin + ")"));
    // Socket should be closed by close() or by error handlers. Ensure it is.
    if (m_socket.is_open()) {
        // This is a fallback; normally, close() should have been called.
        LOG_WARNING("Socket still open in ~BattleNetClient for " + m_clientIpAddress + ". Forcing close.");
        boost::system::error_code ec;
        m_socket.close(ec); // Just close, shutdown might have been attempted.
    }
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::start() {
    DEBUG_FUNCTION_ENTER();
    LOG_DEBUG("Starting client session for " + m_clientIpAddress);
    doReadHeader();
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::close() {
    DEBUG_FUNCTION_ENTER();
    if (m_socket.is_open()) {
        boost::system::error_code ec_shutdown;
        m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec_shutdown);
        if (ec_shutdown && ec_shutdown != boost::asio::error::not_connected) { 
            LOG_WARNING("Error shutting down socket for " + m_clientIpAddress + ": " + ec_shutdown.message());
        }
        boost::system::error_code ec_close;
        m_socket.close(ec_close);
        if (ec_close) {
            LOG_WARNING("Error closing socket for " + m_clientIpAddress + ": " + ec_close.message());
        }
        LOG_INFO("Socket explicitly closed for client " + m_clientIpAddress);
    }    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::doReadHeader() {
    DEBUG_FUNCTION_ENTER();
    if (!m_socket.is_open()) { 
        LOG_DEBUG("Socket not open, cannot read header for " + m_clientIpAddress + ". Client might be disconnecting.");
        DEBUG_FUNCTION_EXIT(); 
        // Server will remove on next check or if error was propagated
        return; 
    }
    
    m_readBuffer.assign(protocol::PacketHeader::SIZE, 0); // Clear and resize for header
    auto self = shared_from_this();
    boost::asio::async_read(m_socket, 
        boost::asio::buffer(m_readBuffer.data(), protocol::PacketHeader::SIZE),
        boost::asio::transfer_exactly(protocol::PacketHeader::SIZE), // Ensure full header is read
        [this, self](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            handleReadHeader(ec, bytes_transferred);
        }
    );
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::handleReadHeader(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    DEBUG_FUNCTION_ENTER();
    if (!m_socket.is_open()) { 
        LOG_DEBUG("Socket not open in handleReadHeader for " + m_clientIpAddress + ". Client might be disconnecting.");
        m_server.removeClient(shared_from_this());
        DEBUG_FUNCTION_EXIT(); 
        return; 
    }

    if (!ec) {
        // bytes_transferred should be equal to protocol::PacketHeader::SIZE due to transfer_exactly
        if (bytes_transferred == protocol::PacketHeader::SIZE) {
            if (!m_currentHeader.deserialize(m_readBuffer)) { 
                LOG_ERROR("Failed to deserialize packet header from " + m_clientIpAddress + ". Raw: [" + utils::CryptoUtils::hexEncode(m_readBuffer) + "]");
                close();
                m_server.removeClient(shared_from_this());
                return;
            }

            LOG_INFO("Header from " + m_clientIpAddress + ": ServiceID=0x" + 
                     utils::StringUtils::toHex(m_currentHeader.service_id) +
                     ", MethodID=0x" + utils::StringUtils::toHex(m_currentHeader.method_id) +
                     ", RequestID=" + std::to_string(m_currentHeader.request_id) +
                     ", BodyLength=" + std::to_string(m_currentHeader.body_length));

            if (m_currentHeader.body_length == 0) {
                processPacket({}); 
            } else if (m_currentHeader.body_length > 0 && m_currentHeader.body_length <= protocol::MAX_PACKET_BODY_SIZE) {
                doReadBody(m_currentHeader.body_length);
            } else {
                LOG_WARNING("Invalid body length: " + std::to_string(m_currentHeader.body_length) + 
                            " (Max: " + std::to_string(protocol::MAX_PACKET_BODY_SIZE) + ") from " + m_clientIpAddress);
                close();
                m_server.removeClient(shared_from_this());
            }
        } else { // Should not happen with transfer_exactly unless error
            LOG_WARNING("Incomplete header (" + std::to_string(bytes_transferred) + " bytes) from " + m_clientIpAddress + ". Expected " + std::to_string(protocol::PacketHeader::SIZE));
            close();
            m_server.removeClient(shared_from_this());
        }
    } else {
        if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
            LOG_INFO("Client " + m_clientIpAddress + " disconnected (EOF/Reset during header read).");
        } else {
            LOG_ERROR("Error reading header from " + m_clientIpAddress + ": " + ec.message());
        }
        close(); 
        m_server.removeClient(shared_from_this());
    }
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::doReadBody(uint32_t body_length) {
    DEBUG_FUNCTION_ENTER();
    if (!m_socket.is_open()) { 
        LOG_DEBUG("Socket not open, cannot read body for " + m_clientIpAddress + ". Client might be disconnecting.");
        DEBUG_FUNCTION_EXIT(); 
        return; 
    }

    m_readBuffer.assign(body_length, 0); // Clear and resize for body
    auto self = shared_from_this();
    boost::asio::async_read(m_socket, 
        boost::asio::buffer(m_readBuffer.data(), body_length),
        boost::asio::transfer_exactly(body_length), // Ensure full body is read
        [this, self, body_length](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            handleReadBody(ec, bytes_transferred, body_length);
        }
    );
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::handleReadBody(const boost::system::error_code& ec, std::size_t bytes_transferred, uint32_t expected_body_length) {
    DEBUG_FUNCTION_ENTER();
    if (!m_socket.is_open()) { 
        LOG_DEBUG("Socket not open in handleReadBody for " + m_clientIpAddress + ". Client might be disconnecting.");
        m_server.removeClient(shared_from_this()); 
        DEBUG_FUNCTION_EXIT(); 
        return; 
    }

    if (!ec) {
        if (bytes_transferred == expected_body_length) {
            LOG_DEBUG("Body received (" + std::to_string(bytes_transferred) + " bytes) from " + m_clientIpAddress);
            // DEBUG_LOG("Raw Body Data: [" + utils::CryptoUtils::hexEncode(m_readBuffer) + "]");
            
            processPacket(m_readBuffer); // Pass the current m_readBuffer (which is the body)
        } else { // Should not happen with transfer_exactly
            LOG_WARNING("Incomplete body (" + std::to_string(bytes_transferred) + " bytes) from " + m_clientIpAddress + ". Expected " + std::to_string(expected_body_length));
            close();
            m_server.removeClient(shared_from_this());
        }
    } else {
        if (ec == boost::asio::error::eof || ec == boost::asio::error::connection_reset) {
            LOG_INFO("Client " + m_clientIpAddress + " disconnected (EOF/Reset during body read).");
        } else {
            LOG_ERROR("Error reading body from " + m_clientIpAddress + ": " + ec.message());
        }
        close();
        m_server.removeClient(shared_from_this());
    }
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::processPacket(const std::vector<unsigned char>& packet_data) {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Processing packet from " + m_clientIpAddress + ": ServiceID=0x" + utils::StringUtils::toHex(m_currentHeader.service_id) +
             ", MethodID=0x" + utils::StringUtils::toHex(m_currentHeader.method_id) +
             ", RequestID=" + std::to_string(m_currentHeader.request_id) +
             ", BodySize=" + std::to_string(packet_data.size()));

    switch (m_currentHeader.service_id) {
        case protocol::SERVICE_ID_AUTHENTICATION:
            switch (m_currentHeader.method_id) {
                case protocol::METHOD_ID_AUTH_CHALLENGE_REQUEST:
                    LOG_DEBUG("Dispatching to handleAuthChallenge.");
                    handleAuthChallenge(); 
                    break;
                case protocol::METHOD_ID_AUTH_SESSION_REQUEST:
                    LOG_DEBUG("Dispatching to handleAuthSession.");
                    handleAuthSession(packet_data); 
                    break;
                default:
                    LOG_WARNING("Unhandled MethodID 0x" + utils::StringUtils::toHex(m_currentHeader.method_id) + 
                                " for Service AUTHENTICATION from " + m_clientIpAddress);
                    break;
            }
            break;
        default:
            LOG_WARNING("Unhandled ServiceID 0x" + utils::StringUtils::toHex(m_currentHeader.service_id) + " from " + m_clientIpAddress);
            break;
    }

    if (m_socket.is_open()) {
        doReadHeader(); // Continue reading loop if connection is still active
    }
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::doWrite(const std::vector<unsigned char>& data) {
    DEBUG_FUNCTION_ENTER();
    if (!m_socket.is_open()) {
        LOG_WARNING("Attempted to write to a closed socket for " + m_clientIpAddress);
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    auto self = shared_from_this();
    boost::asio::async_write(m_socket, boost::asio::buffer(data),
        boost::asio::transfer_all(), // Ensure all data is written
        [this, self, data_size = data.size()](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            handleWrite(ec, bytes_transferred, data_size);
        }
    );
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::handleWrite(const boost::system::error_code& ec, std::size_t bytes_transferred, size_t expected_bytes) {
    DEBUG_FUNCTION_ENTER();
    if (!m_socket.is_open() && !ec) {
        // If socket closed between write initiation and handler, but no error reported by asio
        LOG_DEBUG("Socket closed before write completion for " + m_clientIpAddress + ", but no Asio error.");
        // Server might have already handled removal or will on next check.
        DEBUG_FUNCTION_EXIT();
        return;
    }

    if (!ec) {
        // bytes_transferred should be equal to expected_bytes due to transfer_all
        if (bytes_transferred == expected_bytes) {
            LOG_DEBUG("Sent " + std::to_string(bytes_transferred) + " bytes successfully to " + m_clientIpAddress);
        } else { // Should not happen with transfer_all
            LOG_WARNING("Sent " + std::to_string(bytes_transferred) + " bytes, but expected " + 
                        std::to_string(expected_bytes) + " to " + m_clientIpAddress + ". Asio error not set!");
        }
    } else {
        LOG_ERROR("Error writing to " + m_clientIpAddress + ": " + ec.message());
        close(); 
        m_server.removeClient(shared_from_this());
    }
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::handleAuthChallenge() {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Handling Auth Challenge for " + m_clientIpAddress + " (RequestID: " + std::to_string(m_currentHeader.request_id) + ")");
    std::vector<unsigned char> challenge_payload; // Empty for mock
    LOG_INFO("Sending mock Auth Challenge Response to " + m_clientIpAddress);
    sendPacket(protocol::SERVICE_ID_AUTHENTICATION, 
               protocol::METHOD_ID_AUTH_CHALLENGE_RESPONSE, 
               m_currentHeader.request_id, 
               challenge_payload);
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::handleAuthSession(const std::vector<unsigned char>& request_data) {
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("Handling Auth Session for " + m_clientIpAddress + " (RequestID: " + std::to_string(m_currentHeader.request_id) + ", Data size: " + std::to_string(request_data.size()) + ")");
    // DEBUG_LOG("Raw Auth Session Request Data: [" + utils::CryptoUtils::hexEncode(request_data) + "]");

    bool auth_successful = true; // Placeholder
    std::string mock_login = "testuser_srp"; // Placeholder, would parse from request_data

    if (auth_successful) {
        m_isAuthenticated = true;
        m_accountLogin = mock_login; 
        LOG_INFO("User '" + m_accountLogin + "' authenticated successfully (mock). IP: " + m_clientIpAddress);
        // m_dbManager->updateAccountLastLogin(m_accountLogin); // Uncomment when DB is ready

        std::vector<unsigned char> success_payload; 
        sendPacket(protocol::SERVICE_ID_AUTHENTICATION, 
                   protocol::METHOD_ID_LOGON_SUCCESS, 
                   m_currentHeader.request_id, 
                   success_payload);
    } else {
        LOG_WARNING("User authentication failed (mock) for IP: " + m_clientIpAddress);
        std::vector<unsigned char> failure_payload; 
        sendPacket(protocol::SERVICE_ID_AUTHENTICATION, 
                   protocol::METHOD_ID_AUTH_SESSION_RESPONSE, // Generic failure response
                   m_currentHeader.request_id, 
                   failure_payload); 
        // close(); // Optionally close on failure
        // m_server.removeClient(shared_from_this());
    }
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::sendPacket(uint16_t serviceId, uint16_t methodId, uint32_t requestId, const std::vector<unsigned char>& body_payload) {
   DEBUG_FUNCTION_ENTER();
   protocol::PacketHeader header;
   header.service_id = serviceId;
   header.method_id = methodId;
   header.request_id = requestId;
   header.body_length = static_cast<uint32_t>(body_payload.size());

   std::vector<unsigned char> packet_to_send;
   packet_to_send.resize(protocol::PacketHeader::SIZE); // Resize for header first
   header.serialize(packet_to_send); // Serialize header into the start of packet_to_send
                                      // Assumes serialize writes exactly PacketHeader::SIZE

   if (header.body_length > 0) {
       packet_to_send.insert(packet_to_send.end(), body_payload.begin(), body_payload.end());
   }

   LOG_INFO("Queueing packet to send to " + m_clientIpAddress + ": Service=0x" + utils::StringUtils::toHex(serviceId) +
             ", Method=0x" + utils::StringUtils::toHex(methodId) +
             ", ReqID=" + std::to_string(requestId) +
             ", BodySize=" + std::to_string(header.body_length) +
             ", TotalSize=" + std::to_string(packet_to_send.size()));
   // DEBUG_LOG("Raw Packet to send: [" + utils::CryptoUtils::hexEncode(packet_to_send) + "]");

   doWrite(packet_to_send);
   DEBUG_FUNCTION_EXIT();
}

// Placeholder for Protobuf based sendPacket (if used later)
// void BattleNetClient::sendPacket(uint16_t serviceId, uint16_t methodId, uint32_t requestId, const google::protobuf::Message& message) { ... }

} // namespace battle_net
} // namespace d3server 