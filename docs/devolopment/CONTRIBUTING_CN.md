# D3Server 贡献指南

感谢您对 D3Server 项目的兴趣！本指南将帮助您了解如何为项目做出贡献，包括代码规范、提交流程、测试要求、提交PR流程等信息。

## 目录

- [开发环境设置](#开发环境设置)
- [代码风格](#代码风格)
- [分支管理](#分支管理)
- [提交更改](#提交更改)
- [测试要求](#测试要求)
- [调试指南](#调试指南)
- [文档](#文档)
- [常见问题解答](#常见问题解答)

## 开发环境设置

1. **克隆仓库**

   ```bash
   git clone https://github.com/example/d3server.git
   cd d3server
   ```

2. **安装依赖**

   在 Ubuntu/Debian 系统上：
   ```bash
   sudo apt-get update
   sudo apt-get install -y build-essential cmake libsqlite3-dev libssl-dev libgtest-dev libboost-all-dev
   ```

   在 CentOS/RHEL 系统上：
   ```bash
   sudo yum install -y gcc-c++ cmake sqlite-devel openssl-devel gtest-devel boost-devel
   ```

3. **构建项目**

   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

4. **运行测试**

   ```bash
   make test
   ```

## 代码风格

我们使用基于 Google C++ 风格指南的代码规范。以下是主要规则：

1. **命名约定**
   - 类名：使用首字母大写的 PascalCase，例如 `GameServer`
   - 方法和函数：使用小驼峰命名法，例如 `createAccount()`
   - 成员变量：使用 m_ 前缀，例如 `m_dbManager`
   - 常量：使用全大写和下划线，例如 `MAX_PLAYERS`
   - 命名空间：使用小写，例如 `d3server::core`

2. **缩进和格式**
   - 使用 4 个空格缩进（不使用制表符）
   - 行长度限制为 100 个字符
   - 大括号始终使用新行
   - 每个语句后添加分号，即使是单行代码块

3. **注释规范**
   - 所有公共 API 应有 Doxygen 风格的注释
   - 复杂算法或不明显的代码应有解释性注释
   - 使用 `TODO:`、`FIXME:` 等标签标记需要改进的代码

4. **其他规则**
   - 使用 `nullptr` 而不是 `NULL` 或 `0`
   - 包含头文件时使用 `#pragma once` 作为防护
   - 优先使用 `std::unique_ptr` 和 `std::shared_ptr` 而非原始指针
   - 优先使用 C++17 特性，如 structured bindings，`std::optional` 等

## 分支管理

我们使用以下分支模型：

- `master`: 主分支，包含稳定发布版本
- `develop`: 开发分支，用于集成功能
- `feature/*`: 新功能分支
- `bugfix/*`: 错误修复分支
- `hotfix/*`: 紧急修复分支

开发新功能或修复 bug 时，请从 `develop` 分支创建一个新的特性/错误修复分支：

```bash
git checkout develop
git pull
git checkout -b feature/your-feature-name
```

## 提交更改

1. **提交前检查**
   - 确保代码遵循代码风格
   - 确保所有测试通过
   - 确保没有引入新的编译警告

2. **提交信息规范**
   
   使用清晰的提交信息，格式如下：
   ```
   类型(作用域): 简短描述

   详细描述（如果需要）
   
   解决问题 #123
   ```

   类型可以是：
   - `feat`: 新功能
   - `fix`: 错误修复
   - `docs`: 文档更改
   - `style`: 代码风格调整（不影响代码逻辑）
   - `refactor`: 代码重构
   - `perf`: 性能优化
   - `test`: 测试相关
   - `chore`: 构建过程或辅助工具的变动

3. **创建拉取请求（PR）**
   
   推送您的分支并在 GitHub 上创建拉取请求：
   
   ```bash
   git push origin feature/your-feature-name
   ```
   
   PR 标题应遵循与提交信息相同的格式。

4. **代码审查**
   
   所有代码更改必须经过至少一名项目维护者的审查。请耐心等待反馈，并根据反馈进行必要的更改。

## 测试要求

所有新代码都必须包含适当的测试：

1. **单元测试**
   - 每个新功能必须有对应的单元测试
   - 每个修复的bug应有测试以防止回归
   - 使用 Google Test 框架

2. **代码覆盖率**
   - 新代码应至少有 80% 的测试覆盖率
   - 对于核心功能，建议达到 90% 以上的覆盖率

3. **测试结构**
   - 测试应放在 `test/unit/` 目录下
   - 测试文件命名应使用 `{component}_tests.cpp` 格式
   - 测试类应使用 `{Component}Test` 命名

4. **测试示例**

   ```cpp
   #include <gtest/gtest.h>
   #include "core/config.h"
   
   namespace d3server {
   namespace core {
   namespace test {
   
   class ConfigTest : public ::testing::Test {
   protected:
       void SetUp() override {
           m_config = std::make_shared<Config>();
           ASSERT_TRUE(m_config->init(":memory:"));
       }
       
       void TearDown() override {
           m_config.reset();
       }
       
       std::shared_ptr<Config> m_config;
   };
   
   TEST_F(ConfigTest, ServerConfig) {
       auto serverConfig = m_config->getServerConfig();
       EXPECT_FALSE(serverConfig.serverName.empty());
       
       serverConfig.serverName = "TestServer";
       EXPECT_TRUE(m_config->updateServerConfig(serverConfig));
       
       auto updatedConfig = m_config->getServerConfig();
       EXPECT_EQ(updatedConfig.serverName, "TestServer");
   }
   
   } // namespace test
   } // namespace core
   } // namespace d3server
   ```

## 调试指南

项目包含强大的调试工具，有助于快速定位和修复问题：

1. **调试宏**

   项目提供了一系列调试宏，在定义了 `DEBUG_MODE` 后可用：
   
   ```cpp
   DEBUG_LOG("消息内容");               // 记录一般调试消息
   DEBUG_FUNCTION_ENTER();             // 记录函数进入
   DEBUG_FUNCTION_EXIT();              // 记录函数退出
   DEBUG_VARIABLE(variableName);       // 记录变量值
   DEBUG_TIMER_START("操作名称");       // 开始计时
   DEBUG_TIMER_STOP();                 // 停止计时并记录耗时
   ```

2. **启用调试模式**

   在 CMake 中启用调试模式：
   
   ```bash
   cmake -DDEBUG_MODE=ON ..
   ```

3. **调试日志**

   调试日志包含详细信息，如：
   - 时间戳（精确到毫秒）
   - 线程 ID
   - 源文件名和行号
   - 函数名
   - 上下文信息（如变量值）

4. **性能分析**

   使用定时器分析性能瓶颈：
   
   ```cpp
   void someExpensiveOperation() {
       DEBUG_TIMER_START("ExpensiveOperation");
       
       // 执行耗时操作...
       
       DEBUG_TIMER_STOP(); // 输出操作耗时
   }
   ```

5. **自定义调试回调**

   可以注册自定义回调来处理调试信息：
   
   ```cpp
   utils::Debug::getInstance().setLogCallback([](const std::string& message) {
       // 处理调试消息，例如发送到远程监控系统
   });
   ```

## 文档

代码更改应同时更新相关文档：

1. **API 文档**
   - 更新 API 端点信息
   - 包含请求和响应示例

2. **设计文档**
   - 更新架构图
   - 记录设计决策和权衡

3. **中文文档**
   - 所有核心文档应同时提供中文版本
   - 中文文档文件使用 `*_cn.md` 命名

## 常见问题解答

**Q: 如何运行特定的测试？**

A: 您可以使用 Google Test 过滤器运行特定测试：

```bash
./tests/unit_tests --gtest_filter=ConfigTest.ServerConfig
```

**Q: 如何处理第三方依赖？**

A: 首选使用系统包管理器安装依赖。对于特殊需求，可以使用 git submodule 或 CMake FetchContent。

**Q: 如何贡献文档而非代码？**

A: 文档贡献与代码贡献流程相同。创建分支，更新文档，提交 PR。

**Q: 如何报告 bug？**

A: 在 GitHub issue 跟踪器中创建新 issue，提供以下信息：
- 问题描述
- 重现步骤
- 期望行为
- 实际行为
- 环境信息（操作系统、编译器版本等）
- 如可能，附上日志和截图

---

感谢您的贡献！如有任何疑问，请联系项目维护者。 