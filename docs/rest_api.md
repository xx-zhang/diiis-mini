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