#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace d3server {
namespace core {

/**
 * @brief Database configuration structure
 */
struct DatabaseConfig {
    std::string type;         // Database type (sqlite)
    std::string filePath;     // SQLite database file path
    std::string accountsDb;   // Account database name
    std::string worldsDb;     // Worlds database name
};

/**
 * @brief Network configuration structure
 */
struct NetworkConfig {
    std::string bindIp;       // IP address to bind to
    std::string publicIp;     // Public IP address
    int battleNetPort;        // Battle.net server port
    int gameServerPort;       // Game server port
    int restApiPort;          // REST API server port
    bool enableSsl;           // Whether to enable SSL
    std::string sslCertPath;  // Path to SSL certificate
    std::string sslKeyPath;   // Path to SSL private key
};

/**
 * @brief Server configuration structure
 */
struct ServerConfig {
    std::string serverName;         // Server name
    int maxPlayers;                 // Maximum number of players
    int maxAccountsPerIp;           // Maximum number of accounts per IP
    int maxCharactersPerAccount;    // Maximum number of characters per account
    std::string defaultLocale;      // Default locale
    std::string motd;               // Message of the day
    bool enableDebug;               // Whether debug mode is enabled
    std::string logLevel;           // Log level
    std::string logPath;            // Log file path
};

/**
 * @brief Configuration class for the server
 */
class Config {
public:
    /**
     * @brief Constructor
     */
    Config();

    /**
     * @brief Destructor
     */
    ~Config();

    /**
     * @brief Load configuration from a file
     * @param filePath Path to the configuration file
     * @return True if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& filePath);

    /**
     * @brief Save configuration to a file
     * @param filePath Path to the configuration file
     * @return True if saved successfully, false otherwise
     */
    bool saveToFile(const std::string& filePath) const;

    /**
     * @brief Get the database configuration
     * @return Database configuration
     */
    const DatabaseConfig& getDatabaseConfig() const;

    /**
     * @brief Get the network configuration
     * @return Network configuration
     */
    const NetworkConfig& getNetworkConfig() const;

    /**
     * @brief Get the server configuration
     * @return Server configuration
     */
    const ServerConfig& getServerConfig() const;

    /**
     * @brief Set a configuration value
     * @param section Configuration section
     * @param key Configuration key
     * @param value Configuration value
     */
    void setValue(const std::string& section, const std::string& key, const std::string& value);

    /**
     * @brief Get a configuration value
     * @param section Configuration section
     * @param key Configuration key
     * @param defaultValue Default value to return if not found
     * @return Configuration value or default value if not found
     */
    std::string getValue(const std::string& section, const std::string& key, const std::string& defaultValue = "") const;

private:
    /**
     * @brief Parse configuration values into structured config objects
     */
    void parseConfig();

    /**
     * @brief Set default configuration values
     */
    void setDefaults();

    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_config;
    DatabaseConfig m_databaseConfig;
    NetworkConfig m_networkConfig;
    ServerConfig m_serverConfig;
};

} // namespace core
} // namespace d3server 
