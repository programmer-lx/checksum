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

## 测试规则

### 1. 测试框架
- 使用Google Test v1.17.0
- 测试目录：tests/
- cmake添加测试函数：cks_add_test(test_file, test_name)

### 2. 测试类型要求
- 空buffer测试：传入空指针，size=0
- 小buffer测试：1-512字节数据块
- 大buffer测试：1-10MB数据块
- 分段校验一致性测试：验证分段计算结果
- 非对齐buffer测试：使用alignas(4+1)进行偏移测试
- 经典值测试：使用已知值进行验证
- 各种边界条件测试

### 3. 运行测试脚本
- x86平台: `cd {PROJECT_ROOT} && python scripts/test_x86.py`
- arm平台: `cd {PROJECT_ROOT} && python scripts/test_arm.py`


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

2. **src/config/** - 配置和检测
   - attributes.hpp：编译器特性
   - cpu.hpp/cpp：CPU信息检测
   - compiler.hpp：编译器检测

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
1. 先声明并实现软件版本，硬件指令版本 (在命名空间 detail_XXX 中)
2. 再实现一个函数，那个函数用于判断cpu feature，并返回一个最优的函数指针
3. 再实现一个主接口，比如 crc32c_update ，这个接口只是一个转发器，用于调用最优的函数指针

### 设计原则
- 零开销抽象：使用编译时特性检测(如果可能)
- 自动选择最优硬件指令：判断CPU特性，选择最优的函数执行
- 跨平台：支持多种操作系统和架构
- 无异常：禁用异常处理
- 无RTTI：禁用运行时类型信息

## 示例测试用例

```cpp
// 小buffer测试示例
TEST(CRC32C, SmallBuffer)
{
    // soft
    const uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    CRC32C soft_result = cks::crc32c_begin();
    soft_result = cks::detail::crc32c_update_soft(soft_result, data, sizeof(data));
    soft_result = cks::crc32c_end(soft_result);
    EXPECT_EQ(soft_result, expected_value);

    // x86
    #if CKS_ARCH_X86
    {
        const uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
        CRC32C result = cks::crc32c_begin();
        result = cks::detail::crc32c_update_sse42(result, data, sizeof(data));
        result = cks::crc32c_end(result);
        EXPECT_EQ(result, soft_result);
    }
    #endif

    // arm
    #if CKS_ARCH_ARM
    {
        const uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
        CRC32C result = cks::crc32c_begin();
        result = cks::detail::crc32c_update_arm(result, data, sizeof(data));
        result = cks::crc32c_end(result);
        EXPECT_EQ(result, soft_result);
    }
    #endif
}
```

## 注意事项

1. 所有测试必须通过
2. 代码质量检查必须通过
3. 构建必须成功
