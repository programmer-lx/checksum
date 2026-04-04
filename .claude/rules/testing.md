# 测试规则

## 1. 测试框架
- 使用Google Test v1.17.0
- 测试目录：tests/
- cmake添加测试函数：cks_add_test(test_file, test_name)

## 2. 测试类型要求
- 经典值测试：使用多个已知值进行验证，需要上网查找最官方、最权威的资料
- 空buffer测试：传入空指针，size=0
- 小buffer测试：1-512字节数据块
- 大buffer测试：1-10MB数据块
- 分段校验一致性测试：验证分段计算结果
- 非对齐buffer测试：使用alignas(4+1)进行偏移测试
- 各种边界条件测试

## 3. 运行测试脚本
- x86平台: `cd {PROJECT_ROOT} && python scripts/test_x86.py`
- arm平台: `cd {PROJECT_ROOT} && python scripts/test_arm.py`

## 示例测试用例

```cpp
// 已知值测试示例
TEST(CRC32C, standard)
{
  std::pair<std::string, uint32_t> values[] = {
    {"123456789", 0xE3069283},
    {"6516861AVSDV", 0xD3936279},
    {"aoscjnfsoidjc", 0xB92A3D05},
    {"/*-+-*/*-*/-*qewreqw", 0x92187ED5}
  };

  for (size_t i = 0; i < std::size(values); ++i)
  {
    size_t len = values[i].first.size();
    const void* data = values[i].first.c_str();

    uint32_t soft_res = crc32c_end(detail::crc32c_update_soft(crc32c_begin(), data, len));
    uint32_t expected = values[i].second;

    EXPECT_EQ(soft_res, expected);

    #if CKS_ARCH_X86
    uint32_t sse_res = crc32c_end(detail::crc32c_update_sse42(crc32c_begin(), data, len));
    EXPECT_EQ(sse_res, expected);
    #endif

    #if CKS_ARCH_ARM
    uint32_t arm_res = crc32c_end(detail::crc32c_update_arm(crc32c_begin(), data, len));
    EXPECT_EQ(arm_res, expected);
    #endif
  }
}

// 小buffer测试示例
TEST(CRC32C, SmallBuffer)
{
  // soft
  const uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
  CRC32C soft_result = cks::crc32c_begin();
  soft_result = cks::detail::crc32c_update_soft(soft_result, data, sizeof(data));
  soft_result = cks::crc32c_end(soft_result);

  // x86
  #if CKS_ARCH_X86
  {
    CRC32C result = cks::crc32c_begin();
    result = cks::detail::crc32c_update_sse42(result, data, sizeof(data));
    result = cks::crc32c_end(result);
    EXPECT_EQ(result, soft_result);
  }
  #endif

  // arm
  #if CKS_ARCH_ARM
  {
    CRC32C result = cks::crc32c_begin();
    result = cks::detail::crc32c_update_arm(result, data, sizeof(data));
    result = cks::crc32c_end(result);
    EXPECT_EQ(result, soft_result);
  }
  #endif
}
```