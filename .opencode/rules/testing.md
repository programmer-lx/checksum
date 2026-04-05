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
// 辅助函数：将SHA256结果转换为hex字符串
static std::string to_hex(const cks_SHA256& hash)
{
  const char hex_chars[] = "0123456789abcdef";
  std::string result;
  result.reserve(64);
  for (int i = 0; i < 32; ++i)
  {
    result += hex_chars[(hash.bytes[i] >> 4) & 0xF];
    result += hex_chars[hash.bytes[i] & 0xF];
  }
  return result;
}

// 1. 已知值测试（NIST标准向量）
// https://www.di-mgt.com.au/sha_testvectors.html
TEST(SHA256, Standard)
{
  // 测试向量: {输入, 期望的hex结果}
  std::vector<std::pair<std::string, std::string>> test_vectors = {
    // 空字符串
    {"", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"},

    // "abc"
    {"abc", "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"},

    // "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
    {"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"},

    // abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu
    {"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
      "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1"},
  };

  // 1'000'000 个 "a"
  test_vectors.emplace_back(std::string(1'000'000, 'a'), "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");

  int idx = 0;
  for (const auto& vec : test_vectors)
  {
    // 软件实现
    cks_SHA256_Context soft_ctx = cks_sha256_begin();
    cks_impl_sha256_update_soft(&soft_ctx, vec.first.data(), vec.first.size());
    cks_SHA256 soft_result = cks_impl_sha256_end_soft(&soft_ctx);
    EXPECT_EQ(to_hex(soft_result), vec.second) << "[soft] idx: " << idx;

    // 通用接口
    {
      cks_SHA256_Context ctx = cks_sha256_begin();
      cks_sha256_update(&ctx, vec.first.data(), vec.first.size());
      cks_SHA256 result = cks_sha256_end(&ctx);
      EXPECT_EQ(to_hex(result), vec.second);
    }

    #if CKS_ARCH_X86
    // x86 SHA-NI实现
    {
      cks_SHA256_Context shani_ctx = cks_sha256_begin();
      cks_impl_sha256_update_sha(&shani_ctx, vec.first.data(), vec.first.size());
      cks_SHA256 shani_result = cks_impl_sha256_end_sha(&shani_ctx);
      EXPECT_EQ(to_hex(shani_result), vec.second) << "[x86] idx: " << idx;
    }
    #endif

    #if CKS_ARCH_ARM
    // ARM实现
    {
      cks_SHA256_Context arm_ctx = cks_sha256_begin();
      cks_impl_sha256_update_arm(&arm_ctx, vec.first.data(), vec.first.size());
      cks_SHA256 arm_result = cks_impl_sha256_end_arm(&arm_ctx);
      EXPECT_EQ(to_hex(arm_result), vec.second) << "[arm] idx: " << idx;
    }
    #endif

    ++idx;
  }
}

// 2. 空Buffer测试
TEST(SHA256, EmptyBuffer)
{
  // 空字符串的SHA256（NIST标准值）
  const char* expected_hex = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";

  cks_SHA256_Context ctx = cks_sha256_begin();
  cks_SHA256 result = cks_sha256_end(&ctx);

  EXPECT_EQ(to_hex(result), expected_hex);

  // 显式传入nullptr和0
  ctx = cks_sha256_begin();
  cks_sha256_update(&ctx, nullptr, 0);
  result = cks_sha256_end(&ctx);

  EXPECT_EQ(to_hex(result), expected_hex);

  // 软件实现
  cks_SHA256_Context soft_ctx = cks_sha256_begin();
  cks_impl_sha256_update_soft(&soft_ctx, nullptr, 0);
  cks_SHA256 soft_result = cks_impl_sha256_end_soft(&soft_ctx);

  EXPECT_EQ(to_hex(soft_result), expected_hex);

#if CKS_ARCH_X86
  // x86 SHA-NI实现
  {
    cks_SHA256_Context shani_ctx = cks_sha256_begin();
    cks_impl_sha256_update_sha(&shani_ctx, nullptr, 0);
    cks_SHA256 shani_result = cks_impl_sha256_end_sha(&shani_ctx);
    EXPECT_EQ(to_hex(shani_result), expected_hex);
  }
#endif

#if CKS_ARCH_ARM
  // ARM实现
  {
    cks_SHA256_Context arm_ctx = cks_sha256_begin();
    cks_impl_sha256_update_arm(&arm_ctx, nullptr, 0);
    cks_SHA256 arm_result = cks_impl_sha256_end_arm(&arm_ctx);
    EXPECT_EQ(to_hex(arm_result), expected_hex);
  }
#endif
}
```