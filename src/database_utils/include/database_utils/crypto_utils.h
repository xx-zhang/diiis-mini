#ifndef D3SERVER_CRYPTO_UTILS_H
#define D3SERVER_CRYPTO_UTILS_H

#include <string>
#include <vector>
#include <random>
#include <iomanip>
#include <sstream>

namespace d3server {
namespace database_utils {

/**
 * @brief Utility class for cryptographic operations
 */
class CryptoUtils {
public:
    /**
     * @brief Generate a random salt for password hashing
     * @param length Length of the salt to generate
     * @return Random salt as a hex string
     */
    static std::string generateSalt(size_t length = 16) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::vector<unsigned char> salt(length);
        for (size_t i = 0; i < length; ++i) {
            salt[i] = static_cast<unsigned char>(dis(gen));
        }
        
        return bytesToHex(salt);
    }
    
    /**
     * @brief Hash a password using SHA-256 with a salt
     * @param password Password to hash
     * @param salt Salt to use for hashing
     * @return Hashed password as a hex string
     */
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        // In a real implementation, use a proper crypto library like OpenSSL
        // This is a placeholder implementation
        std::string combined = salt + password;
        
        // Simple hash function for demonstration
        // Replace with proper SHA-256 in production
        std::hash<std::string> hasher;
        size_t hash = hasher(combined);
        
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash;
        return ss.str();
    }
    
    /**
     * @brief Verify a password against a stored hash
     * @param password Password to verify
     * @param storedHash Stored hash to compare against
     * @param salt Salt used for hashing
     * @return True if password matches, false otherwise
     */
    static bool verifyPassword(const std::string& password, const std::string& storedHash, const std::string& salt) {
        std::string computedHash = hashPassword(password, salt);
        return computedHash == storedHash;
    }
    
private:
    /**
     * @brief Convert bytes to a hex string
     * @param bytes Bytes to convert
     * @return Hex string representation
     */
    static std::string bytesToHex(const std::vector<unsigned char>& bytes) {
        std::stringstream ss;
        for (unsigned char b : bytes) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
        }
        return ss.str();
    }
};

} // namespace database_utils
} // namespace d3server

#endif // D3SERVER_CRYPTO_UTILS_H 
