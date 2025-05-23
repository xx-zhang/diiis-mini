#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <boost/asio.hpp>

namespace d3server {

// Forward declarations
namespace core {
class Config;
}

namespace database {
class DatabaseManager;
}

namespace rest_api {

class AccountController;
class AdminController;

/**
 * @brief Represents an HTTP request
 */
struct HttpRequest {
    std::string method;
    std::string uri;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

/**
 * @brief Represents an HTTP response
 */
struct HttpResponse {
    int statusCode;
    std::string statusMessage;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

/**
 * @brief Function type for API route handlers
 */
using RouteHandler = std::function<HttpResponse(const HttpRequest&)>;

/**
 * @brief REST API server that handles HTTP requests
 */
class RestServer {
public:
    /**
     * @brief Constructor
     * @param config Configuration instance
     * @param dbManager Database manager instance
     */
    RestServer(
        std::shared_ptr<core::Config> config,
        std::shared_ptr<database::DatabaseManager> dbManager
    );

    /**
     * @brief Destructor
     */
    ~RestServer();

    /**
     * @brief Initialize the REST server
     * @return True if initialization succeeded, false otherwise
     */
    bool init();

    /**
     * @brief Run the REST server (blocking call)
     */
    void run();

    /**
     * @brief Shutdown the REST server
     */
    void shutdown();

private:
    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(boost::asio::ip::tcp::socket socket, RestServer& server);
        void start();
        
    private:
        void readRequest();
        void processRequest(const HttpRequest& request);
        void writeResponse(const HttpResponse& response);
        
        boost::asio::ip::tcp::socket m_socket;
        RestServer& m_server;
        boost::asio::streambuf m_requestBuffer;
        std::string m_response;
    };
    
    /**
     * @brief Start accepting connections
     */
    void startAccept();
    
    /**
     * @brief Handle a new connection
     * @param socket Socket for the new connection
     */
    void handleAccept(boost::asio::ip::tcp::socket socket);
    
    /**
     * @brief Parse an HTTP request
     * @param data Request data
     * @return Parsed HTTP request
     */
    HttpRequest parseRequest(const std::string& data);
    
    /**
     * @brief Format an HTTP response
     * @param response HTTP response
     * @return Formatted HTTP response
     */
    std::string formatResponse(const HttpResponse& response);
    
    /**
     * @brief Register API routes
     */
    void registerRoutes();
    
    /**
     * @brief Create a JSON response
     * @param statusCode HTTP status code
     * @param body JSON body
     * @return HTTP response
     */
    HttpResponse createJsonResponse(int statusCode, const std::string& body);
    
    /**
     * @brief Handle unauthorized access
     * @return HTTP response for unauthorized access
     */
    HttpResponse handleUnauthorized();
    
    /**
     * @brief Check if a request is authorized
     * @param request HTTP request
     * @return True if authorized, false otherwise
     */
    bool isAuthorized(const HttpRequest& request);
    
    /**
     * @brief Extract API key from request
     * @param request HTTP request
     * @return API key or empty string if not found
     */
    std::string getApiKey(const HttpRequest& request);
    
    /**
     * @brief Validate an API key
     * @param apiKey API key to validate
     * @return True if valid, false otherwise
     */
    bool validateApiKey(const std::string& apiKey);
    
    std::shared_ptr<core::Config> m_config;
    std::shared_ptr<database::DatabaseManager> m_dbManager;
    std::shared_ptr<AccountController> m_accountController;
    std::shared_ptr<AdminController> m_adminController;
    
    boost::asio::io_context m_ioContext;
    boost::asio::ip::tcp::acceptor m_acceptor;
    
    std::atomic<bool> m_running;
    std::thread m_ioThread;
    std::mutex m_routesMutex;
    
    // Routes map: method -> (uri -> handler)
    std::unordered_map<std::string, std::unordered_map<std::string, RouteHandler>> m_routes;
};

} // namespace rest_api
} // namespace d3server 