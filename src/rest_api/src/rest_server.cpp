#include "rest_api/rest_server.h"
#include "rest_api/account_controller.h"
#include "rest_api/admin_controller.h"
#include "core/config.h"
#include "core/logger.h"
#include "utils/debug.h"
#include "utils/string_utils.h"
#include "database/database_manager.h"

#include <iostream>
#include <sstream>
#include <regex>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace d3server {
namespace rest_api {

// Session implementation
RestServer::Session::Session(boost::asio::ip::tcp::socket socket, RestServer& server)
    : m_socket(std::move(socket)), m_server(server) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_FUNCTION_EXIT();
}

void RestServer::Session::start() {
    DEBUG_FUNCTION_ENTER();
    readRequest();
    DEBUG_FUNCTION_EXIT();
}

void RestServer::Session::readRequest() {
    DEBUG_FUNCTION_ENTER();
    
    auto self = shared_from_this();
    boost::asio::async_read_until(
        m_socket,
        m_requestBuffer,
        "\r\n\r\n",
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                DEBUG_LOG("Request header received, reading body");
                
                // Read the request data into a string
                std::istream request_stream(&m_requestBuffer);
                std::string request_data;
                std::string line;
                std::size_t content_length = 0;
                
                // Parse headers
                while (std::getline(request_stream, line) && line != "\r") {
                    request_data += line + "\n";
                    
                    // Look for Content-Length header
                    std::regex content_length_regex("Content-Length:\\s*(\\d+)", std::regex::icase);
                    std::smatch match;
                    if (std::regex_search(line, match, content_length_regex)) {
                        content_length = std::stoi(match[1]);
                    }
                }
                
                // If there's a body, read it
                if (content_length > 0) {
                    // Calculate how many bytes are already in the buffer
                    std::size_t body_bytes_available = m_requestBuffer.size();
                    
                    if (body_bytes_available < content_length) {
                        // Need to read more data
                        boost::asio::async_read(
                            m_socket,
                            m_requestBuffer,
                            boost::asio::transfer_exactly(content_length - body_bytes_available),
                            [this, self, request_data, content_length](boost::system::error_code ec, std::size_t) {
                                if (!ec) {
                                    std::string body(std::istreambuf_iterator<char>(&m_requestBuffer), {});
                                    processRequest(m_server.parseRequest(request_data + "\n" + body));
                                } else {
                                    DEBUG_LOG("Error reading request body: " + ec.message());
                                }
                            }
                        );
                    } else {
                        // All data already available
                        std::string body(std::istreambuf_iterator<char>(&m_requestBuffer), {});
                        processRequest(m_server.parseRequest(request_data + "\n" + body));
                    }
                } else {
                    // No body, process the request
                    processRequest(m_server.parseRequest(request_data));
                }
            } else {
                DEBUG_LOG("Error reading request: " + ec.message());
            }
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

void RestServer::Session::processRequest(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(request.method);
    DEBUG_VARIABLE(request.uri);
    
    // Get handler for this route
    std::lock_guard<std::mutex> lock(m_server.m_routesMutex);
    
    // Find method handler
    auto methodIt = m_server.m_routes.find(request.method);
    if (methodIt != m_server.m_routes.end()) {
        // Find URI handler
        auto uriIt = methodIt->second.find(request.uri);
        if (uriIt != methodIt->second.end()) {
            // Found a direct match
            HttpResponse response = uriIt->second(request);
            writeResponse(response);
            DEBUG_FUNCTION_EXIT();
            return;
        }
        
        // Try to find a pattern match
        for (const auto& routePair : methodIt->second) {
            const std::string& pattern = routePair.first;
            
            // Skip non-pattern routes
            if (pattern.find('{') == std::string::npos) {
                continue;
            }
            
            // Convert pattern to regex
            std::string regexPattern = pattern;
            std::regex paramRegex("\\{([^\\}]+)\\}");
            regexPattern = std::regex_replace(regexPattern, paramRegex, "([^/]+)");
            regexPattern = "^" + regexPattern + "$";
            
            std::regex pathRegex(regexPattern);
            std::smatch matches;
            
            if (std::regex_match(request.uri, matches, pathRegex)) {
                // Found a pattern match
                HttpResponse response = routePair.second(request);
                writeResponse(response);
                DEBUG_FUNCTION_EXIT();
                return;
            }
        }
    }
    
    // No handler found, return 404
    HttpResponse response;
    response.statusCode = 404;
    response.statusMessage = "Not Found";
    response.headers["Content-Type"] = "application/json";
    response.body = "{\"error\":\"Not Found\",\"message\":\"The requested resource was not found\"}";
    
    writeResponse(response);
    DEBUG_FUNCTION_EXIT();
}

void RestServer::Session::writeResponse(const HttpResponse& response) {
    DEBUG_FUNCTION_ENTER();
    
    auto self = shared_from_this();
    m_response = m_server.formatResponse(response);
    
    boost::asio::async_write(
        m_socket,
        boost::asio::buffer(m_response),
        [this, self](boost::system::error_code ec, std::size_t) {
            if (ec) {
                DEBUG_LOG("Error writing response: " + ec.message());
            }
            
            // Close the socket after sending the response
            boost::system::error_code ignored_ec;
            m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

// RestServer implementation
RestServer::RestServer(
    std::shared_ptr<core::Config> config,
    std::shared_ptr<database::DatabaseManager> dbManager
) : 
    m_config(config),
    m_dbManager(dbManager),
    m_ioContext(),
    m_acceptor(m_ioContext),
    m_running(false)
{
    DEBUG_FUNCTION_ENTER();
    LOG_INFO("REST API server instance created");
    DEBUG_FUNCTION_EXIT();
}

RestServer::~RestServer() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        shutdown();
    }
    
    LOG_INFO("REST API server instance destroyed");
    DEBUG_FUNCTION_EXIT();
}

bool RestServer::init() {
    DEBUG_FUNCTION_ENTER();
    
    try {
        LOG_INFO("Initializing REST API server...");
        
        // Create controllers
        m_accountController = std::make_shared<AccountController>(m_config, m_dbManager);
        m_adminController = std::make_shared<AdminController>(m_config, m_dbManager);
        
        // Register API routes
        registerRoutes();
        
        // Initialize acceptor
        auto& networkConfig = m_config->getNetworkConfig();
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(networkConfig.bindIp),
            networkConfig.restApiPort
        );
        
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();
        
        LOG_INFO("REST API server initialized on " + networkConfig.bindIp + ":" + std::to_string(networkConfig.restApiPort));
        DEBUG_FUNCTION_EXIT();
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during REST API server initialization: " + std::string(e.what()));
        DEBUG_FUNCTION_EXIT();
        return false;
    }
}

void RestServer::run() {
    DEBUG_FUNCTION_ENTER();
    
    if (m_running) {
        LOG_WARNING("REST API server is already running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_running = true;
    
    LOG_INFO("Starting REST API server...");
    
    // Start accepting connections
    startAccept();
    
    // Run the IO context
    m_ioThread = std::thread([this]() {
        DEBUG_CONTEXT("REST API server IO thread started");
        
        try {
            m_ioContext.run();
        }
        catch (const std::exception& e) {
            LOG_ERROR("Exception in REST API server IO thread: " + std::string(e.what()));
        }
        
        DEBUG_LOG("REST API server IO thread exited");
    });
    
    LOG_INFO("REST API server started");
    DEBUG_FUNCTION_EXIT();
}

void RestServer::shutdown() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        LOG_WARNING("REST API server is not running");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    LOG_INFO("Shutting down REST API server...");
    m_running = false;
    
    // Stop the IO context
    m_ioContext.stop();
    
    // Wait for the IO thread to finish
    if (m_ioThread.joinable()) {
        m_ioThread.join();
    }
    
    LOG_INFO("REST API server shutdown complete");
    DEBUG_FUNCTION_EXIT();
}

void RestServer::startAccept() {
    DEBUG_FUNCTION_ENTER();
    
    if (!m_running) {
        DEBUG_LOG("REST API server is not running, not accepting connections");
        DEBUG_FUNCTION_EXIT();
        return;
    }
    
    m_acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
            if (!ec) {
                // Create a new session
                std::make_shared<Session>(std::move(socket), *this)->start();
            } else {
                DEBUG_LOG("Error accepting connection: " + ec.message());
            }
            
            // Accept the next connection
            startAccept();
        }
    );
    
    DEBUG_FUNCTION_EXIT();
}

