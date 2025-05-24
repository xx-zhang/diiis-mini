# Utilities Module (`utils`)

The `utils` module provides a collection of helper classes and functions for common tasks such as debugging, string manipulation, file operations, and cryptography.

## Utility Classes

### `Debug` Class (`utils/include/utils/debug.h`)

The `Debug` class is a singleton utility designed to aid in debugging. It provides macros for conditional logging based on the `DEBUG_MODE` preprocessor directive, along with context management and simple timing capabilities for performance checks.

#### Public Methods

*   `static Debug& getInstance()`: Returns a reference to the singleton `Debug` instance.
*   `void initialize(Logger& logger, bool enabled)`: Initializes the debug utility with a logger instance and an enable/disable flag. Debug messages are routed through the provided `Logger`.
*   `bool isEnabled() const`: Checks if debug mode is currently active.
*   `void setContext(const std::string& context_name)`: Sets a string identifier for the current debugging context (e.g., "AccountCreation", "PacketParsing"). This context can be included in debug log messages.
*   `void clearContext()`: Clears the current debugging context.
*   `std::string getContext() const`: Retrieves the current debugging context string.
*   `void log(const std::string& message, const char* file, int line, const char* func)`: Internal log method used by macros. If debug mode is enabled, it formats the message with timestamp, thread ID, file, line, function, context (if set), and the actual message, then passes it to the `Logger` with `DEBUG` level.
*   `void startTimer(const std::string& timer_name)`: Starts a simple timer with a given name. Stores the start time internally.
*   `void stopTimer(const std::string& timer_name)`: Stops the timer with the given name, calculates the duration, and logs it using the `log` method (e.g., "Timer 'MyOperation' took X ms").

#### Macros (Primary Interface)

These macros are the intended way to use the `Debug` class. They automatically handle the `DEBUG_MODE` check and pass source location information.

*   `DEBUG_LOG(message)`: Logs `message` if `DEBUG_MODE` is enabled.
*   `DEBUG_CONTEXT(context_name)`: Sets the debug context if `DEBUG_MODE` is enabled.
*   `DEBUG_CLEAR_CONTEXT()`: Clears the debug context if `DEBUG_MODE` is enabled.
*   `DEBUG_VARIABLE(variable)`: Logs the name and value of `variable` (e.g., using stringification and `std::to_string` or `operator<<` if available) if `DEBUG_MODE` is enabled.
*   `DEBUG_FUNCTION_ENTER()`: Logs a message indicating entry into the current function if `DEBUG_MODE` is enabled.
*   `DEBUG_FUNCTION_EXIT()`: Logs a message indicating exit from the current function if `DEBUG_MODE` is enabled.
*   `DEBUG_TIMER_START(timer_name)`: Starts a named timer if `DEBUG_MODE` is enabled.
*   `DEBUG_TIMER_STOP(timer_name)`: Stops a named timer and logs its duration if `DEBUG_MODE` is enabled.

### `StringUtils` Class (`utils/include/utils/string_utils.h`)

A static utility class providing various string manipulation functions.

#### Public Static Methods

*   `std::string trim(const std::string& str)`: Removes leading and trailing whitespace from `str`.
*   `std::string trimLeft(const std::string& str)`: Removes leading whitespace from `str`.
*   `std::string trimRight(const std::string& str)`: Removes trailing whitespace from `str`.
*   `std::vector<std::string> split(const std::string& str, char delimiter)`: Splits `str` into a vector of strings based on `delimiter`.
*   `std::vector<std::string> split(const std::string& str, const std::string& delimiter)`: Splits `str` into a vector of strings based on a `delimiter` string.
*   `std::string join(const std::vector<std::string>& P_strings, const std::string& P_delimiter)`: Joins a vector of strings into a single string, separated by `P_delimiter`.
*   `std::string toLower(const std::string& str)`: Converts `str` to lowercase.
*   `std::string toUpper(const std::string& str)`: Converts `str` to uppercase.
*   `bool startsWith(const std::string& str, const std::string& prefix)`: Checks if `str` starts with `prefix`.
*   `bool endsWith(const std::string& str, const std::string& suffix)`: Checks if `str` ends with `suffix`.
*   `std::string replace(const std::string& str, const std::string& P_from, const std::string& P_to)`: Replaces all occurrences of `P_from` with `P_to` in `str`.
*   `std::string getCurrentTimeString(const std::string& P_format = "%Y-%m-%d %H:%M:%S")`: Returns the current date and time formatted as a string according to `P_format`.

