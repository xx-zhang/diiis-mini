#include "battle_net/battle_net_client.h"
#include "battle_net/battle_net_server.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/crypto_utils.h"
#include "utils/audit.h"
#include "database/database_manager.h"

#include <chrono>
#include <functional>
#include <sstream>
#include <iomanip>

namespace d3server {
namespace battle_net {

// Header size for Battle.net messages
constexpr size_t HEADER_SIZE = 4;

BattleNetClient::BattleNetClient(
    boost::asio::ip::tcp::socket socket,
    BattleNetServer& server,
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_socket(std::move(socket)),
    m_server(server),
    m_config(config),
    m_dbManager(dbManager),
    m_connected(true),
    m_authState(AuthState::NotAuthenticated),
    m_headerBuffer(HEADER_SIZE),
    m_writing(false)
{
    DEBUG_FUNCTION_ENTER();
    
    // Get client IP address
    try {
        m_ipAddress = m_socket.remote_endpoint().address().to_string();
    }
    catch (const std::exception& e) {
        m_ipAddress = "unknown";
        DEBUG_LOG("Failed to get client IP address: " + std::string(e.what()));
    }
    
    // Set client connect time
    m_connectTime = std::time(nullptr);
    m_lastActivityTime = m_connectTime;
    
    LOG_INFO("Battle.net client created from " + m_ipAddress);
    
    // 记录审计日志：客户端连接
    AUDIT_LOG("SYSTEM", m_ipAddress, utils::AuditActionType::NETWORK,
              "Client Connection", utils::AuditResult::SUCCESS,
              "Battle.net client connected");
              
    DEBUG_FUNCTION_EXIT();
}

BattleNetClient::~BattleNetClient() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_connected) {
        disconnect();
    }
    
    LOG_INFO("Battle.net client destroyed from " + m_ipAddress);
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::start() {
    DEBUG_FUNCTION_ENTER();
    
    // Start reading messages
    readHeader();
    
    // Send welcome message
    std::vector<uint8_t> welcomeMsg = { 0x01, 0x00, 0x00, 0x00, 0x00 };
    sendMessage(welcomeMsg);
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::sendMessage(const std::vector<uint8_t>& message) {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_LOG("Cannot send message, client disconnected");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    bool shouldWrite = false;
    
    {
        std::lock_guard<std::mutex> lock(m_writeMutex);
        shouldWrite = m_writeQueue.empty() && !m_writing;
        m_writeQueue.push(message);
    }
    
    if (shouldWrite) {
        writeNextMessage();
    }
    
    DEBUG_FUNCTION_EXIT();
}

bool BattleNetClient::isConnected() const {
    return m_connected;
}

AuthState BattleNetClient::getAuthState() const {
    return m_authState;
}

std::string BattleNetClient::getLogin() const {
    return m_login;
}

std::string BattleNetClient::getIpAddress() const {
    return m_ipAddress;
}

std::time_t BattleNetClient::getConnectTime() const {
    return m_connectTime;
}

std::time_t BattleNetClient::getLastActivityTime() const {
    return m_lastActivityTime;
}

void BattleNetClient::readHeader() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_LOG("Cannot read header, client disconnected");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    auto self = shared_from_this();
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_headerBuffer),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                // Update activity time
                updateLastActivityTime();
                
                // Extract message size from header
                uint32_t messageSize = 0;
                messageSize |= m_headerBuffer[0];
                messageSize |= (m_headerBuffer[1] << 8);
                messageSize |= (m_headerBuffer[2] << 16);
                messageSize |= (m_headerBuffer[3] << 24);
                
                // Check message size
                if (messageSize <= 0 || messageSize > 1024 * 1024) {
                    // Invalid message size, disconnect
                    LOG_WARNING("Invalid message size: " + std::to_string(messageSize));
                    disconnect();
                    return;
                }
                