HttpRequest RestServer::parseRequest(const std::string& data) {
    DEBUG_FUNCTION_ENTER();
    
    HttpRequest request;
    std::istringstream stream(data);
    std::string line;
    
    // Parse request line
    if (std::getline(stream, line)) {
        std::istringstream request_line(line);
        request_line >> request.method >> request.uri >> request.version;
    }
    
    // Parse headers
    bool inHeaders = true;
    while (inHeaders && std::getline(stream, line)) {
        // Check for end of headers
        if (line == "\r" || line.empty()) {
            inHeaders = false;
            continue;
        }
        
        // Parse header
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string name = utils::StringUtils::trim(line.substr(0, colon));
            std::string value = utils::StringUtils::trim(line.substr(colon + 1));
            request.headers[name] = value;
        }
    }
    
    // Parse body
    if (!inHeaders) {
        std::stringstream body_stream;
        body_stream << stream.rdbuf();
        request.body = body_stream.str();
    }
    
    DEBUG_FUNCTION_EXIT();
    return request;
}

std::string RestServer::formatResponse(const HttpResponse& response) {
    DEBUG_FUNCTION_ENTER();
    
    std::ostringstream stream;
    
    // Format status line
    stream << "HTTP/1.1 " << response.statusCode << " " << response.statusMessage << "\r\n";
    
    // Format headers
    for (const auto& header : response.headers) {
        stream << header.first << ": " << header.second << "\r\n";
    }
    
    // Add Content-Length header if not already present
    if (response.headers.find("Content-Length") == response.headers.end()) {
        stream << "Content-Length: " << response.body.length() << "\r\n";
    }
    
    // Add Date header if not already present
    if (response.headers.find("Date") == response.headers.end()) {
        stream << "Date: " << utils::StringUtils::getCurrentTimeString("%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
    }
    
    // Add Server header if not already present
    if (response.headers.find("Server") == response.headers.end()) {
        stream << "Server: D3Server/" << m_config->getServerConfig().serverName << "\r\n";
    }
    
    // End of headers
    stream << "\r\n";
    
    // Body
    stream << response.body;
    
    DEBUG_FUNCTION_EXIT();
    return stream.str();
}

void RestServer::registerRoutes() {
    DEBUG_FUNCTION_ENTER();
    
    std::lock_guard<std::mutex> lock(m_routesMutex);
    
    // Register account routes
    
    // Create account
    m_routes["POST"]["/api/accounts"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route doesn't require authentication as it's used to create new accounts
        if (m_accountController) {
            return m_accountController->createAccount(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Get all accounts
    m_routes["GET"]["/api/accounts"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            return m_accountController->getAllAccounts(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Get account by login
    m_routes["GET"]["/api/accounts/{login}"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->getAccount(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Update account
    m_routes["PUT"]["/api/accounts/{login}"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->updateAccount(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Delete account
    m_routes["DELETE"]["/api/accounts/{login}"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->deleteAccount(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Ban account
    m_routes["POST"]["/api/accounts/{login}/ban"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)/ban");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_adminController->banAccount(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    // Get character list for account
    m_routes["GET"]["/api/accounts/{login}/characters"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)/characters");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->getCharacters(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Create character for account
    m_routes["POST"]["/api/accounts/{login}/characters"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_accountController) {
            // Extract login from URI
            std::regex pathRegex("/api/accounts/([^/]+)/characters");
            std::smatch matches;
            if (std::regex_match(request.uri, matches, pathRegex) && matches.size() > 1) {
                std::string login = matches[1];
                return m_accountController->createCharacter(request, login);
            }
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Account controller not initialized\"}");
    };
    
    // Admin routes
    
    // Server status
    m_routes["GET"]["/api/server/status"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            return m_adminController->getServerStatus(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    // Server shutdown
    m_routes["POST"]["/api/server/shutdown"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            return m_adminController->shutdownServer(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    // Server restart
    m_routes["POST"]["/api/server/restart"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            return m_adminController->restartServer(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    // Get logs
    m_routes["GET"]["/api/logs"] = [this](const HttpRequest& request) -> HttpResponse {
        // This route requires authentication
        if (!isAuthorized(request)) {
            return handleUnauthorized();
        }
        
        if (m_adminController) {
            return m_adminController->getLogs(request);
        }
        return createJsonResponse(500, "{\"error\":\"Internal Server Error\",\"message\":\"Admin controller not initialized\"}");
    };
    
    LOG_INFO("REST API routes registered");
    DEBUG_FUNCTION_EXIT();
}

HttpResponse RestServer::createJsonResponse(int statusCode, const std::string& body) {
    DEBUG_FUNCTION_ENTER();
    
    HttpResponse response;
    response.statusCode = statusCode;
    
    switch (statusCode) {
        case 200: response.statusMessage = "OK"; break;
        case 201: response.statusMessage = "Created"; break;
        case 204: response.statusMessage = "No Content"; break;
        case 400: response.statusMessage = "Bad Request"; break;
        case 401: response.statusMessage = "Unauthorized"; break;
        case 403: response.statusMessage = "Forbidden"; break;
        case 404: response.statusMessage = "Not Found"; break;
        case 409: response.statusMessage = "Conflict"; break;
        case 500: response.statusMessage = "Internal Server Error"; break;
        default: response.statusMessage = "Unknown"; break;
    }
    
    response.headers["Content-Type"] = "application/json";
    response.body = body;
    
    DEBUG_FUNCTION_EXIT();
    return response;
}

HttpResponse RestServer::handleUnauthorized() {
    DEBUG_FUNCTION_ENTER();
    
    HttpResponse response = createJsonResponse(401, "{\"error\":\"Unauthorized\",\"message\":\"Authentication required\"}");
    response.headers["WWW-Authenticate"] = "Bearer";
    
    DEBUG_FUNCTION_EXIT();
    return response;
}

bool RestServer::isAuthorized(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    
    // Get API key from request
    std::string apiKey = getApiKey(request);
    
    // Validate API key
    bool authorized = !apiKey.empty() && validateApiKey(apiKey);
    
    DEBUG_FUNCTION_EXIT();
    return authorized;
}

std::string RestServer::getApiKey(const HttpRequest& request) {
    DEBUG_FUNCTION_ENTER();
    
    // Check Authorization header
    auto it = request.headers.find("Authorization");
    if (it != request.headers.end()) {
        std::string authHeader = it->second;
        
        // Check if it's a Bearer token
        if (utils::StringUtils::startsWith(authHeader, "Bearer ")) {
            DEBUG_FUNCTION_EXIT();
            return authHeader.substr(7); // Skip "Bearer "
        }
    }
    
    // Check for API key in query parameters
    auto queryPos = request.uri.find('?');
    if (queryPos != std::string::npos) {
        std::string queryString = request.uri.substr(queryPos + 1);
        std::vector<std::string> pairs = utils::StringUtils::split(queryString, '&');
        
        for (const auto& pair : pairs) {
            std::vector<std::string> keyValue = utils::StringUtils::split(pair, '=');
            if (keyValue.size() == 2 && keyValue[0] == "api_key") {
                DEBUG_FUNCTION_EXIT();
                return keyValue[1];
            }
        }
    }
    
    DEBUG_FUNCTION_EXIT();
    return "";
}

bool RestServer::validateApiKey(const std::string& apiKey) {
    DEBUG_FUNCTION_ENTER();
    DEBUG_VARIABLE(apiKey);
    
    // TODO: Implement proper API key validation against database
    // For now, just check if it's not empty and meets a minimum length requirement
    bool valid = !apiKey.empty() && apiKey.length() >= 32;
    
    DEBUG_FUNCTION_EXIT();
    return valid;
}

} // namespace rest_api
} // namespace d3server 