#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace d3server {
namespace utils {

/**
 * @brief Utility class for string operations
 */
class StringUtils {
public:
    /**
     * @brief Trim whitespace from the beginning and end of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trim(const std::string& str);

    /**
     * @brief Trim whitespace from the beginning of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trimLeft(const std::string& str);

    /**
     * @brief Trim whitespace from the end of a string
     * @param str String to trim
     * @return Trimmed string
     */
    static std::string trimRight(const std::string& str);

    /**
     * @brief Split a string into tokens using a delimiter
     * @param str String to split
     * @param delimiter Delimiter to split by
     * @return Vector of tokens
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);

    /**
     * @brief Join a vector of strings using a delimiter
     * @param strings Vector of strings to join
     * @param delimiter Delimiter to join with
     * @return Joined string
     */
    static std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

    /**
     * @brief Convert a string to lowercase
     * @param str String to convert
     * @return Lowercase string
     */
    static std::string toLower(const std::string& str);

    /**
     * @brief Convert a string to uppercase
     * @param str String to convert
     * @return Uppercase string
     */
    static std::string toUpper(const std::string& str);

    /**
     * @brief Check if a string starts with a prefix
     * @param str String to check
     * @param prefix Prefix to check for
     * @return True if the string starts with the prefix, false otherwise
     */
    static bool startsWith(const std::string& str, const std::string& prefix);

    /**
     * @brief Check if a string ends with a suffix
     * @param str String to check
     * @param suffix Suffix to check for
     * @return True if the string ends with the suffix, false otherwise
     */
    static bool endsWith(const std::string& str, const std::string& suffix);

    /**
     * @brief Replace all occurrences of a substring in a string
     * @param str String to modify
     * @param from Substring to replace
     * @param to Replacement substring
     * @return Modified string
     */
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);

    /**
     * @brief Get current time as a formatted string
     * @param format Format string (default: "%Y-%m-%d %H:%M:%S")
     * @return Formatted time string
     */
    static std::string getCurrentTimeString(const std::string& format = "%Y-%m-%d %H:%M:%S");

    // Converts an integer to its hexadecimal string representation
    template<typename T>
    static std::string toHex(T value) {
        std::stringstream ss;
        ss << std::hex << std::setw(sizeof(T) * 2) << std::setfill('0') << static_cast<int>(value);
        return ss.str();
    }
};

} // namespace utils
} // namespace d3server 