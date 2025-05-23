#pragma once

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <boost/asio.hpp>
#include <ctime>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace battle_net {

// Forward declarations
class BattleNetServer;

/**
 * @brief Authentication state of the client
 */
enum class AuthState {
    NotAuthenticated, ///< Client is not authenticated
    InProgress,       ///< Authentication is in progress
    Authenticated,    ///< Client is authenticated
    AuthFailed        ///< Authentication failed
};

/**
 * @brief Battle.net client class
 * 
 * This class represents a client connection to the Battle.net server
 */
class BattleNetClient : public std::enable_shared_from_this<BattleNetClient> {
public:
    /**
     * @brief Constructor
     * @param socket TCP socket
     * @param server Reference to the server
     * @param config Server configuration
     * @param dbManager Database manager
     */
    BattleNetClient(
        boost::asio::ip::tcp::socket socket,
        BattleNetServer& server,
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~BattleNetClient();
    
    /**
     * @brief Start reading from the socket
     */
    void start();
    
    /**
     * @brief Send a message to the client
     * @param message Message to send
     */
    virtual void sendMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Get client's login
     * @return Client's login
     */
    std::string getLogin() const { return m_login; }
    
    /**
     * @brief Get client's IP address
     * @return Client's IP address
     */
    std::string getIpAddress() const { return m_ipAddress; }
    
    /**
     * @brief Get client's authentication state
     * @return Authentication state
     */
    AuthState getAuthState() const { return m_authState; }
    
    /**
     * @brief Set client's authentication state
     * @param state New authentication state
     */
    void setAuthState(AuthState state);
    
    /**
     * @brief Get client's connection time
     * @return Connection time as time_t
     */
    std::time_t getConnectTime() const { return m_connectTime; }
    
    /**
     * @brief Get client's last activity time
     * @return Last activity time as time_t
     */
    std::time_t getLastActivityTime() const { return m_lastActivityTime; }
    
    /**
     * @brief Update client's last activity time to current time
     */
    void updateLastActivityTime();
    
    /**
     * @brief Disconnect the client
     */
    void disconnect();
    
    /**
     * @brief Check if the client is connected
     * @return True if connected, false otherwise
     */
    bool isConnected() const { return m_connected; }
    
protected:
    /**
     * @brief Read message header from the socket
     */
    void readHeader();
    
    /**
     * @brief Read message body from the socket
     * @param bodySize Size of the message body
     */
    void readBody(size_t bodySize);
    
    /**
     * @brief Process received message
     * @param message Received message
     */
    void processMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Handle authentication
     * @param login Client's login
     * @param password Client's password
     * @return True if authentication succeeded, false otherwise
     */
    bool handleAuth(const std::string& login, const std::string& password);
    
    /**
     * @brief Handle authentication message
     * @param message Authentication message
     */
    void handleAuthMessage(const std::vector<uint8_t>& message);
    
    boost::asio::ip::tcp::socket m_socket;
    BattleNetServer& m_server;
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    bool m_connected;
    std::string m_login;
    std::string m_ipAddress;
    AuthState m_authState;
    std::time_t m_connectTime;
    std::time_t m_lastActivityTime;
    std::vector<uint8_t> m_headerBuffer;
    std::vector<uint8_t> m_bodyBuffer;
    bool m_writing;
    std::queue<std::vector<uint8_t>> m_writeQueue;
};

} // namespace battle_net
} // namespace d3server 