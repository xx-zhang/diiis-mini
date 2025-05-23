#pragma once

#include <string>
#include <vector>

namespace d3server {
namespace utils {

/**
 * @brief Utility class for file operations
 */
class FileUtils {
public:
    /**
     * @brief Check if a file exists
     * @param filePath Path to the file
     * @return True if the file exists, false otherwise
     */
    static bool fileExists(const std::string& filePath);

    /**
     * @brief Check if a directory exists
     * @param dirPath Path to the directory
     * @return True if the directory exists, false otherwise
     */
    static bool directoryExists(const std::string& dirPath);

    /**
     * @brief Create a directory
     * @param dirPath Path to the directory
     * @param recursive Whether to create parent directories if they don't exist
     * @return True if the directory was created or already exists, false otherwise
     */
    static bool createDirectory(const std::string& dirPath, bool recursive = true);

    /**
     * @brief Read a file into a string
     * @param filePath Path to the file
     * @param content Output variable for the file content
     * @return True if the file was read successfully, false otherwise
     */
    static bool readFile(const std::string& filePath, std::string& content);

    /**
     * @brief Read a file into a vector of lines
     * @param filePath Path to the file
     * @param lines Output variable for the file lines
     * @return True if the file was read successfully, false otherwise
     */
    static bool readFileLines(const std::string& filePath, std::vector<std::string>& lines);

    /**
     * @brief Write a string to a file
     * @param filePath Path to the file
     * @param content Content to write
     * @param append Whether to append to the file (true) or overwrite it (false)
     * @return True if the file was written successfully, false otherwise
     */
    static bool writeFile(const std::string& filePath, const std::string& content, bool append = false);

    /**
     * @brief Write a vector of lines to a file
     * @param filePath Path to the file
     * @param lines Lines to write
     * @param append Whether to append to the file (true) or overwrite it (false)
     * @return True if the file was written successfully, false otherwise
     */
    static bool writeFileLines(const std::string& filePath, const std::vector<std::string>& lines, bool append = false);

    /**
     * @brief Delete a file
     * @param filePath Path to the file
     * @return True if the file was deleted successfully, false otherwise
     */
    static bool deleteFile(const std::string& filePath);

    /**
     * @brief Get the basename of a path
     * @param path Path to get the basename from
     * @return Basename of the path
     */
    static std::string getBasename(const std::string& path);

    /**
     * @brief Get the directory name of a path
     * @param path Path to get the directory name from
     * @return Directory name of the path
     */
    static std::string getDirname(const std::string& path);

    /**
     * @brief Get the extension of a file
     * @param path Path to get the extension from
     * @return Extension of the file
     */
    static std::string getExtension(const std::string& path);

    /**
     * @brief Normalize a path
     * @param path Path to normalize
     * @return Normalized path
     */
    static std::string normalizePath(const std::string& path);

    /**
     * @brief Join path components
     * @param components Path components to join
     * @return Joined path
     */
    static std::string joinPath(const std::vector<std::string>& components);

    /**
     * @brief Join two path components
     * @param path1 First path component
     * @param path2 Second path component
     * @return Joined path
     */
    static std::string joinPath(const std::string& path1, const std::string& path2);
};

} // namespace utils
} // namespace d3server 