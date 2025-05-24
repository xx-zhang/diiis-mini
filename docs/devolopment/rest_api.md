# D3Server REST API Documentation

This document describes the REST API endpoints available in the D3Server.

## Authentication

Most API endpoints require authentication. Authentication is performed using a Bearer token in the Authorization header.

```
Authorization: Bearer your_api_key
```

Alternatively, you can provide the API key as a query parameter:

```
?api_key=your_api_key
```

## API Endpoints

### Account Management

#### Create Account

Create a new user account.

**URL**: `/api/accounts`

**Method**: `POST`

**Auth required**: No

**Request Body**:
```json
{
    "login": "username",
    "email": "user@example.com",
    "password": "password123"
}
```

**Success Response**:
- **Code**: 201 Created
- **Content**:
```json
{
    "success": true,
    "message": "Account created successfully"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 409 Conflict (Account already exists)
- **Code**: 500 Internal Server Error

#### Get All Accounts

Get a list of all accounts.

**URL**: `/api/accounts`

**Method**: `GET`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
[
    {
        "login": "username1",
        "email": "user1@example.com",
        "created": "2023-01-01 12:00:00",
        "last_login": "2023-01-02 12:00:00",
        "banned": false,
        "character_count": 2
    },
    {
        "login": "username2",
        "email": "user2@example.com",
        "created": "2023-01-03 12:00:00",
        "last_login": "2023-01-04 12:00:00",
        "banned": false,
        "character_count": 1
    }
]
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 500 Internal Server Error

#### Get Account

Get information about a specific account.

**URL**: `/api/accounts/{login}`

**Method**: `GET`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "login": "username",
    "email": "user@example.com",
    "created": "2023-01-01 12:00:00",
    "last_login": "2023-01-02 12:00:00",
    "banned": false,
    "character_count": 2
}
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

#### Update Account

Update an existing account.

**URL**: `/api/accounts/{login}`

**Method**: `PUT`

**Auth required**: Yes

**Request Body**:
```json
{
    "email": "newemail@example.com",
    "password": "newpassword123"
}
```
Note: Both fields are optional. Include only the fields you want to update.

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Account updated successfully"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

#### Delete Account

Delete an account.

**URL**: `/api/accounts/{login}`

**Method**: `DELETE`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Account deleted successfully"
}
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

#### Ban Account

Ban or unban an account.

**URL**: `/api/accounts/{login}/ban`

**Method**: `POST`

**Auth required**: Yes

**Request Body**:
```json
{
    "banned": true,
    "reason": "Cheating"
}
```

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Account banned successfully"
}
```
or
```json
{
    "success": true,
    "message": "Account unbanned successfully"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

### Character Management

#### Get All Characters

Get a list of all characters for an account.

**URL**: `/api/accounts/{login}/characters`

**Method**: `GET`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
[
    {
        "id": 1,
        "name": "Barbarian1",
        "class_id": 1,
        "level": 60,
        "created": "2023-01-01 12:00:00",
        "last_played": "2023-01-02 12:00:00"
    },
    {
        "id": 2,
        "name": "Wizard1",
        "class_id": 3,
        "level": 45,
        "created": "2023-01-03 12:00:00",
        "last_played": "2023-01-04 12:00:00"
    }
]
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found
- **Code**: 500 Internal Server Error

#### Create Character

Create a new character for an account.

**URL**: `/api/accounts/{login}/characters`

**Method**: `POST`

**Auth required**: Yes

**Request Body**:
```json
{
    "name": "MyCharacter",
    "class_id": 2
}
```

Class IDs:
1. Barbarian
2. Monk
3. Wizard
4. Witch Doctor
5. Demon Hunter
6. Crusader
7. Necromancer

**Success Response**:
- **Code**: 201 Created
- **Content**:
```json
{
    "success": true,
    "message": "Character created successfully"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data or max character limit reached)
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found (Account not found)
- **Code**: 409 Conflict (Character name already exists)
- **Code**: 500 Internal Server Error

### Server Management

#### Get Server Status

Get current server status.

**URL**: `/api/server/status`

**Method**: `GET`

**Auth required**: Yes

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "server_name": "D3Server",
    "version": "1.0.0",
    "uptime": "3 days, 2 hours, 15 minutes",
    "current_time": "2023-01-01 12:00:00",
    "server_stats": {
        "accounts": 150,
        "characters": 432,
        "online_players": 25,
        "max_players": 1000
    },
    "system_info": {
        "os": "Linux",
        "cpu_usage": 15.2,
        "memory_usage": 256.5,
        "debug_enabled": true
    }
}
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 500 Internal Server Error

#### Server Shutdown

Initiate server shutdown.

**URL**: `/api/server/shutdown`

**Method**: `POST`

**Auth required**: Yes

