#include "utils/file_utils.h"
#include "utils/debug.h"
#include "utils/string_utils.h"

#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace d3server {
namespace utils {

bool FileUtils::fileExists(const std::string& filePath) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception checking if file exists: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::directoryExists(const std::string& dirPath) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(dirPath);
    
    try {
        return fs::exists(dirPath) && fs::is_directory(dirPath);
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception checking if directory exists: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::createDirectory(const std::string& dirPath, bool recursive) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(dirPath);
    DEBUG_VARIABLE(recursive);
    
    try {
        if (directoryExists(dirPath)) {
            DEBUG_LOG("Directory already exists");
            return true;
        }
        
        if (recursive) {
            return fs::create_directories(dirPath);
        }
        else {
            return fs::create_directory(dirPath);
        }
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception creating directory: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::readFile(const std::string& filePath, std::string& content) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        std::ifstream file(filePath, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            DEBUG_LOG("Failed to open file for reading");
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        content = buffer.str();
        
        file.close();
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception reading file: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::readFileLines(const std::string& filePath, std::vector<std::string>& lines) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            DEBUG_LOG("Failed to open file for reading lines");
            return false;
        }
        
        lines.clear();
        std::string line;
        
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        
        file.close();
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception reading file lines: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::writeFile(const std::string& filePath, const std::string& content, bool append) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    DEBUG_VARIABLE(append);
    
    try {
        // Create directory if it doesn't exist
        std::string dirName = getDirname(filePath);
        if (!dirName.empty() && !directoryExists(dirName)) {
            if (!createDirectory(dirName)) {
                DEBUG_LOG("Failed to create directory for file");
                return false;
            }
        }
        
        std::ios_base::openmode mode = std::ios::out | std::ios::binary;
        if (append) {
            mode |= std::ios::app;
        }
        
        std::ofstream file(filePath, mode);
        if (!file.is_open()) {
            DEBUG_LOG("Failed to open file for writing");
            return false;
        }
        
        file << content;
        file.close();
        
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception writing file: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::writeFileLines(const std::string& filePath, const std::vector<std::string>& lines, bool append) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    DEBUG_VARIABLE(append);
    
    try {
        // Create directory if it doesn't exist
        std::string dirName = getDirname(filePath);
        if (!dirName.empty() && !directoryExists(dirName)) {
            if (!createDirectory(dirName)) {
                DEBUG_LOG("Failed to create directory for file");
                return false;
            }
        }
        
        std::ios_base::openmode mode = std::ios::out;
        if (append) {
            mode |= std::ios::app;
        }
        
        std::ofstream file(filePath, mode);
        if (!file.is_open()) {
            DEBUG_LOG("Failed to open file for writing lines");
            return false;
        }
        
        for (const auto& line : lines) {
            file << line << std::endl;
        }
        
        file.close();
        
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception writing file lines: " + std::string(e.what()));
        return false;
    }
}

bool FileUtils::deleteFile(const std::string& filePath) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(filePath);
    
    try {
        return fs::remove(filePath);
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception deleting file: " + std::string(e.what()));
        return false;
    }
}

std::string FileUtils::getBasename(const std::string& path) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path);
    
    try {
        return fs::path(path).filename().string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception getting basename: " + std::string(e.what()));
        return "";
    }
}

std::string FileUtils::getDirname(const std::string& path) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path);
    
    try {
        return fs::path(path).parent_path().string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception getting dirname: " + std::string(e.what()));
        return "";
    }
}

std::string FileUtils::getExtension(const std::string& path) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path);
    
    try {
        return fs::path(path).extension().string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception getting extension: " + std::string(e.what()));
        return "";
    }
}

std::string FileUtils::normalizePath(const std::string& path) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path);
    
    try {
        return fs::path(path).lexically_normal().string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception normalizing path: " + std::string(e.what()));
        return path;
    }
}

std::string FileUtils::joinPath(const std::vector<std::string>& components) {
    DEBUG_FUNCTION_ENTER();
    
    if (components.empty()) {
        return "";
    }
    
    try {
        fs::path result(components[0]);
        
        for (size_t i = 1; i < components.size(); ++i) {
            result /= components[i];
        }
        
        return result.string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception joining paths: " + std::string(e.what()));
        return "";
    }
}

std::string FileUtils::joinPath(const std::string& path1, const std::string& path2) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(path1);
    DEBUG_VARIABLE(path2);
    
    try {
        return (fs::path(path1) / fs::path(path2)).string();
    }
    catch (const std::exception& e) {
        DEBUG_LOG("Exception joining paths: " + std::string(e.what()));
        return "";
    }
}

} // namespace utils
} // namespace d3server 