                // Read message body
                readBody(messageSize);
            } else if (ec != boost::asio::error::operation_aborted) {
                if (ec == boost::asio::error::eof) {
                    LOG_INFO("Client disconnected: " + m_ipAddress);
                } else {
                    LOG_ERROR("Error reading header: " + ec.message());
                }
                disconnect();
            }
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::readBody(uint32_t messageSize) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(messageSize);
    
    if (!m_connected) {
        DEBUG_LOG("Cannot read body, client disconnected");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    // Resize body buffer
    m_bodyBuffer.resize(messageSize);
    
    auto self = shared_from_this();
    boost::asio::async_read(
        m_socket,
        boost::asio::buffer(m_bodyBuffer),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                // Update activity time
                updateLastActivityTime();
                
                // Process message
                processMessage(m_bodyBuffer);
                
                // Start reading next message
                readHeader();
            } else if (ec != boost::asio::error::operation_aborted) {
                if (ec == boost::asio::error::eof) {
                    LOG_INFO("Client disconnected: " + m_ipAddress);
                } else {
                    LOG_ERROR("Error reading body: " + ec.message());
                }
                disconnect();
            }
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::processMessage(const std::vector<uint8_t>& message) {
    DEBUG_FUNCTION_ENTER();
    
    if (message.empty()) {
        LOG_WARNING("Received empty message");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    // First byte is the message type
    uint8_t messageType = message[0];
    
    DEBUG_VARIABLE(messageType);
    
    // Handle message based on type
    switch (messageType) {
        case 0x01: // Authentication request
            if (message.size() >= 3) {
                // Simple authentication flow for demo
                // In a real implementation, this would be more complex
                
                // Extract login and password
                std::string login, password;
                
                // Parse login (assuming login length is at message[1])
                if (message.size() >= 2 + message[1]) {
                    login.assign(message.begin() + 2, message.begin() + 2 + message[1]);
                }
                
                // Parse password (assuming password length is after login)
                if (message.size() >= 3 + message[1] + message[2 + message[1]]) {
                    password.assign(
                        message.begin() + 3 + message[1],
                        message.begin() + 3 + message[1] + message[2 + message[1]]
                    );
                }
                
                LOG_INFO("Authentication request from " + m_ipAddress + ", login: " + login);
                
                // Update auth state and handle authentication
                m_authState = AuthState::Authenticating;
                
                if (handleAuth(login, password)) {
                    m_authState = AuthState::Authenticated;
                    m_login = login;
                    
                    // Send auth success
                    std::vector<uint8_t> authSuccessMsg = { 0x02, 0x01 };
                    sendMessage(authSuccessMsg);
                    
                    LOG_INFO("Authentication successful for " + login + " from " + m_ipAddress);
                } else {
                    m_authState = AuthState::AuthFailed;
                    
                    // Send auth failed
                    std::vector<uint8_t> authFailedMsg = { 0x02, 0x00 };
                    sendMessage(authFailedMsg);
                    
                    LOG_WARNING("Authentication failed for " + login + " from " + m_ipAddress);
                }
            } else {
                LOG_WARNING("Invalid authentication request");
            }
            break;
            
        case 0x03: // Ping
            // Send pong
            std::vector<uint8_t> pongMsg = { 0x04 };
            sendMessage(pongMsg);
            break;
            
        case 0x05: // Character list request
            if (m_authState == AuthState::Authenticated) {
                // In a real implementation, this would fetch characters from the database
                LOG_INFO("Character list requested by " + m_login);
                
                // For now, just send an empty character list
                std::vector<uint8_t> characterListMsg = { 0x06, 0x00, 0x00 };
                sendMessage(characterListMsg);
            } else {
                LOG_WARNING("Character list requested but client not authenticated");
                disconnect();
            }
            break;
            
        default:
            LOG_WARNING("Unknown message type: " + std::to_string(messageType));
            break;
    }
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::updateLastActivityTime() {
    m_lastActivityTime = std::time(nullptr);
}

void BattleNetClient::disconnect() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_connected = false;
    
    try {
        if (m_socket.is_open()) {
            boost::system::error_code ec;
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            m_socket.close(ec);
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during client disconnect: " + std::string(e.what()));
    }
    
    LOG_INFO("Client disconnected: " + m_ipAddress);
    
    // 记录审计日志：客户端断开连接
    AUDIT_LOG(m_login.empty() ? "UNKNOWN" : m_login, m_ipAddress, 
              utils::AuditActionType::NETWORK, "Client Disconnection",
              utils::AuditResult::SUCCESS, "Battle.net client disconnected");
              
    DEBUG_FUNCTION_EXIT();
}

bool BattleNetClient::handleAuth(const std::string& login, const std::string& password) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(login);
    
    try {
        // Check if account exists
        if (!m_dbManager->accountExists(login)) {
            // 记录审计日志：账号不存在
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                     "Login Attempt", utils::AuditResult::FAILURE,
                     "Account does not exist");
                     
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Check if account is banned
        if (m_dbManager->isAccountBanned(login)) {
            LOG_WARNING("Login attempt on banned account: " + login + " from " + m_ipAddress);
            
            // 记录审计日志：尝试登录被封禁账号
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                     "Login Attempt", utils::AuditResult::UNAUTHORIZED,
                     "Account is banned");
                     
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Verify password
        bool success = m_dbManager->verifyAccountPassword(login, password);
        
        if (success) {
            // Update last login time
            m_dbManager->updateAccountLastLogin(login);
            
            // 记录审计日志：登录成功
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                     "Login Success", utils::AuditResult::SUCCESS,
                     "Authentication successful");
        } else {
            // 记录审计日志：密码错误
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                     "Login Failure", utils::AuditResult::FAILURE,
                     "Invalid password");
        }
        
        DEBUG_FUNCTION_EXIT();
        return success;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during authentication: " + std::string(e.what()));
        
        // 记录审计日志：认证过程发生异常
        AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                 "Login Error", utils::AuditResult::FAILURE,
                 "Exception: " + std::string(e.what()));
                 
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void BattleNetClient::writeNextMessage() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_connected) {
        DEBUG_LOG("Cannot write message, client disconnected");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_writeMutex);
    
