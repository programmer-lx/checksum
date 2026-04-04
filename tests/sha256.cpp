#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include <string>
#include <utility>
#include "checksum/sha256.hpp"

using namespace cks;

// 辅助函数：将SHA256结果转换为hex字符串
static std::string to_hex(const SHA256& hash)
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
        SHA256_Context soft_ctx = sha256_begin();
        detail::sha256_update_soft(&soft_ctx, vec.first.data(), vec.first.size());
        SHA256 soft_result = sha256_end(&soft_ctx);
        EXPECT_EQ(to_hex(soft_result), vec.second) << "[soft] idx: " << idx;

        #if CKS_ARCH_X86
        // x86 SHA-NI实现
        {
            SHA256_Context shani_ctx = sha256_begin();
            detail::sha256_update_sha(&shani_ctx, vec.first.data(), vec.first.size());
            SHA256 shani_result = sha256_end(&shani_ctx);
            EXPECT_EQ(to_hex(shani_result), vec.second) << "[x86] idx: " << idx;
        }
        #endif

        #if CKS_ARCH_ARM
        // ARM实现
        {
            SHA256_Context arm_ctx = sha256_begin();
            detail::sha256_update_arm(&arm_ctx, vec.first.data(), vec.first.size());
            SHA256 arm_result = sha256_end(&arm_ctx);
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

    SHA256_Context ctx = sha256_begin();
    SHA256 result = sha256_end(&ctx);

    EXPECT_EQ(to_hex(result), expected_hex);

    // 显式传入nullptr和0
    ctx = sha256_begin();
    sha256_update(&ctx, nullptr, 0);
    result = sha256_end(&ctx);

    EXPECT_EQ(to_hex(result), expected_hex);

    // 软件实现
    SHA256_Context soft_ctx = sha256_begin();
    detail::sha256_update_soft(&soft_ctx, nullptr, 0);
    SHA256 soft_result = sha256_end(&soft_ctx);

    EXPECT_EQ(to_hex(soft_result), expected_hex);

#if CKS_ARCH_X86
    // x86 SHA-NI实现
    {
        SHA256_Context shani_ctx = sha256_begin();
        detail::sha256_update_sha(&shani_ctx, nullptr, 0);
        SHA256 shani_result = sha256_end(&shani_ctx);
        EXPECT_EQ(to_hex(shani_result), expected_hex);
    }
#endif

#if CKS_ARCH_ARM
    // ARM实现
    {
        SHA256_Context arm_ctx = sha256_begin();
        detail::sha256_update_arm(&arm_ctx, nullptr, 0);
        SHA256 arm_result = sha256_end(&arm_ctx);
        EXPECT_EQ(to_hex(arm_result), expected_hex);
    }
#endif
}

// 3. 小Buffer测试（1-128字节）
TEST(SHA256, SmallBuffer)
{
    // 测试各种小数据块
    for (size_t len = 1; len <= 128; ++len)
    {
        std::vector<uint8_t> data(len);
        for (size_t i = 0; i < len; ++i)
        {
            data[i] = static_cast<uint8_t>((i * 7 + 13) & 0xFF);
        }

        // 软件实现（基准）
        SHA256_Context soft_ctx = sha256_begin();
        detail::sha256_update_soft(&soft_ctx, data.data(), data.size());
        SHA256 soft_result = sha256_end(&soft_ctx);

        // 通用接口
        SHA256_Context ctx = sha256_begin();
        sha256_update(&ctx, data.data(), data.size());
        SHA256 result = sha256_end(&ctx);

        EXPECT_EQ(result, soft_result) << "Failed for length: " << len;

#if CKS_ARCH_X86
        // x86 SHA-NI实现
        {
            SHA256_Context shani_ctx = sha256_begin();
            detail::sha256_update_sha(&shani_ctx, data.data(), data.size());
            SHA256 shani_result = sha256_end(&shani_ctx);
            EXPECT_EQ(shani_result, soft_result) << "SHA-NI failed for length: " << len;
        }
#endif

#if CKS_ARCH_ARM
        // ARM实现
        {
            SHA256_Context arm_ctx = sha256_begin();
            detail::sha256_update_arm(&arm_ctx, data.data(), data.size());
            SHA256 arm_result = sha256_end(&arm_ctx);
            EXPECT_EQ(arm_result, soft_result) << "ARM failed for length: " << len;
        }
#endif
    }
}

