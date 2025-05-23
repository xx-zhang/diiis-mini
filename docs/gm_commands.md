# GM Commands

This document outlines the available Game Master (GM) commands for server administration.

## Using the GM Command System

There are two ways to use GM commands:

1. **In-game commands**: Used by admins while connected to the game
2. **REST API endpoints**: Used for external administration

## Authentication

For in-game commands, you must be logged in with an account that has appropriate admin privileges.

For REST API commands, you must authenticate using an admin API key or credentials.

## Available Commands

### Account Management

#### Create Account

**In-game:**
```
!account add <login> <password> <battleTag> [role]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/accounts" \
  -H "Content-Type: application/json" \
  -d '{
    "login": "username@",
    "password": "YourPassword",
    "battleTag": "YourBattleTag",
    "role": "user"
  }'
```

Roles: `user`, `gm`, `admin`, `owner`

#### Delete Account

**In-game:**
```
!account delete <login>
```

**REST API:**
```bash
curl -X DELETE "http://localhost:8080/api/accounts/<login>" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

#### Ban Account

**In-game:**
```
!account ban <login> <reason> [duration]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/accounts/<login>/ban" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "reason": "Violation of rules",
    "duration": "7d"
  }'
```

Duration format: `[number][unit]` where unit can be `m` (minutes), `h` (hours), `d` (days)

#### List Accounts

**In-game:**
```
!account list [page] [limit]
```

**REST API:**
```bash
curl -X GET "http://localhost:8080/api/accounts?page=1&limit=20" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

### Character Management

#### Create Character

**In-game:**
```
!character create <accountLogin> <characterName> <class> <level>
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/accounts/<login>/characters" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "name": "CharacterName",
    "class": "barbarian",
    "level": 70
  }'
```

Classes: `barbarian`, `crusader`, `demon_hunter`, `monk`, `witch_doctor`, `wizard`, `necromancer`

#### Delete Character

**In-game:**
```
!character delete <characterId>
```

**REST API:**
```bash
curl -X DELETE "http://localhost:8080/api/characters/<characterId>" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

#### Modify Character

**In-game:**
```
!character modify <characterId> <property> <value>
```

**REST API:**
```bash
curl -X PATCH "http://localhost:8080/api/characters/<characterId>" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "property": "level",
    "value": 70
  }'
```

### World Management

#### Restart World

**In-game:**
```
!world restart <worldId>
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/worlds/<worldId>/restart" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

#### List Worlds

**In-game:**
```
!world list
```

**REST API:**
```bash
curl -X GET "http://localhost:8080/api/worlds" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

### Item Management

#### Give Item

**In-game:**
```
!item give <characterId> <itemId> [quality] [quantity]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/characters/<characterId>/items" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "itemId": "Unique_Sword_2H_001_x1",
    "quality": "legendary",
    "quantity": 1
  }'
```

### Server Management

#### Shutdown Server

**In-game:**
```
!server shutdown [delay]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/server/shutdown" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "delay": "5m"
  }'
```

#### Restart Server

**In-game:**
```
!server restart [delay]
```

**REST API:**
```bash
curl -X POST "http://localhost:8080/api/server/restart" \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_API_KEY" \
  -d '{
    "delay": "5m"
  }'
```

#### Server Status

**In-game:**
```
!server status
```

**REST API:**
```bash
curl -X GET "http://localhost:8080/api/server/status" \
  -H "Authorization: Bearer YOUR_API_KEY"
```

## Logging

All GM commands are logged for security purposes. Logs can be accessed:

**In-game:**
```
!logs view [lines]
```

**REST API:**
```bash
curl -X GET "http://localhost:8080/api/logs?lines=100" \
  -H "Authorization: Bearer YOUR_API_KEY"
``` 