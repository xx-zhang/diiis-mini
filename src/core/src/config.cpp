#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

#include <fstream>
#include <sstream>

namespace d3server {
namespace core {

Config::Config() {
    DEBUG_FUNCTION_ENTER();
    setDefaults();
    DEBUG_FUNCTION_EXIT();
}

Config::~Config() {
    DEBUG_FUNCTION_ENTER();
    DEBUG_FUNCTION_EXIT();
}

bool Config::loadFromFile(const std::string& filePath) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    // Check if file exists
    if (!utils::FileUtils::fileExists(filePath)) {
        LOG_WARNING("Configuration file not found: " + filePath + ", using defaults");
        DEBUG_FUNCTION_EXIT();
        return false;
    }
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open configuration file: " + filePath);
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        std::string line;
        std::string currentSection;
        
        // Read file line by line
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#' || line[0] == ';') {
                continue;
            }
            
            // Trim whitespace
            line = utils::StringUtils::trim(line);
            
            // Check for section
            if (line[0] == '[' && line[line.length() - 1] == ']') {
                currentSection = line.substr(1, line.length() - 2);
                continue;
            }
            
            // Parse key=value
            auto pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = utils::StringUtils::trim(line.substr(0, pos));
                std::string value = utils::StringUtils::trim(line.substr(pos + 1));
                
                // Store in config map
                m_config[currentSection][key] = value;
            }
        }
        
        file.close();
        
        // Parse configuration values into structured configs
        parseConfig();
        
        LOG_INFO("Configuration loaded from: " + filePath);
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception loading configuration: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

bool Config::saveToFile(const std::string& filePath) const {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            LOG_ERROR("Failed to open configuration file for writing: " + filePath);
            DEBUG_FUNCTION_EXIT();
            return false;
        }
        
        // Write configuration file header
        file << "# D3Server Configuration File" << std::endl;
        file << "# Generated on " << utils::StringUtils::getCurrentTimeString() << std::endl;
        file << std::endl;
        
        // Write sections and key-value pairs
        for (const auto& section : m_config) {
            // Skip empty sections
            if (section.second.empty()) {
                continue;
            }
            
            // Write section header
            file << "[" << section.first << "]" << std::endl;
            
            // Write key-value pairs
            for (const auto& keyValue : section.second) {
                file << keyValue.first << " = " << keyValue.second << std::endl;
            }
            
            file << std::endl;
        }
        
        file.close();
        
        LOG_INFO("Configuration saved to: " + filePath);
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception saving configuration: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

const DatabaseConfig& Config::getDatabaseConfig() const {
    return m_databaseConfig;
}

const NetworkConfig& Config::getNetworkConfig() const {
    return m_networkConfig;
}

const ServerConfig& Config::getServerConfig() const {
    return m_serverConfig;
}

void Config::setValue(const std::string& section, const std::string& key, const std::string& value) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(section);
    DEBUG_VARIABLE(key);
    DEBUG_VARIABLE(value);
    
    m_config[section][key] = value;
    parseConfig();
    
    DEBUG_FUNCTION_EXIT();
}

std::string Config::getValue(const std::string& section, const std::string& key, const std::string& defaultValue) const {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(section);
    DEBUG_VARIABLE(key);
    DEBUG_VARIABLE(defaultValue);
    
    // Check if section exists
    auto sectionIt = m_config.find(section);
    if (sectionIt != m_config.end()) {
        // Check if key exists
        auto keyIt = sectionIt->second.find(key);
        if (keyIt != sectionIt->second.end()) {
            DEBUG_FUNCTION_EXIT();
            return keyIt->second;
        }
    }
    
    DEBUG_FUNCTION_EXIT();
    return defaultValue;
}

void Config::parseConfig() {
    DEBUG_FUNCTION_ENTER();
    
    // Parse database configuration
    m_databaseConfig.type = getValue("Database", "Type", "sqlite");
    m_databaseConfig.filePath = getValue("Database", "FilePath", "d3server.db");
    m_databaseConfig.accountsDb = getValue("Database", "AccountsDB", "accounts");
    m_databaseConfig.worldsDb = getValue("Database", "WorldsDB", "worlds");
    
    // Parse network configuration
    m_networkConfig.bindIp = getValue("Network", "BindIP", "0.0.0.0");
    m_networkConfig.publicIp = getValue("Network", "PublicIP", "127.0.0.1");
    m_networkConfig.battleNetPort = std::stoi(getValue("Network", "BattleNetPort", "1119"));
    m_networkConfig.gameServerPort = std::stoi(getValue("Network", "GameServerPort", "1120"));
    m_networkConfig.restApiPort = std::stoi(getValue("Network", "RestApiPort", "8080"));
    m_networkConfig.enableSsl = (getValue("Network", "EnableSSL", "true") == "true");
    m_networkConfig.sslCertPath = getValue("Network", "SSLCertPath", "certs/server.crt");
    m_networkConfig.sslKeyPath = getValue("Network", "SSLKeyPath", "certs/server.key");
    
    // Parse server configuration
    m_serverConfig.serverName = getValue("Server", "ServerName", "D3Server");
    m_serverConfig.maxPlayers = std::stoi(getValue("Server", "MaxPlayers", "1000"));
    m_serverConfig.maxAccountsPerIp = std::stoi(getValue("Server", "MaxAccountsPerIP", "10"));
    m_serverConfig.maxCharactersPerAccount = std::stoi(getValue("Server", "MaxCharactersPerAccount", "10"));
    m_serverConfig.defaultLocale = getValue("Server", "DefaultLocale", "enUS");
    m_serverConfig.motd = getValue("Server", "MOTD", "Welcome to D3Server!");
    m_serverConfig.enableDebug = (getValue("Server", "EnableDebug", "false") == "true");
    m_serverConfig.logLevel = getValue("Server", "LogLevel", "INFO");
    m_serverConfig.logPath = getValue("Server", "LogPath", "logs/d3server.log");
    
    DEBUG_FUNCTION_EXIT();
}

void Config::setDefaults() {
    DEBUG_FUNCTION_ENTER();
    
    // Database defaults
    setValue("Database", "Type", "sqlite");
    setValue("Database", "FilePath", "d3server.db");
    setValue("Database", "AccountsDB", "accounts");
    setValue("Database", "WorldsDB", "worlds");
    
    // Network defaults
    setValue("Network", "BindIP", "0.0.0.0");
    setValue("Network", "PublicIP", "127.0.0.1");
    setValue("Network", "BattleNetPort", "1119");
    setValue("Network", "GameServerPort", "1120");
    setValue("Network", "RestApiPort", "8080");
    setValue("Network", "EnableSSL", "true");
    setValue("Network", "SSLCertPath", "certs/server.crt");
    setValue("Network", "SSLKeyPath", "certs/server.key");
    
    // Server defaults
    setValue("Server", "ServerName", "D3Server");
    setValue("Server", "MaxPlayers", "1000");
    setValue("Server", "MaxAccountsPerIP", "10");
    setValue("Server", "MaxCharactersPerAccount", "10");
    setValue("Server", "DefaultLocale", "enUS");
    setValue("Server", "MOTD", "Welcome to D3Server!");
    setValue("Server", "EnableDebug", "false");
    setValue("Server", "LogLevel", "INFO");
    setValue("Server", "LogPath", "logs/d3server.log");
    
    // Parse defaults into structured configs
    parseConfig();
    
    DEBUG_FUNCTION_EXIT();
}

} // namespace core
} // namespace d3server 
