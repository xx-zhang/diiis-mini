# D3Server Battle.net Protocol Documentation

This document describes the Battle.net protocol used for communication between the D3Server and clients.

## Overview

The Battle.net protocol is a binary protocol used for authentication, character management, and game coordination. It uses a simple message-based format with a header followed by a variable-length body.

## Message Format

Each message has the following format:

```
+----------------+----------------+
| Header (4 bytes)| Body (variable) |
+----------------+----------------+
```

### Header

The header is 4 bytes long and contains the length of the body in little-endian format.

```
+-------------------+-------------------+-------------------+-------------------+
| Length (byte 0)   | Length (byte 1)   | Length (byte 2)   | Length (byte 3)   |
+-------------------+-------------------+-------------------+-------------------+
```

### Body

The body contains the message data, with the first byte indicating the message type.

```
+-------------------+-------------------+
| Message Type      | Message Data      |
+-------------------+-------------------+
```

## Message Types

### Server to Client

| Type (Hex) | Description            | Format                                    |
|------------|------------------------|-------------------------------------------|
| 0x01       | Welcome                | [0x01] [Version (4 bytes)]                |
| 0x02       | Authentication Result  | [0x02] [Result (1 = Success, 0 = Fail)]   |
| 0x04       | Pong                   | [0x04]                                    |
| 0x06       | Character List         | [0x06] [Count (2 bytes)] [Character Data] |

#### Character Data Format

Each character entry in the character list has the following format:

```
+--------------------+-----------------+--------------+--------------+------------------+
| ID (4 bytes)       | Name Length     | Name         | Class (1 byte)| Level (1 byte)  |
+--------------------+-----------------+--------------+--------------+------------------+
```

### Client to Server

| Type (Hex) | Description            | Format                                   |
|------------|------------------------|------------------------------------------|
| 0x01       | Authentication Request | [0x01] [Login Len (1 byte)] [Login] [Password Len (1 byte)] [Password] |
| 0x03       | Ping                   | [0x03]                                   |
| 0x05       | Request Character List | [0x05]                                   |
| 0x07       | Create Character       | [0x07] [Name Len (1 byte)] [Name] [Class (1 byte)] |
| 0x09       | Enter Game             | [0x09] [Character ID (4 bytes)]          |

## Authentication Process

1. Server sends Welcome message (0x01) upon client connection.
2. Client sends Authentication Request (0x01) with login and password.
3. Server validates credentials and sends Authentication Result (0x02).
4. If authentication is successful, client can proceed to request character list, create characters, etc.

## Class IDs

| ID | Class Name    |
|----|---------------|
| 1  | Barbarian     |
| 2  | Monk          |
| 3  | Wizard        |
| 4  | Witch Doctor  |
| 5  | Demon Hunter  |
| 6  | Crusader      |
| 7  | Necromancer   |

## Ping/Pong Mechanism

To detect connection drops and maintain connectivity, clients should send periodic Ping messages (0x03) to the server. The server responds with a Pong message (0x04).

## Error Handling

When an error occurs during message processing, the server may disconnect the client. No explicit error messages are defined in the protocol. The client should interpret connection termination as an indication of an error.

## Example Message Flow

### Authentication

1. Server → Client: Welcome
   ```
   Header: 05 00 00 00
   Body:   01 01 00 00 00
   ```

2. Client → Server: Authentication Request
   ```
   Header: 10 00 00 00
   Body:   01 05 "alice" 08 "password"
   ```

3. Server → Client: Authentication Result (Success)
   ```
   Header: 02 00 00 00
   Body:   02 01
   ```

### Character List Request

1. Client → Server: Request Character List
   ```
   Header: 01 00 00 00
   Body:   05
   ```

2. Server → Client: Character List (2 characters)
   ```
   Header: 20 00 00 00
   Body:   06 02 00 01 00 00 00 08 "Barbarian" 01 60 02 00 00 00 05 "Wizard" 03 45
   ```

## Implementation Notes

- All multi-byte integer values are transmitted in little-endian format.
- Strings are transmitted with a length prefix followed by the raw string bytes (no null terminator).
- The server enforces a maximum message size (currently 1MB) to prevent memory exhaustion attacks.
- Clients should handle connection drops and implement reconnection logic.
- Authentication uses plaintext passwords in this simple implementation, but a real implementation should use a secure authentication mechanism. 