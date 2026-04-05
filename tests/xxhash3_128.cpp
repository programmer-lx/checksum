#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include <string>
#include <utility>
#include "checksum/xxhash3_128.h"
#include "utils.hpp"

// 1. 已知值测试
// https://www.codertools.net/tools/xxhash.php?lang=zh
TEST(xxHash3_128, Standard)
{
    // 测试向量: {输入, 期望的hex结果}
    // 使用seed=0的默认结果（xxHash v0.8.3 canonical格式）
    std::vector<std::pair<std::string, std::string>> test_vectors = {
        // "abc"
        { "abc", "06b05ab6733a618578af5f94892f3950" },

        // "abcdefghijklmnopqrstuvwxyz"
        { "abcdefghijklmnopqrstuvwxyz", "db7ca44e84843d67ebe162220154e1e6" },

        // "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "5bcb80b619500686a3c0560bd47a4ffb" },
    };

    int idx = 0;
    for (const auto& vec : test_vectors)
    {
        // 无seed版本
        {
            cks_xxHash3_128_Context ctx = cks_xxhash3_128_begin();
            cks_xxhash3_128_update(&ctx, vec.first.data(), vec.first.size());
            cks_xxHash3_128 result = cks_xxhash3_128_end(&ctx);
            EXPECT_EQ(to_hex(result), vec.second) << "[no seed] idx: " << idx;
        }

        ++idx;
    }
}

// 2. 带seed的已知值测试
TEST(xxHash3_128, WithSeed)
{
    // 使用seed=0x123456789ABCDEF的测试结果（xxHash v0.8.3 canonical格式）
    std::vector<std::pair<std::string, std::string>> test_vectors = {
        // "abc"
        { "abc", "521f5ca64895f66c4a2ec311f0e180a4" },

        // "abcdefghijklmnopqrstuvwxyz"
        { "abcdefghijklmnopqrstuvwxyz", "f4fc8a5c35791028daad8aeb0b9ef50b" },

        // "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "ff6880d137b287a0430c1b61d8d7f8ec" }
    };

    const uint64_t seed = 0x123456789ABCDEFULL;

    int idx = 0;
    for (const auto& vec : test_vectors)
    {
        cks_xxHash3_128_Context ctx = cks_xxhash3_128_begin_with_seed(seed);
        cks_xxhash3_128_update(&ctx, vec.first.data(), vec.first.size());
        cks_xxHash3_128 result = cks_xxhash3_128_end(&ctx);
        EXPECT_EQ(to_hex(result), vec.second) << "[with seed] idx: " << idx;
        ++idx;
    }
}

// 3. 空Buffer测试
TEST(xxHash3_128, EmptyBuffer)
{
    // 空字符串的xxHash3-128（默认seed=0）
    const char* expected_hex = "99aa06d3014798d86001c324468d497f";

    cks_xxHash3_128_Context ctx = cks_xxhash3_128_begin();
    cks_xxHash3_128 result = cks_xxhash3_128_end(&ctx);

    EXPECT_EQ(to_hex(result), expected_hex);

    // 显式传入nullptr和0
    ctx = cks_xxhash3_128_begin();
    cks_xxhash3_128_update(&ctx, nullptr, 0);
    result = cks_xxhash3_128_end(&ctx);

    EXPECT_EQ(to_hex(result), expected_hex);
}

// 5. 非对齐内存测试
TEST(xxHash3_128, UnalignedMemory)
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

        cks_xxHash3_128_Context ctx = cks_xxhash3_128_begin();
        cks_xxhash3_128_update(&ctx, unaligned_data, len);
        cks_xxHash3_128 result = cks_xxhash3_128_end(&ctx);

        // 验证结果不为零
        bool all_zero = true;
        for (int i = 0; i < 16; ++i)
        {
            if (result.bytes[i] != 0)
            {
                all_zero = false;
                break;
            }
        }
        EXPECT_FALSE(all_zero) << "Failed for offset: " << offset;
    }
}

// 6. 分段一致性测试（一次性 vs 分段）
TEST(xxHash3_128, SegmentConsistency)
{
    // 测试数据
    std::vector<uint8_t> data(1024);
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] = static_cast<uint8_t>((i * 11 + 23) & 0xFF);
    }

    // 一次性计算
    cks_xxHash3_128_Context ctx1 = cks_xxhash3_128_begin();
    cks_xxhash3_128_update(&ctx1, data.data(), data.size());
    cks_xxHash3_128 full_result = cks_xxhash3_128_end(&ctx1);

    // 分段计算
    cks_xxHash3_128_Context ctx2 = cks_xxhash3_128_begin();
    size_t pos = 0;
    while (pos < data.size())
    {
        size_t chunk = (pos + 100 <= data.size()) ? 100 : (data.size() - pos);
        cks_xxhash3_128_update(&ctx2, data.data() + pos, chunk);
        pos += chunk;
    }
    cks_xxHash3_128 seg_result = cks_xxhash3_128_end(&ctx2);

    EXPECT_TRUE(cks_xxhash3_128_equal(&full_result, &seg_result));
}

// 8. 不同seed的一致性测试
TEST(xxHash3_128, DifferentSeeds)
{
    const std::string test_data = "Hello, xxHash3-128!";

    // 使用不同seed应该产生不同结果
    std::vector<uint64_t> seeds = { 0, 1, 0x1234567890ABCDEFULL, 0xFFFFFFFFFFFFFFFFULL };
    std::vector<cks_xxHash3_128> results;

    for (uint64_t seed : seeds)
    {
        cks_xxHash3_128_Context ctx = cks_xxhash3_128_begin_with_seed(seed);
        cks_xxhash3_128_update(&ctx, test_data.data(), test_data.size());
        cks_xxHash3_128 result = cks_xxhash3_128_end(&ctx);
        results.push_back(result);
    }

    // 验证不同seed产生不同结果（至少前两个应该不同）
    EXPECT_FALSE(cks_xxhash3_128_equal(&results[0], &results[1]));
}

// 9. 边界测试（正好在stripe boundary上）
TEST(xxHash3_128, StripeBoundary)
{
    // xxHash3内部使用64字节的stripe
    // 测试正好在边界上的情况
    for (size_t len = 64; len <= 256; len += 64)
    {
        std::vector<uint8_t> data(len);
        for (size_t i = 0; i < len; ++i)
        {
            data[i] = static_cast<uint8_t>((i + 1) & 0xFF);
        }

        cks_xxHash3_128_Context ctx = cks_xxhash3_128_begin();
        cks_xxhash3_128_update(&ctx, data.data(), data.size());
        cks_xxHash3_128 result = cks_xxhash3_128_end(&ctx);

        // 验证结果不为零
        bool all_zero = true;
        for (int i = 0; i < 16; ++i)
        {
            if (result.bytes[i] != 0)
            {
                all_zero = false;
                break;
            }
        }
        EXPECT_FALSE(all_zero) << "Failed for length: " << len;
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
