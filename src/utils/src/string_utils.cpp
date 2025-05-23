#include "utils/string_utils.h"
#include "utils/debug.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace d3server {
namespace utils {

std::string StringUtils::trim(const std::string& str) {
    return trimRight(trimLeft(str));
}

std::string StringUtils::trimLeft(const std::string& str) {
    auto it = std::find_if(str.begin(), str.end(), [](char ch) {
        return !std::isspace(static_cast<unsigned char>(ch));
    });
    return std::string(it, str.end());
}

std::string StringUtils::trimRight(const std::string& str) {
    auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) {
        return !std::isspace(static_cast<unsigned char>(ch));
    }).base();
    return std::string(str.begin(), it);
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    DEBUG_FUNCTION_ENTER();
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    DEBUG_FUNCTION_EXIT();
    return tokens;
}

std::string StringUtils::join(const std::vector<std::string>& strings, const std::string& delimiter) {
    DEBUG_FUNCTION_ENTER();
    std::ostringstream result;
    
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) {
            result << delimiter;
        }
        result << strings[i];
    }
    
    DEBUG_FUNCTION_EXIT();
    return result.str();
}

std::string StringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

std::string StringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return std::toupper(c);
    });
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }
    return str.compare(0, prefix.length(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

std::string StringUtils::replace(const std::string& str, const std::string& from, const std::string& to) {
    DEBUG_FUNCTION_ENTER();
    std::string result = str;
    size_t startPos = 0;
    
    while ((startPos = result.find(from, startPos)) != std::string::npos) {
        result.replace(startPos, from.length(), to);
        startPos += to.length();
    }
    
    DEBUG_FUNCTION_EXIT();
    return result;
}

std::string StringUtils::getCurrentTimeString(const std::string& format) {
    DEBUG_FUNCTION_ENTER();
    
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_c), format.c_str());
    
    DEBUG_FUNCTION_EXIT();
    return ss.str();
}

} // namespace utils
} // namespace d3server 