    if (m_writeQueue.empty() || m_writing) {
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_writing = true;
    
    // Get next message
    std::vector<uint8_t> message = m_writeQueue.front();
    m_writeQueue.pop();
    
    // Prepare header (4 bytes, little-endian)
    std::vector<uint8_t> header(HEADER_SIZE);
    uint32_t messageSize = static_cast<uint32_t>(message.size());
    header[0] = messageSize & 0xFF;
    header[1] = (messageSize >> 8) & 0xFF;
    header[2] = (messageSize >> 16) & 0xFF;
    header[3] = (messageSize >> 24) & 0xFF;
    
    // Combine header and message
    std::vector<uint8_t> packet;
    packet.reserve(header.size() + message.size());
    packet.insert(packet.end(), header.begin(), header.end());
    packet.insert(packet.end(), message.begin(), message.end());
    
    // Write to socket
    auto self = shared_from_this();
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(packet),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            std::lock_guard<std::mutex> lock(m_writeMutex);
            
            m_writing = false;
            
            if (!ec) {
                // Write next message if any
                if (!m_writeQueue.empty()) {
                    writeNextMessage();
                }
            } else if (ec != boost::asio::error::operation_aborted) {
                LOG_ERROR("Error writing message: " + ec.message());
                disconnect();
            }
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void BattleNetClient::setAuthState(AuthState state) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(static_cast<int>(state));
    
    AuthState oldState = m_authState;
    m_authState = state;
    
    // 记录审计日志：认证状态变更
    if (oldState != state) {
        std::string stateStr;
        switch (state) {
            case AuthState::NotAuthenticated: stateStr = "NotAuthenticated"; break;
            case AuthState::InProgress: stateStr = "InProgress"; break;
            case AuthState::Authenticated: stateStr = "Authenticated"; break;
            case AuthState::AuthFailed: stateStr = "AuthFailed"; break;
            default: stateStr = "Unknown"; break;
        }
        
        utils::AuditResult auditResult;
        switch (state) {
            case AuthState::Authenticated: 
                auditResult = utils::AuditResult::SUCCESS; 
                break;
            case AuthState::AuthFailed: 
                auditResult = utils::AuditResult::FAILURE; 
                break;
            default: 
                auditResult = utils::AuditResult::WARNING; 
                break;
        }
        
        AUDIT_LOG(m_login.empty() ? "UNKNOWN" : m_login, m_ipAddress,
                 utils::AuditActionType::AUTHENTICATION, "Auth State Changed",
                 auditResult, "New state: " + stateStr);
    }
    
    DEBUG_FUNCTION_EXIT();
}

// 处理认证相关消息的函数
void BattleNetClient::handleAuthMessage(const std::vector<uint8_t>& message) {
    DEBUG_FUNCTION_ENTER();
    
    try {
        if (message.size() < 2) {
            LOG_WARNING("Auth message too short from " + m_ipAddress);
            
            // 记录审计日志：无效的认证消息
            AUDIT_LOG("UNKNOWN", m_ipAddress, utils::AuditActionType::SECURITY,
                     "Invalid Auth Message", utils::AuditResult::WARNING,
                     "Message too short");
                     
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Extract login length
        uint8_t loginLength = message[1];
        if (message.size() < 2 + loginLength) {
            LOG_WARNING("Auth message missing login from " + m_ipAddress);
            
            // 记录审计日志：无效的认证消息
            AUDIT_LOG("UNKNOWN", m_ipAddress, utils::AuditActionType::SECURITY,
                     "Invalid Auth Message", utils::AuditResult::WARNING,
                     "Missing login data");
                     
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Extract login
        std::string login(message.begin() + 2, message.begin() + 2 + loginLength);
        
        // Extract password length
        if (message.size() < 2 + loginLength + 1) {
            LOG_WARNING("Auth message missing password length from " + m_ipAddress);
            
            // 记录审计日志：无效的认证消息
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::SECURITY,
                     "Invalid Auth Message", utils::AuditResult::WARNING,
                     "Missing password length");
                     
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        uint8_t passwordLength = message[2 + loginLength];
        if (message.size() < 2 + loginLength + 1 + passwordLength) {
            LOG_WARNING("Auth message missing password from " + m_ipAddress);
            
            // 记录审计日志：无效的认证消息
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::SECURITY,
                     "Invalid Auth Message", utils::AuditResult::WARNING,
                     "Missing password data");
                     
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Extract password
        std::string password(message.begin() + 2 + loginLength + 1, 
                            message.begin() + 2 + loginLength + 1 + passwordLength);
        
        // Set state to in progress
        setAuthState(AuthState::InProgress);
        
        // 记录审计日志：开始认证过程
        AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::AUTHENTICATION,
                 "Authentication Started", utils::AuditResult::WARNING,
                 "Authentication in progress");
        
        // Authenticate
        bool authSuccess = handleAuth(login, password);
        
        // Set login and auth state
        if (authSuccess) {
            m_login = login;
            setAuthState(AuthState::Authenticated);
            
            // 记录获取角色操作
            AUDIT_LOG(login, m_ipAddress, utils::AuditActionType::CHARACTER_MANAGEMENT,
                     "Characters Requested", utils::AuditResult::SUCCESS,
                     "Character list requested after successful login");
        } else {
            m_login.clear();
            setAuthState(AuthState::AuthFailed);
        }
        
        // Send response
        std::vector<uint8_t> response = { 0x02, authSuccess ? (uint8_t)0x01 : (uint8_t)0x00 };
        sendMessage(response);
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception handling auth message: " + std::string(e.what()));
        
        // 记录审计日志：处理认证消息时发生异常
        AUDIT_LOG("UNKNOWN", m_ipAddress, utils::AuditActionType::SECURITY,
                 "Auth Message Error", utils::AuditResult::FAILURE,
                 "Exception: " + std::string(e.what()));
    }
    
    DEBUG_FUNCTION_EXIT();
}

} // namespace battle_net
} // namespace d3server 