# 数据库设计

本文档描述了 D3Server 项目的数据库结构和设计。

## 数据库类型

当前版本使用 SQLite 作为主要数据库。

## 数据表结构

### 1. `accounts` 表

*   `id` (INTEGER, PRIMARY KEY, AUTOINCREMENT): 账户唯一ID。
*   `login` (TEXT, UNIQUE, NOT NULL): 登录名。
*   `password_hash` (TEXT, NOT NULL): 加密后的密码。
*   `email` (TEXT, UNIQUE): 电子邮箱。
*   `creation_date` (DATETIME, DEFAULT CURRENT_TIMESTAMP): 创建日期。
*   `last_login` (DATETIME): 最后登录日期。
*   `is_banned` (BOOLEAN, DEFAULT 0): 是否被封禁。
*   `user_roles` (TEXT): 用户角色 (例如, 'admin', 'player')。

### 2. `characters` 表

*   `id` (INTEGER, PRIMARY KEY, AUTOINCREMENT): 角色唯一ID。
*   `account_id` (INTEGER, NOT NULL, FOREIGN KEY REFERENCES `accounts`(`id`)): 所属账户ID。
*   `name` (TEXT, NOT NULL): 角色名称。
*   `class` (INTEGER, NOT NULL): 角色职业 (例如, 0: 野蛮人, 1: 魔法师等)。
*   `level` (INTEGER, DEFAULT 1): 角色等级。
*   `experience` (INTEGER, DEFAULT 0): 经验值。
*   `stats` (TEXT): 角色属性 (JSON 格式存储，例如力量、敏捷等)。
*   `inventory` (TEXT): 角色物品栏 (JSON 格式存储)。
*   `creation_date` (DATETIME, DEFAULT CURRENT_TIMESTAMP): 创建日期。

### 3. (其他表...)

请根据需要添加更多表结构描述。

## 注意事项

*   所有密码均应加密存储，不可明文存储。
*   敏感数据操作应有日志记录。 
