#pragma once

#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <boost/asio.hpp>

namespace d3server {

// Forward declarations
namespace d3core {
class Config;
}

namespace database {
class DatabaseManager;
class Character;
}

namespace game_server {

// Forward declarations
class GameServer;
class GameSession;

/**
 * @brief Class representing a connected game client
 */
class GameClient : public std::enable_shared_from_this<GameClient> {
public:
    /**
     * @brief Constructor
     * @param socket Socket for the client connection
     * @param server Game server instance
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    GameClient(
        boost::asio::ip::tcp::socket socket,
        GameServer& server,
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );
    
    /**
     * @brief Destructor
     */
    ~GameClient();
    
    /**
     * @brief Start processing client connection
     */
    void start();
    
    /**
     * @brief Send a message to the client
     * @param message Message to send
     */
    void sendMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Check if the client is connected
     * @return True if connected, false otherwise
     */
    bool isConnected() const;
    
    /**
     * @brief Get the client's account login
     * @return Account login
     */
    std::string getLogin() const;
    
    /**
     * @brief Set the client's account login
     * @param login Account login
     */
    void setLogin(const std::string& login);
    
    /**
     * @brief Get the client's IP address
     * @return IP address
     */
    std::string getIpAddress() const;
    
    /**
     * @brief Get the client's character ID
     * @return Character ID
     */
    uint32_t getCharacterId() const;
    
    /**
     * @brief Set the client's character
     * @param character Character
     */
    void setCharacter(const database::Character& character);
    
    /**
     * @brief Set the client's game session
     * @param session Game session
     */
    void setGameSession(std::shared_ptr<GameSession> session);
    
    /**
     * @brief Get the client's game session
     * @return Game session
     */
    std::shared_ptr<GameSession> getGameSession() const;
    
    /**
     * @brief Get the time the client connected
     * @return Connection time
     */
    std::time_t getConnectTime() const;
    
    /**
     * @brief Get the last activity time
     * @return Last activity time
     */
    std::time_t getLastActivityTime() const;
    
    /**
     * @brief Disconnect the client
     */
    void disconnect();
    
    /**
     * @brief Update the client
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);
    
private:
    /**
     * @brief Read message header from the client
     */
    void readHeader();
    
    /**
     * @brief Read message body from the client
     * @param messageSize Size of the message body
     */
    void readBody(uint32_t messageSize);
    
    /**
     * @brief Process a received message
     * @param message Message data
     */
    void processMessage(const std::vector<uint8_t>& message);
    
    /**
     * @brief Write next message in the queue
     */
    void writeNextMessage();
    
    /**
     * @brief Update the last activity time
     */
    void updateLastActivityTime();
    
    /**
     * @brief Handle authentication
     * @param token Authentication token
     * @param characterId Character ID
     * @return True if authentication succeeded, false otherwise
     */
    bool handleAuth(const std::string& token, uint32_t characterId);
    
    boost::asio::ip::tcp::socket m_socket;
    GameServer& m_server;
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    
    std::atomic<bool> m_connected;
    std::atomic<bool> m_authenticated;
    std::string m_login;
    std::string m_ipAddress;
    uint32_t m_characterId;
    std::string m_characterName;
    int m_characterClass;
    
    std::time_t m_connectTime;
    std::time_t m_lastActivityTime;
    
    std::weak_ptr<GameSession> m_gameSession;
    
    std::vector<uint8_t> m_headerBuffer;
    std::vector<uint8_t> m_bodyBuffer;
    
    std::mutex m_writeMutex;
    std::queue<std::vector<uint8_t>> m_writeQueue;
    bool m_writing;
};

} // namespace game_server
} // namespace d3server 