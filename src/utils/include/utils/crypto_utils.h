#pragma once

#include <string>
#include <vector>

namespace d3server {
namespace utils {

/**
 * @brief Utility class for cryptography operations
 */
class CryptoUtils {
public:
    /**
     * @brief Compute SHA-1 hash of a string
     * @param data Input data
     * @return SHA-1 hash as hex string
     */
    static std::string sha1(const std::string& data);

    /**
     * @brief Compute SHA-256 hash of a string
     * @param data Input data
     * @return SHA-256 hash as hex string
     */
    static std::string sha256(const std::string& data);

    /**
     * @brief Compute MD5 hash of a string
     * @param data Input data
     * @return MD5 hash as hex string
     */
    static std::string md5(const std::string& data);

    /**
     * @brief Generate a random string
     * @param length Length of the string to generate
     * @param includeSpecialChars Whether to include special characters
     * @return Random string
     */
    static std::string generateRandomString(size_t length, bool includeSpecialChars = false);

    /**
     * @brief Generate a secure random bytes
     * @param length Length of the bytes to generate
     * @return Random bytes as vector
     */
    static std::vector<uint8_t> generateRandomBytes(size_t length);

    /**
     * @brief Base64 encode a string
     * @param data Input data
     * @return Base64 encoded string
     */
    static std::string base64Encode(const std::string& data);

    /**
     * @brief Base64 decode a string
     * @param data Base64 encoded string
     * @return Decoded string
     */
    static std::string base64Decode(const std::string& data);

    /**
     * @brief Encrypt a string using AES-256
     * @param data String to encrypt
     * @param key Encryption key
     * @param iv Initialization vector
     * @return Encrypted string
     */
    static std::string aesEncrypt(const std::string& data, const std::string& key, const std::string& iv);

    /**
     * @brief Decrypt a string using AES-256
     * @param data String to decrypt
     * @param key Encryption key
     * @param iv Initialization vector
     * @return Decrypted string
     */
    static std::string aesDecrypt(const std::string& data, const std::string& key, const std::string& iv);

    /**
     * @brief Hex encode a byte array
     * @param data Byte array to encode
     * @param length Length of the byte array
     * @return Hex encoded string
     */
    static std::string hexEncode(const uint8_t* data, size_t length);

    /**
     * @brief Hex decode a string
     * @param hexString Hex encoded string
     * @return Decoded byte vector
     */
    static std::vector<uint8_t> hexDecode(const std::string& hexString);

    /**
     * @brief Hash a password with a salt
     * @param password Password to hash
     * @param salt Salt to use (if empty, a random salt will be generated)
     * @return Hashed password with salt
     */
    static std::string hashPassword(const std::string& password, const std::string& salt = "");

    /**
     * @brief Verify a password against a hash, using a provided salt
     * @param password Password to verify
     * @param salt Salt used during hashing
     * @param passwordHash Password hash to verify against (should not contain the salt)
     * @return True if the password matches the hash, false otherwise
     */
    static bool verifyPassword(const std::string& password, const std::string& salt, const std::string& passwordHash);
};

} // namespace utils
} // namespace d3server 