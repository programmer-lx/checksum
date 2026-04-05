# Checksum库开发规则

## 项目构建规则

### 1. C标准要求
- 必须使用C11标准
- 启用所有警告（-Wall）
- 将警告视为错误（-Werror）

### 2. 编译器要求
- 支持的编译器：MSVC, GCC, Clang, Clang-cl, MinGW
- 编译器版本要求：C11支持
- Windows平台：使用Unicode字符集

### 3. 构建选项
- 测试构建选项（CKS_BUILD_TESTS=ON）

## 跨平台要求

### 1. 支持平台
- x86_64, arm64

### 2. 平台特定代码
- 使用条件编译处理平台差异

## 问题报告

- 清晰的问题描述
- 复现步骤
- 环境信息
- 预期和实际结果

## 架构设计

### 核心组件
1. **include/checksum/** - 公共头文件
   - crc32c.h：CRC32C算法
   - md5.h: MD5算法
   - sha256.h: SHA-256算法

2. **include/checksum/detail/** - 配置和检测
   - attributes.h：编译器特性，比如 `__attribute__` `__declspec` 等
   - cpu.h/c：CPU信息检测
   - compiler.h：编译器判断

3. **src/** - 源代码实现
   - crc32c.c：CRC32C算法实现
   - md5.c: MD5算法实现
   - sha256.c: SHA-256算法实现

4. **tests/** - 测试套件 (不链接gtest-main，需要在每个测试文件的最后补充main函数)
   - crc32c.cpp：CRC32C测试用例
   - md5.cpp: MD5测试用例
   - sha256.cpp: SHA-256测试用例

### 最优指令集选取规则
参考 @include/sha256.h, @src/sha256.c 文件:
1. 声明并实现软件版本，硬件指令版本 (函数的前缀是 cks_impl_)
2. 编写Dispatcher(如果需要)，使用线程安全的 cks_call_once 调用函数指针表初始化函数，初始化一个静态的 Dispatcher
3. 如果这个函数有Dispatcher，则实现主接口，比如 cks_sha256_update ，这个接口只是一个转发器，调用步骤2初始化后的Dispatcher的函数

### 设计原则
- 零开销抽象：使用编译时特性检测(如果可能)
- 自动选择最优硬件指令：判断CPU特性，选择最优的函数执行
- 跨平台：支持多种操作系统和架构