### `FileUtils` Class (`utils/include/utils/file_utils.h`)

A static utility class for common file system operations. Uses `std::filesystem` internally.

#### Public Static Methods

*   `bool fileExists(const std::string& path)`: Checks if a file exists at `path`.
*   `bool directoryExists(const std::string& path)`: Checks if a directory exists at `path`.
*   `bool createDirectory(const std::string& path)`: Creates a directory (and any necessary parent directories) at `path`. Returns `true` on success or if the directory already exists.
*   `std::optional<std::string> readFile(const std::string& path)`: Reads the entire content of a file at `path` into a string. Returns `std::nullopt` if the file cannot be read.
*   `bool writeFile(const std::string& path, const std::string& content)`: Writes `content` to a file at `path`, overwriting if the file exists, creating it if it doesn't. Returns `true` on success.
*   `bool deleteFile(const std::string& path)`: Deletes the file at `path`. Returns `true` on success.
*   `std::string getFileName(const std::string& path)`: Extracts the filename (with extension) from `path`.
*   `std::string getFileExtension(const std::string& path)`: Extracts the file extension from `path`.
*   `std::string getParentPath(const std::string& path)`: Extracts the parent directory path from `path`.
*   `std::string combinePaths(const std::string& path1, const std::string& path2)`: Combines two path segments into a single, normalized path.

### `CryptoUtils` Class (`utils/include/utils/crypto_utils.h`)

A static utility class providing cryptographic functions using OpenSSL.

#### Public Static Methods

*   `std::string sha1(const std::string& data)`: Computes the SHA1 hash of `data` and returns it as a hex string.
*   `std::string sha256(const std::string& data)`: Computes the SHA256 hash of `data` and returns it as a hex string.
*   `std::string md5(const std::string& data)`: Computes the MD5 hash of `data` and returns it as a hex string.
*   `std::string generateRandomString(size_t length)`: Generates a cryptographically secure random string of the specified `length` (e.g., alphanumeric).
*   `std::vector<unsigned char> generateRandomBytes(size_t length)`: Generates a vector of cryptographically secure random bytes of the specified `length`.
*   `std::string base64Encode(const std::string& data)`: Encodes `data` using Base64.
*   `std::string base64Encode(const std::vector<unsigned char>& data)`: Encodes a vector of bytes using Base64.
*   `std::string base64Decode(const std::string& P_encoded_string)`: Decodes a Base64 encoded string `P_encoded_string`.
*   `std::vector<unsigned char> base64DecodeBytes(const std::string& P_encoded_string)`: Decodes a Base64 encoded string `P_encoded_string` into a vector of bytes.
*   `std::optional<std::vector<unsigned char>> aes256Encrypt(const std::vector<unsigned char>& P_plain_text, const std::vector<unsigned char>& P_key, const std::vector<unsigned char>& P_iv)`: Encrypts `P_plain_text` using AES-256-CBC with the given `P_key` and `P_iv`. Returns encrypted bytes or `std::nullopt` on error.
*   `std::optional<std::vector<unsigned char>> aes256Decrypt(const std::vector<unsigned char>& P_cipher_text, const std::vector<unsigned char>& P_key, const std::vector<unsigned char>& P_iv)`: Decrypts `P_cipher_text` using AES-256-CBC with the given `P_key` and `P_iv`. Returns decrypted bytes or `std::nullopt` on error (e.g., padding error, incorrect key/iv).
*   `std::string bytesToHexString(const std::vector<unsigned char>& bytes)`: Converts a vector of bytes to its hexadecimal string representation.
*   `std::vector<unsigned char> hexStringToBytes(const std::string& P_hex_string)`: Converts a hexadecimal string to a vector of bytes.
*   `std::string hashPassword(const std::string& password, const std::string& salt)`: Hashes a password using a strong algorithm (e.g., PBKDF2 with SHA256) combined with a `salt`. Returns the derived key as a hex string.
*   `bool verifyPassword(const std::string& password, const std::string& salt, const std::string& P_known_hash)`: Verifies a `password` against a `P_known_hash` using the same `salt` and hashing algorithm. Returns `true` if they match. 