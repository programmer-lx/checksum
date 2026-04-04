# Checksum库开发规则

## 项目构建规则

### 1. C++标准要求
- 必须使用C++20标准
- 禁用异常（-fno-exceptions）
- 禁用运行时类型信息（-fno-rtti）
- 启用所有警告（-Wall）
- 将警告视为错误（-Werror）

### 2. 编译器要求
- 支持的编译器：MSVC, GCC, Clang, Clang-cl, MinGW
- 编译器版本要求：C++20支持
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
   - crc32c.hpp：CRC32C算法
   - md5.hpp: MD5算法
   - sha256.hpp: SHA-256算法

2. **include/checksum/detail/** - 配置和检测
   - attributes.hpp：编译器特性，比如 `__attribute__` `__declspec` 等
   - cpu.hpp/cpp：CPU信息检测
   - compiler.hpp：编译器判断

3. **src/** - 源代码实现
   - crc32c.cpp：CRC32C算法实现
   - md5.cpp: MD5算法实现
   - sha256.cpp: SHA-256算法实现

4. **tests/** - 测试套件 (不链接gtest-main，需要在每个测试文件的最后补充main函数)
   - crc32c.cpp：CRC32C测试用例
   - md5.cpp: MD5测试用例
   - sha256.cpp: SHA-256测试用例

### 最优指令集选取规则
参考 include/crc32c.hpp, src/crc32c.cpp 文件:
1. 声明并实现软件版本，硬件指令版本 (在命名空间 detail 中)
2. 实现在匿名命名空间的函数，那个函数用于判断cpu feature，并返回一个最优的函数指针
3. 实现主接口，比如 crc32c_update ，这个接口只是一个转发器，调用步骤2所返回的函数指针

### 设计原则
- 零开销抽象：使用编译时特性检测(如果可能)
- 自动选择最优硬件指令：判断CPU特性，选择最优的函数执行
- 跨平台：支持多种操作系统和架构
- 无异常：禁用异常处理
- 无RTTI：禁用运行时类型信息
