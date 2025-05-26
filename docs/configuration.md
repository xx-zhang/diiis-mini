# 配置指南

本文档说明如何配置 D3Server。

## 主要配置文件

*   `config.json` (或类似名称)：描述此文件的用途和主要部分。

## 配置项说明

### 通用配置

*   `bindIp`: 服务器绑定IP地址。
*   `logLevel`: 日志级别 (例如, "debug", "info", "warning", "error")。

### 数据库配置

*   `database.type`: 数据库类型 (例如, "sqlite")。
*   `database.path`: SQLite 数据库文件路径。

### 战网服务器配置

*   `bnetServer.port`: 战网服务器端口。

### 游戏服务器配置

*   `gameServer.port`: 游戏服务器端口。

### REST API 配置

*   `restApi.port`: REST API 端口。
*   `restApi.apiKey`: REST API 的访问密钥。

## 示例配置

```json
{
  "bindIp": "0.0.0.0",
  "logLevel": "info",
  "database": {
    "type": "sqlite",
    "path": "d3server.db"
  },
  "bnetServer": {
    "port": 1119
  },
  "gameServer": {
    "port": 6112
  },
  "restApi": {
    "port": 8080,
    "apiKey": "your_secret_api_key_here"
  }
}
``` 