**Request Body**:
```json
{
    "delay": 60,
    "reason": "Server maintenance"
}
```
Note: Delay is in seconds, defaults to 0 (immediate) if not provided.

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Server will shut down in 60 seconds"
}
```
or
```json
{
    "success": true,
    "message": "Server is shutting down"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 401 Unauthorized
- **Code**: 500 Internal Server Error

#### Server Restart

Initiate server restart.

**URL**: `/api/server/restart`

**Method**: `POST`

**Auth required**: Yes

**Request Body**:
```json
{
    "delay": 60,
    "reason": "Server update"
}
```
Note: Delay is in seconds, defaults to 0 (immediate) if not provided.

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "success": true,
    "message": "Server will restart in 60 seconds"
}
```
or
```json
{
    "success": true,
    "message": "Server is restarting"
}
```

**Error Responses**:
- **Code**: 400 Bad Request (Invalid data)
- **Code**: 401 Unauthorized
- **Code**: 500 Internal Server Error

#### Get Logs

Get server logs.

**URL**: `/api/logs`

**Method**: `GET`

**Auth required**: Yes

**Query Parameters**:
- `level` (optional): Log level to filter (e.g., INFO, WARNING, ERROR, ALL). Default: INFO
- `lines` (optional): Number of log lines to retrieve. Default: 100, Max: 1000

**Success Response**:
- **Code**: 200 OK
- **Content**:
```json
{
    "log_level": "INFO",
    "total_lines": 1500,
    "returned_lines": 100,
    "logs": [
        "[INFO] 2023-01-01 12:00:00 - Server started",
        "[INFO] 2023-01-01 12:01:00 - Player connected: username",
        "..."
    ]
}
```

**Error Responses**:
- **Code**: 401 Unauthorized
- **Code**: 404 Not Found (Log file not found)
- **Code**: 500 Internal Server Error

## Error Response Format

All API errors follow this format:

```json
{
    "error": true,
    "message": "Error message description"
}
```

# REST API Module (`rest_api`)

The `rest_api` module provides an HTTP-based API for managing server operations, accounts, and potentially other game-related data. It uses Boost.Asio for asynchronous network operations and `nlohmann/json` for JSON processing.

## Core Components

### `RestServer` Class (`rest_api/include/rest_api/rest_server.h`)

The `RestServer` class is responsible for listening for incoming HTTP connections, parsing requests, routing them to appropriate handlers, and sending back HTTP responses.

#### Key Features:

*   Asynchronous request handling using Boost.Asio.
*   Route management with support for path parameters (e.g., `/api/accounts/{login}`).
*   Basic API key authentication (Bearer token or `api_key` query parameter).
*   JSON request and response handling via `nlohmann/json`.

#### Public Methods

*   `RestServer(boost::asio::io_context& P_io_context, short P_port, Config& P_config, Database& P_db, Logger& P_logger, Debug& P_debug)`: Constructor. Initializes the server with an I/O context, port, configuration, database, logger, and debug instances.
*   `void start()`: Starts the HTTP server, begins accepting connections.
*   `void stop()`: Stops the HTTP server, closes the acceptor and active connections.

#### Internal `Session` Class

*   Handles individual client connections asynchronously.
*   Reads and parses HTTP requests (method, URI, version, headers, body).
*   Invokes the appropriate registered `RouteHandler`.
*   Formats and sends HTTP responses.

#### Route Handling

*   Routes are stored in a map: `std::unordered_map<std::string /* HTTP Method */, std::unordered_map<std::string /* Path Pattern */, RouteHandler>>`
*   `RouteHandler` is a `std::function<HttpResponse(const HttpRequest&, const std::smatch& /* path_matches */)>`.
*   `registerRoute(const std::string& method, const std::string& path_pattern, RouteHandler handler)`: Registers a handler for a specific HTTP method and path pattern. Path patterns can include named capture groups for parameters (e.g., `"/api/users/([a-zA-Z0-9_]+)"` where `([a-zA-Z0-9_]+)` captures a user ID).

#### Request and Response Structures

*   `struct HttpRequest`:
    *   `std::string method`: e.g., "GET", "POST".
    *   `std::string uri`: The full request URI.
    *   `std::string path`: The path part of the URI.
    *   `std::map<std::string, std::string> query_params`: Parsed query parameters.
    *   `int http_version_major`, `int http_version_minor`.
    *   `std::map<std::string, std::string> headers`.
    *   `std::string body`.
    *   `nlohmann::json json_body`: Parsed JSON body (if `Content-Type` is `application/json`).
*   `struct HttpResponse`:
    *   `int status_code`: e.g., 200, 404, 500.
    *   `std::map<std::string, std::string> headers`.
    *   `std::string body`.
    *   `nlohmann::json json_body`: If set, this is serialized and sent as the body with `Content-Type: application/json`.

### `AccountController` Class (`rest_api/include/rest_api/account_controller.h`)

Handles API requests related to user accounts and characters. It uses the `Database` module for persistence.

#### Public Methods

*   `AccountController(Database& db, Logger& logger, Debug& debug)`: Constructor.
*   `void registerRoutes(RestServer& server)`: Registers all account and character related routes with the `RestServer` instance.

#### Endpoints Handled (and corresponding private handler methods):

*   **Create Account:**
    *   `POST /api/accounts`
    *   Request Body: `{ "login": "user@email.com", "password": "securePassword123", "battle_tag": "Player#1234" }`
    *   Response: `201 Created` with account details, or error (e.g., `400 Bad Request`, `409 Conflict`).
    *   Private Handler: `HttpResponse handleCreateAccount(const HttpRequest& request, const std::smatch& matches)`
*   **Get Account:**
    *   `GET /api/accounts/{login}` (Path parameter: `login`)
    *   Response: `200 OK` with account details, or `404 Not Found`.
    *   Private Handler: `HttpResponse handleGetAccount(const HttpRequest& request, const std::smatch& matches)`
*   **Update Account:**
    *   `PUT /api/accounts/{login}`
    *   Request Body: `{ "email": "new_email@example.com", "battle_tag": "NewTag#5678" }` (Password update might be a separate endpoint or require current password).
    *   Response: `200 OK` with updated account details, or error (e.g., `400 Bad Request`, `404 Not Found`).
    *   Private Handler: `HttpResponse handleUpdateAccount(const HttpRequest& request, const std::smatch& matches)`
*   **Delete Account:**
    *   `DELETE /api/accounts/{login}`
    *   Response: `204 No Content`, or `404 Not Found`.
    *   Private Handler: `HttpResponse handleDeleteAccount(const HttpRequest& request, const std::smatch& matches)`
*   **Create Character:**
    *   `POST /api/accounts/{login}/characters`
    *   Request Body: `{ "name": "HeroName", "class_id": 0 }` (Refer to `docs/rest_api.md` or a dedicated section for `class_id` mapping)
    *   Response: `201 Created` with character details, or error.
    *   Private Handler: `HttpResponse handleCreateCharacter(const HttpRequest& request, const std::smatch& matches)`
*   **Get Characters for Account:**
    *   `GET /api/accounts/{login}/characters`
    *   Response: `200 OK` with a list of characters, or `404 Not Found` (if account doesn't exist).
    *   Private Handler: `HttpResponse handleGetCharacters(const HttpRequest& request, const std::smatch& matches)`

### `AdminController` Class (`rest_api/include/rest_api/admin_controller.h`)

Handles API requests for administrative operations like server management, user banning, and log viewing.

#### Public Methods

*   `AdminController(Database& db, Logger& logger, Debug& debug, Server& P_server_instance)`: Constructor. Takes a reference to the main `Server` instance for shutdown/restart operations.
*   `void registerRoutes(RestServer& server)`: Registers all admin-related routes.

#### Endpoints Handled (and corresponding private handler methods):

*   **Ban Account:**
    *   `POST /api/admin/accounts/{login}/ban`
    *   Request Body: `{ "reason": "Violation of terms.", "duration_hours": 72 }` (duration optional, 0 or absent for permanent)
    *   Response: `200 OK`, or `404 Not Found`.
    *   Private Handler: `HttpResponse handleBanAccount(const HttpRequest& request, const std::smatch& matches)`
*   **Unban Account:**
    *   `POST /api/admin/accounts/{login}/unban`
    *   Response: `200 OK`, or `404 Not Found`.
    *   Private Handler: `HttpResponse handleUnbanAccount(const HttpRequest& request, const std::smatch& matches)`
*   **Get Server Status:**
    *   `GET /api/admin/server/status`
    *   Response: `200 OK` with server status information (e.g., uptime, player count, version).
    *   Private Handler: `HttpResponse handleGetServerStatus(const HttpRequest& request, const std::smatch& matches)`
*   **Shutdown Server:**
    *   `POST /api/admin/server/shutdown`
    *   Response: `200 OK` (Request accepted, server will shut down). Uses the `Server` instance to trigger shutdown.
    *   Private Handler: `HttpResponse handleServerShutdown(const HttpRequest& request, const std::smatch& matches)`
*   **Restart Server:** (Placeholder - more complex, might involve an external script or process manager)
    *   `POST /api/admin/server/restart`
    *   Response: `200 OK` (Request accepted).
    *   Private Handler: `HttpResponse handleServerRestart(const HttpRequest& request, const std::smatch& matches)`
*   **Get Server Logs:**
    *   `GET /api/admin/server/logs?level=INFO&lines=100` (Query params: `level`, `lines`)
    *   Response: `200 OK` with log entries.
    *   Private Handler: `HttpResponse handleGetServerLogs(const HttpRequest& request, const std::smatch& matches)`

---

*Further details on request/response JSON structures, `class_id` mappings, and specific error codes should be maintained and expanded here or in linked documents as development progresses.* 