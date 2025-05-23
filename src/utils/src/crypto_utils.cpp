#include "utils/crypto_utils.h"
#include "utils/debug.h"

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/err.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace d3server {
namespace utils {

std::string CryptoUtils::sha1(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    uint8_t hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), hash);
    
    std::string result = hexEncode(hash, SHA_DIGEST_LENGTH);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::sha256(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), hash);
    
    std::string result = hexEncode(hash, SHA256_DIGEST_LENGTH);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::md5(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    uint8_t hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const uint8_t*>(data.c_str()), data.length(), hash);
    
    std::string result = hexEncode(hash, MD5_DIGEST_LENGTH);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::generateRandomString(size_t length, bool includeSpecialChars) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(length);
    DEBUG_VARIABLE(includeSpecialChars);
    
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    
    static const char specialChars[] = "!@#$%^&*()-_=+[]{};:,.<>?";
    
    std::string charset = alphanum;
    if (includeSpecialChars) {
        charset += specialChars;
    }
    
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, charset.size() - 1);
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset[distribution(generator)];
    }
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::vector<uint8_t> CryptoUtils::generateRandomBytes(size_t length) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(length);
    
    std::vector<uint8_t> buffer(length);
    
    if (RAND_bytes(buffer.data(), static_cast<int>(length)) != 1) {
        DEBUG_LOG("RAND_bytes failed, falling back to std::random_device");
        
        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<> distribution(0, 255);
        
        for (size_t i = 0; i < length; ++i) {
            buffer[i] = static_cast<uint8_t>(distribution(generator));
        }
    }
    
    DEBUG_FUNCTION_EXIT();
    return buffer;
}

std::string CryptoUtils::base64Encode(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, bmem);
    BIO_write(b64, data.c_str(), static_cast<int>(data.length()));
    BIO_flush(b64);
    
    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);
    
    std::string result(bptr->data, bptr->length - 1);  // -1 to remove the trailing newline
    
    BIO_free_all(b64);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::base64Decode(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bmem = BIO_new_mem_buf(data.c_str(), static_cast<int>(data.length()));
    bmem = BIO_push(b64, bmem);
    
    std::vector<char> buffer(data.length());
    int decodedLength = BIO_read(bmem, buffer.data(), static_cast<int>(data.length()));
    
    BIO_free_all(bmem);
    
    std::string result(buffer.data(), decodedLength > 0 ? decodedLength : 0);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::aesEncrypt(const std::string& data, const std::string& key, const std::string& iv) {
    DEBUG_FUNCTION_ENTER();
    
    // Validate key and IV sizes
    if (key.length() != 32) {
        DEBUG_LOG("Invalid key size for AES-256");
        return "";
    }
    
    if (iv.length() != 16) {
        DEBUG_LOG("Invalid IV size for AES-256");
        return "";
    }
    
    // Initialize context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        DEBUG_LOG("Failed to create EVP_CIPHER_CTX");
        return "";
    }
    
    // Initialize encryption operation
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                         reinterpret_cast<const uint8_t*>(key.c_str()),
                         reinterpret_cast<const uint8_t*>(iv.c_str())) != 1) {
        DEBUG_LOG("Failed to initialize encryption");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    // Prepare output buffer
    std::vector<uint8_t> ciphertext(data.length() + AES_BLOCK_SIZE);
    int ciphertextLength = 0;
    
    // Encrypt data
    if (EVP_EncryptUpdate(ctx,
                        ciphertext.data(), &ciphertextLength,
                        reinterpret_cast<const uint8_t*>(data.c_str()), static_cast<int>(data.length())) != 1) {
        DEBUG_LOG("Failed during encryption update");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    int finalLength = 0;
    if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + ciphertextLength, &finalLength) != 1) {
        DEBUG_LOG("Failed during encryption finalization");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    ciphertextLength += finalLength;
    EVP_CIPHER_CTX_free(ctx);
    
    // Return base64-encoded ciphertext
    std::string result = base64Encode(std::string(reinterpret_cast<char*>(ciphertext.data()), ciphertextLength));
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::aesDecrypt(const std::string& data, const std::string& key, const std::string& iv) {
    DEBUG_FUNCTION_ENTER();
    
    // Validate key and IV sizes
    if (key.length() != 32) {
        DEBUG_LOG("Invalid key size for AES-256");
        return "";
    }
    
    if (iv.length() != 16) {
        DEBUG_LOG("Invalid IV size for AES-256");
        return "";
    }
    
    // Decode base64
    std::string ciphertext = base64Decode(data);
    
    // Initialize context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        DEBUG_LOG("Failed to create EVP_CIPHER_CTX");
        return "";
    }
    
    // Initialize decryption operation
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr,
                         reinterpret_cast<const uint8_t*>(key.c_str()),
                         reinterpret_cast<const uint8_t*>(iv.c_str())) != 1) {
        DEBUG_LOG("Failed to initialize decryption");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    // Prepare output buffer
    std::vector<uint8_t> plaintext(ciphertext.length());
    int plaintextLength = 0;
    
    // Decrypt data
    if (EVP_DecryptUpdate(ctx,
                        plaintext.data(), &plaintextLength,
                        reinterpret_cast<const uint8_t*>(ciphertext.c_str()), static_cast<int>(ciphertext.length())) != 1) {
        DEBUG_LOG("Failed during decryption update");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    int finalLength = 0;
    if (EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintextLength, &finalLength) != 1) {
        DEBUG_LOG("Failed during decryption finalization");
        EVP_CIPHER_CTX_free(ctx);
        return "";
    }
    
    plaintextLength += finalLength;
    EVP_CIPHER_CTX_free(ctx);
    
    std::string result(reinterpret_cast<char*>(plaintext.data()), plaintextLength);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::hexEncode(const uint8_t* data, size_t length) {
    DEBUG_FUNCTION_ENTER();
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
    }
    
    DEBUG_FUNCTION_EXIT();
    return ss.str();
}

std::vector<uint8_t> CryptoUtils::hexDecode(const std::string& hexString) {
    DEBUG_FUNCTION_ENTER();
    
    std::vector<uint8_t> result;
    for (size_t i = 0; i < hexString.length(); i += 2) {
        std::string byteString = hexString.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoi(byteString, nullptr, 16));
        result.push_back(byte);
    }
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string CryptoUtils::hashPassword(const std::string& password, const std::string& salt) {
    DEBUG_FUNCTION_ENTER();
    
    // Generate a random salt if none provided
    std::string useSalt = salt.empty() 
        ? base64Encode(std::string(reinterpret_cast<char*>(generateRandomBytes(16).data()), 16))
        : salt;
    
    // Combine password and salt
    std::string combined = password + "$" + useSalt;
    
    // Hash using SHA-256
    std::string hash = sha256(combined);
    
    // Format as salt$hash
    std::string result = useSalt + "$" + hash;
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

bool CryptoUtils::verifyPassword(const std::string& password, const std::string& passwordHash) {
    DEBUG_FUNCTION_ENTER();
    
    // Split hash into salt and hash
    size_t separatorPos = passwordHash.find('$');
    if (separatorPos == std::string::npos) {
        DEBUG_LOG("Invalid password hash format");
        return false;
    }
    
    std::string salt = passwordHash.substr(0, separatorPos);
    std::string expectedHash = passwordHash.substr(separatorPos + 1);
    
    // Combine password and salt
    std::string combined = password + "$" + salt;
    
    // Hash
    std::string computedHash = sha256(combined);
    
    // Compare
    bool result = (computedHash == expectedHash);
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

} // namespace utils
} // namespace d3server 