// 4. 非对齐内存测试
TEST(SHA256, UnalignedMemory)
{
    // 创建对齐的数据，然后通过偏移访问来模拟非对齐
    alignas(64) uint8_t aligned_data[256];
    for (size_t i = 0; i < sizeof(aligned_data); ++i)
    {
        aligned_data[i] = static_cast<uint8_t>((i * 3 + 7) & 0xFF);
    }

    // 测试不同偏移量
    for (size_t offset = 1; offset <= 7; ++offset)
    {
        const uint8_t* unaligned_data = aligned_data + offset;
        const size_t len = 64;

        // 软件实现（基准）
        SHA256_Context soft_ctx = sha256_begin();
        detail::sha256_update_soft(&soft_ctx, unaligned_data, len);
        SHA256 soft_result = sha256_end(&soft_ctx);

        // 通用接口
        SHA256_Context ctx = sha256_begin();
        sha256_update(&ctx, unaligned_data, len);
        SHA256 result = sha256_end(&ctx);

        EXPECT_EQ(result, soft_result) << "Failed for offset: " << offset;

#if CKS_ARCH_X86
        {
            SHA256_Context shani_ctx = sha256_begin();
            detail::sha256_update_sha(&shani_ctx, unaligned_data, len);
            SHA256 shani_result = sha256_end(&shani_ctx);
            EXPECT_EQ(shani_result, soft_result) << "SHA-NI failed for offset: " << offset;
        }
#endif

#if CKS_ARCH_ARM
        {
            SHA256_Context arm_ctx = sha256_begin();
            detail::sha256_update_arm(&arm_ctx, unaligned_data, len);
            SHA256 arm_result = sha256_end(&arm_ctx);
            EXPECT_EQ(arm_result, soft_result) << "ARM failed for offset: " << offset;
        }
#endif
    }
}

// 5. 分段一致性测试（一次性 vs 分段）
TEST(SHA256, SegmentConsistency)
{
    // 测试数据
    std::vector<uint8_t> data(1024);
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] = static_cast<uint8_t>((i * 11 + 23) & 0xFF);
    }

    // 一次性计算（软件实现基准）
    SHA256_Context soft_ctx = sha256_begin();
    detail::sha256_update_soft(&soft_ctx, data.data(), data.size());
    SHA256 soft_full = sha256_end(&soft_ctx);

    // 分段计算 - 软件实现
    {
        SHA256_Context ctx = sha256_begin();
        size_t pos = 0;
        while (pos < data.size())
        {
            size_t chunk = (pos + 100 <= data.size()) ? 100 : (data.size() - pos);
            detail::sha256_update_soft(&ctx, data.data() + pos, chunk);
            pos += chunk;
        }
        SHA256 result = sha256_end(&ctx);
        EXPECT_EQ(result, soft_full);
    }

    // 分段计算 - 通用接口
    {
        SHA256_Context ctx = sha256_begin();
        size_t pos = 0;
        while (pos < data.size())
        {
            size_t chunk = (pos + 50 <= data.size()) ? 50 : (data.size() - pos);
            sha256_update(&ctx, data.data() + pos, chunk);
            pos += chunk;
        }
        SHA256 result = sha256_end(&ctx);
        EXPECT_EQ(result, soft_full);
    }

#if CKS_ARCH_X86
    // 分段计算 - SHA-NI
    {
        SHA256_Context ctx = sha256_begin();
        size_t pos = 0;
        while (pos < data.size())
        {
            size_t chunk = (pos + 75 <= data.size()) ? 75 : (data.size() - pos);
            detail::sha256_update_sha(&ctx, data.data() + pos, chunk);
            pos += chunk;
        }
        SHA256 result = sha256_end(&ctx);
        EXPECT_EQ(result, soft_full);
    }
