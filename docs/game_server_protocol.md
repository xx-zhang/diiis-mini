# D3Server Game Server Protocol Documentation

This document describes the Game Server protocol used for in-game communication between the D3Server and game clients.

## Overview

The Game Server protocol is a binary protocol designed for real-time game state updates, player actions, and world state synchronization. It uses a message-based format with a header followed by a variable-length body.

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

| Type (Hex) | Description               | Format                                                |
|------------|---------------------------|-------------------------------------------------------|
| 0x01       | Welcome                   | [0x01] [Version (4 bytes)]                           |
| 0x02       | Authentication Result     | [0x02] [Result (1 = Success, 0 = Fail)]              |
| 0x04       | Pong                      | [0x04]                                               |
| 0x06       | Enter Session Confirmation| [0x06] [Session ID (4 bytes)]                        |
| 0x07       | Player Joined             | [0x07] [Player Data]                                 |
| 0x08       | Player Left               | [0x08] [Player ID (4 bytes)]                         |
| 0x0A       | World State               | [0x0A] [World Data]                                  |
| 0x0C       | Entity Update             | [0x0C] [Entity ID (4 bytes)] [Entity Data]           |
| 0x0E       | Chat Message              | [0x0E] [Sender ID (4 bytes)] [Msg Len] [Message]     |
| 0x10       | Game Event                | [0x10] [Event Type (1 byte)] [Event Data]            |
| 0x12       | Item Data                 | [0x12] [Item ID (4 bytes)] [Item Data]               |
| 0x14       | Skill Effect              | [0x14] [Caster ID (4 bytes)] [Skill ID] [Effect Data]|

### Client to Server

| Type (Hex) | Description               | Format                                                   |
|------------|---------------------------|----------------------------------------------------------|
| 0x01       | Authentication Request    | [0x01] [Token Len (1 byte)] [Token] [Character ID (4 bytes)] |
| 0x03       | Ping                      | [0x03]                                                  |
| 0x05       | Enter Session Request     | [0x05] [Session ID (4 bytes) or 0 for new]              |
| 0x09       | Player Action             | [0x09] [Action Type (1 byte)] [Action Data]             |
| 0x0B       | Player Position           | [0x0B] [Position X (4 bytes)] [Position Y (4 bytes)] [Position Z (4 bytes)] |
| 0x0D       | Chat Message              | [0x0D] [Target Type (1 byte)] [Target ID (4 bytes)] [Msg Len] [Message] |
| 0x0F       | Skill Use                 | [0x0F] [Skill ID (2 bytes)] [Target Type] [Target Data] |
| 0x11       | Item Interaction          | [0x11] [Interaction Type] [Item ID (4 bytes)]           |
| 0x13       | Command                   | [0x13] [Command Len] [Command]                          |

## Player Data Format

The player data format used in Player Joined messages:

```
+----------------+----------------+------------------+----------------+------------------+
| Player ID      | Name Length    | Player Name      | Class ID       | Level            |
+----------------+----------------+------------------+----------------+------------------+
| Position X     | Position Y     | Position Z       | Rotation       | Visual Equipment |
+----------------+----------------+------------------+----------------+------------------+
```

## Action Types

| ID  | Action Name            |
|-----|------------------------|
| 0x01| Move                   |
| 0x02| Attack                 |
| 0x03| Use Skill              |
| 0x04| Use Item               |
| 0x05| Pick Up Item           |
| 0x06| Interact with Object   |
| 0x07| Cast Town Portal       |
| 0x08| Use Potion             |
| 0x09| Revive                 |

## Position Updates

Position updates (Type 0x0B) are frequent messages that update the server about the client's current position. For optimization, these messages use a compressed format with smaller floating-point representations.

## Entity Types

| ID  | Entity Type            |
|-----|------------------------|
| 0x01| Player                 |
| 0x02| NPC                    |
| 0x03| Monster                |
| 0x04| Item                   |
| 0x05| Projectile             |
| 0x06| Effect                 |
| 0x07| Interaction Object     |
| 0x08| Portal                 |

## World Data Format

The world data format provides information about the current game world:

```
+----------------+----------------+------------------+-------------------------+
| World ID       | Difficulty     | Game Mode        | Number of Entities      |
+----------------+----------------+------------------+-------------------------+
| Entity Array   | Item Array     | Object Array     | Terrain Seed            |
+----------------+----------------+------------------+-------------------------+
```

## Authentication Process

1. Server sends Welcome message (0x01) upon client connection.
2. Client sends Authentication Request (0x01) with a token from Battle.net server.
3. Server validates token and sends Authentication Result (0x02).
4. If authentication is successful, client can request to enter a session.

## Session Handling

1. Client sends Enter Session Request (0x05) with session ID (or 0 for a new session).
2. Server creates or joins session and sends Enter Session Confirmation (0x06).
3. Server sends Player Joined (0x07) messages for all existing players in the session.
4. Server sends World State (0x0A) with the current game state.

## Example Message Flow

### Authentication

1. Server → Client: Welcome
   ```
   Header: 05 00 00 00
   Body:   01 01 00 00 00
   ```

2. Client → Server: Authentication Request
   ```
   Header: 15 00 00 00
   Body:   01 10 "auth_token_value" 01 00 00 00
   ```

3. Server → Client: Authentication Result (Success)
   ```
   Header: 02 00 00 00
   Body:   02 01
   ```

### Enter Game Session

1. Client → Server: Enter Session Request (New Session)
   ```
   Header: 05 00 00 00
   Body:   05 00 00 00 00
   ```

2. Server → Client: Enter Session Confirmation
   ```
   Header: 05 00 00 00
   Body:   06 01 00 00 00
   ```

### Player Movement

1. Client → Server: Player Position
   ```
   Header: 0D 00 00 00
   Body:   0B 00 00 80 41 00 00 00 00 00 00 80 41
   ```

2. Server → Client: Entity Update (for other players)
   ```
   Header: 0D 00 00 00
   Body:   0C 02 00 00 00 00 00 80 41 00 00 00 00 00 00 80 41
   ```

## Implementation Notes

- All multi-byte integer values are transmitted in little-endian format.
- Floating-point values are transmitted in IEEE 754 format.
- Strings are transmitted with a length prefix followed by the raw string bytes (no null terminator).
- Position updates use a smaller format for bandwidth optimization.
- The server enforces a maximum message size (currently 1MB) to prevent memory exhaustion attacks.
- For better performance, position updates may be batched or sent at a lower frequency for distant players.
- The protocol uses binary format rather than text-based formats for better bandwidth efficiency.
- Delta compression may be applied to world state updates to reduce bandwidth usage.
- The protocol supports incremental updates to avoid sending complete world state for every change. 