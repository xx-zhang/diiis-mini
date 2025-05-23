# D3Server 调试指南

本文档提供有关 D3Server 调试功能的详细信息，包括日志系统、调试工具、审计日志和性能监控，帮助开发者和管理员诊断问题和优化服务器性能。

## 目录

- [日志系统](#日志系统)
- [调试模式](#调试模式)
- [审计日志](#审计日志)
- [性能监控](#性能监控)
- [错误排查](#错误排查)
- [调试命令](#调试命令)
- [常见问题](#常见问题)

## 日志系统

D3Server 使用多级日志系统记录运行信息，帮助识别和解决问题。

### 日志级别

- **DEBUG**: 详细的调试信息，包括函数调用、变量值等
- **INFO**: 一般操作信息，如服务器启动、新连接等
- **WARNING**: 潜在问题警告，不影响系统正常运行
- **ERROR**: 错误信息，表明功能未正确执行
- **CRITICAL**: 严重错误，可能导致服务崩溃

### 日志文件位置

默认情况下，日志文件存储在以下位置：

- 主日志: `logs/d3server.log`
- 审计日志: `logs/audit.log`
- 错误日志: `logs/error.log`

### 日志配置

可以在 `config.ini` 文件中配置日志系统：

```ini
[Logging]
LogLevel = INFO           # 日志级别: DEBUG, INFO, WARNING, ERROR, CRITICAL
LogToConsole = true       # 是否输出到控制台
LogFilePath = logs/d3server.log  # 日志文件路径
MaxLogFileSize = 10       # 单个日志文件最大大小(MB)
MaxLogFiles = 5           # 保留的日志文件数量
```

### 日志格式

日志条目格式如下：

```
2023-09-15 14:30:45.123 [INFO] Server started on port 8080
2023-09-15 14:31:12.456 [ERROR] Failed to connect to database: Connection refused
```

## 调试模式

调试模式提供更详细的日志和性能信息，适用于开发和故障排除。

### 启用调试模式

可以通过以下方式启用调试模式：

1. **启动参数**:
   ```bash
   ./d3server --debug
   ```

2. **配置文件**:
   ```ini
   [Server]
   DebugMode = true
   ```

3. **REST API**:
   ```bash
   curl -X POST "http://localhost:8080/api/admin/debug/enable" \
     -H "Authorization: Bearer YOUR_API_KEY"
   ```

### 调试宏

代码中的调试宏可以在定义了 `DEBUG_MODE` 时自动激活：

```cpp
DEBUG_LOG("消息内容");               // 记录一般调试消息
DEBUG_FUNCTION_ENTER();             // 记录函数进入
DEBUG_FUNCTION_EXIT();              // 记录函数退出
DEBUG_VARIABLE(variableName);       // 记录变量值
DEBUG_TIMER_START("操作名称");       // 开始计时操作
DEBUG_TIMER_STOP();                 // 停止计时并记录耗时
```

调试日志输出示例：

```
2023-09-15 14:35:22.123 [DEBUG] server.cpp:45 (Server::start) Function entered
2023-09-15 14:35:22.124 [DEBUG] server.cpp:50 (Server::start) port = 8080
2023-09-15 14:35:22.150 [DEBUG] server.cpp:120 (Server::start) Function exited
```

## 审计日志

审计日志系统记录关键操作和安全相关事件，用于故障排查和安全审计。

### 审计日志内容

审计日志记录以下类型的操作：

- **认证操作**: 登录尝试、登出、密码重置
- **账号管理**: 账号创建、修改、删除
- **角色管理**: 角色创建、属性修改、删除
- **游戏会话**: 会话创建、玩家加入/离开
- **配置更改**: 服务器设置修改
- **管理员操作**: 特权命令执行
- **安全事件**: 异常登录尝试、访问控制违规

### 审计日志格式

审计日志使用CSV格式，包含以下字段：

```
时间戳,用户ID,IP地址,操作类型,操作描述,结果,详细信息,源代码位置
```

示例：
```
2023-09-15 14:32:45.789,"admin","192.168.1.100","ADMIN_ACTION","Server Restart","SUCCESS","Scheduled maintenance","admin_controller.cpp:156 AdminController::restartServer"
```

### 审计日志配置

可以在配置文件中设置审计日志：

```ini
[Audit]
Enabled = true                  # 是否启用审计日志
LogFilePath = logs/audit.log    # 审计日志文件路径
MaxEntries = 10000              # 内存中保留的最大条目数
```

### 查看审计日志

1. **通过REST API查看**:
   ```bash
   curl -X GET "http://localhost:8080/api/admin/logs/audit?type=AUTHENTICATION&limit=100" \
     -H "Authorization: Bearer YOUR_API_KEY"
   ```

2. **导出为CSV**:
   ```bash
   curl -X GET "http://localhost:8080/api/admin/logs/audit/export" \
     -H "Authorization: Bearer YOUR_API_KEY" \
     -o audit_export.csv
   ```

3. **过滤审计日志**:
   
   可以按用户、操作类型、结果类型进行过滤：
   ```bash
   curl -X GET "http://localhost:8080/api/admin/logs/audit?user=admin&result=FAILURE" \
     -H "Authorization: Bearer YOUR_API_KEY"
   ```

## 性能监控

D3Server 包含多种性能监控工具，帮助识别瓶颈和优化性能。

### 性能指标

系统跟踪以下性能指标：

- **CPU利用率**: 整体和每个线程的CPU使用情况
- **内存使用**: 堆内存、栈内存、系统内存使用情况
- **网络流量**: 传入/传出流量，连接数量
- **数据库性能**: 查询时间、连接数、缓存命中率
- **请求延迟**: 不同操作的平均响应时间
- **并发度**: 同时在线玩家数、活动会话数

### 查看性能统计

1. **通过REST API查看**:
   ```bash
   curl -X GET "http://localhost:8080/api/admin/stats" \
     -H "Authorization: Bearer YOUR_API_KEY"
   ```

2. **实时监控**:
   管理界面提供实时性能监控页面，访问：
   ```
   http://localhost:8080/admin/dashboard
   ```

3. **性能日志**:
   在调试模式下，性能指标会定期写入日志文件:
   ```
   logs/performance.log
   ```

### 性能分析

1. **函数计时**:
   
   使用调试计时器分析函数性能：
   ```cpp
   DEBUG_TIMER_START("DatabaseQuery");
   // 执行查询...
   DEBUG_TIMER_STOP(); // 输出: "Operation 'DatabaseQuery' completed in XXX μs"
   ```

2. **内存分析**:
   
   使用 Valgrind 或其他内存分析工具：
   ```bash
   valgrind --tool=massif ./d3server
   ```

## 错误排查

### 常见错误代码

| 错误代码 | 描述 | 可能原因 | 解决方案 |
|---------|------|---------|---------|
| ERR_DB_001 | 数据库连接失败 | SQLite文件权限问题或文件损坏 | 检查文件权限和完整性 |
| ERR_NET_001 | 端口绑定失败 | 端口已被占用 | 更改配置中的端口或释放该端口 |
| ERR_AUTH_001 | 认证失败 | 密码错误或账号不存在 | 检查账号状态和密码 |
| ERR_MEM_001 | 内存分配失败 | 系统内存不足 | 增加系统内存或减少服务器负载 |

### 调试工具

1. **内存泄漏检测**:
   ```bash
   valgrind --leak-check=full ./d3server
   ```

2. **线程分析**:
   ```bash
   valgrind --tool=helgrind ./d3server
   ```

3. **核心转储分析**:
   ```bash
   gdb ./d3server core.XXXX
   ```

## 调试命令

D3Server 支持多种调试命令，方便开发和管理。

### REST API 调试命令

| 命令 | 描述 | 示例 |
|-----|------|-----|
| `/api/admin/debug/threads` | 显示线程信息 | `curl -X GET "http://localhost:8080/api/admin/debug/threads"` |
| `/api/admin/debug/memory` | 显示内存使用情况 | `curl -X GET "http://localhost:8080/api/admin/debug/memory"` |
| `/api/admin/debug/connections` | 显示当前连接 | `curl -X GET "http://localhost:8080/api/admin/debug/connections"` |
| `/api/admin/debug/gc` | 触发垃圾收集 | `curl -X POST "http://localhost:8080/api/admin/debug/gc"` |

### 控制台命令

在服务器控制台中可使用以下命令：

```
debug enable              # 启用调试模式
debug disable             # 禁用调试模式
debug memory              # 显示内存使用
debug threads             # 显示线程信息
debug log <level>         # 设置日志级别
debug reload-config       # 重新加载配置
audit enable              # 启用审计
audit disable             # 禁用审计
```

## 常见问题

### 日志文件持续增长

**问题**: 日志文件不断增长，占用大量磁盘空间。

**解决方案**: 启用日志轮转功能：

```ini
[Logging]
EnableRotation = true
MaxLogFileSize = 10       # MB
MaxLogFiles = 5
```

### 性能下降

**问题**: 服务器随时间性能下降。

**排查步骤**:
1. 检查内存泄漏: `valgrind --leak-check=full ./d3server`
2. 查看性能日志: `logs/performance.log`
3. 检查数据库索引
4. 分析连接数和网络流量

### 服务器崩溃

**问题**: 服务器意外崩溃。

**排查步骤**:
1. 检查核心转储: `gdb ./d3server core.XXXX`
2. 查看错误日志: `logs/error.log`
3. 启用更详细的调试信息: `./d3server --debug`
4. 检查系统资源使用情况: `top`, `htop`, `iotop`

### 无法连接数据库

**问题**: 服务器无法连接到数据库。

**解决方案**:
1. 检查数据库文件权限
2. 确保数据库目录存在并可写
3. 验证数据库未损坏: `sqlite3 <db-file> "PRAGMA integrity_check;"`

---

如需更多帮助，请提交 GitHub issue 或联系项目维护人员。