#endif

#if CKS_ARCH_ARM
    // 分段计算 - ARM
    {
        SHA256_Context ctx = sha256_begin();
        size_t pos = 0;
        while (pos < data.size())
        {
            size_t chunk = (pos + 60 <= data.size()) ? 60 : (data.size() - pos);
            detail::sha256_update_arm(&ctx, data.data() + pos, chunk);
            pos += chunk;
        }
        SHA256 result = sha256_end(&ctx);
        EXPECT_EQ(result, soft_full);
    }
#endif
}

// 6. 大Buffer压力测试（1-10MB）
TEST(SHA256, LargeBufferStress)
{
    // 1MB数据
    std::vector<uint8_t> data(1024 * 1024);
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] = static_cast<uint8_t>(((i * 13) ^ 0x5A) & 0xFF);
    }

    // 软件实现（基准）
    SHA256_Context soft_ctx = sha256_begin();
    detail::sha256_update_soft(&soft_ctx, data.data(), data.size());
    SHA256 soft_result = sha256_end(&soft_ctx);

    // 通用接口
    SHA256_Context ctx = sha256_begin();
    sha256_update(&ctx, data.data(), data.size());
    SHA256 result = sha256_end(&ctx);

    EXPECT_EQ(result, soft_result);

#if CKS_ARCH_X86
    {
        SHA256_Context shani_ctx = sha256_begin();
        detail::sha256_update_sha(&shani_ctx, data.data(), data.size());
        SHA256 shani_result = sha256_end(&shani_ctx);
        EXPECT_EQ(shani_result, soft_result);
    }
#endif

#if CKS_ARCH_ARM
    {
        SHA256_Context arm_ctx = sha256_begin();
        detail::sha256_update_arm(&arm_ctx, data.data(), data.size());
        SHA256 arm_result = sha256_end(&arm_ctx);
        EXPECT_EQ(arm_result, soft_result);
    }
#endif
}

// 7. 百万A测试（NIST标准）
// 计算100万个'a'字符的SHA256值
TEST(SHA256, MillionA)
{
    // NIST标准值：1000000个'a'的SHA256
    const char* expected_hex = "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0";

    const size_t count = 1000000;

    // 由于数据量大，分段处理
    SHA256_Context ctx = sha256_begin();

    // 分10000次，每次100字节
    uint8_t chunk[100];
    std::memset(chunk, 'a', sizeof(chunk));

    for (size_t i = 0; i < count / sizeof(chunk); ++i)
    {
        sha256_update(&ctx, chunk, sizeof(chunk));
    }

    SHA256 result = sha256_end(&ctx);
    EXPECT_EQ(to_hex(result), expected_hex);

    // 软件实现验证
    SHA256_Context soft_ctx = sha256_begin();
    for (size_t i = 0; i < count / sizeof(chunk); ++i)
    {
        detail::sha256_update_soft(&soft_ctx, chunk, sizeof(chunk));
    }
    SHA256 soft_result = sha256_end(&soft_ctx);
    EXPECT_EQ(to_hex(soft_result), expected_hex);
    EXPECT_EQ(result, soft_result);
}

// 8. 块边界测试（55-65字节，跨越填充边界）
TEST(SHA256, BlockBoundary)
{
    // SHA256块大小为64字节
    // 55字节 + 1字节(0x80) + 8字节长度 = 64字节（正好一块）
    // 56字节需要两块

    for (size_t len = 55; len <= 65; ++len)
    {
        std::vector<uint8_t> data(len);
        for (size_t i = 0; i < len; ++i)
        {
            data[i] = static_cast<uint8_t>(i & 0xFF);
        }

        SHA256_Context soft_ctx = sha256_begin();
        detail::sha256_update_soft(&soft_ctx, data.data(), data.size());
        SHA256 soft_result = sha256_end(&soft_ctx);

        SHA256_Context ctx = sha256_begin();
        sha256_update(&ctx, data.data(), data.size());
        SHA256 result = sha256_end(&ctx);

        EXPECT_EQ(result, soft_result) << "Failed for length: " << len;
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
