#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include <string>
#include <utility> // std::pair
#include "checksum/crc32c.hpp"

using namespace cks;

// 6. 已知值测试
// https://www.lddgo.net/encrypt/crc 在线计算
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



// 1. 空 Buffer 测试：验证 size=0 时各后端的处理
TEST(CRC32C, EmptyBuffer)
{
    // Software Baseline
    uint32_t soft_val = crc32c_begin();
    soft_val = detail::crc32c_update_soft(soft_val, nullptr, 0);
    uint32_t soft_final = crc32c_end(soft_val);
    EXPECT_EQ(soft_final, 0x00000000);

#if CKS_ARCH_X86
    // SSE4.2
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_sse42(crc, nullptr, 0);
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_final);
    }
#endif

#if CKS_ARCH_ARM
    // ARM CRC32
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_arm(crc, nullptr, 0);
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_final);
    }
#endif
}

// 2. 小 Buffer 与步长测试：验证 11 字节（跨越 8-byte 和剩余字节逻辑）
TEST(CRC32C, SmallBuffer)
{
    const uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31};
    const size_t len = sizeof(data);

    // Software Baseline
    uint32_t soft_val = crc32c_begin();
    soft_val = detail::crc32c_update_soft(soft_val, data, len);
    uint32_t soft_final = crc32c_end(soft_val);

#if CKS_ARCH_X86
    // SSE4.2
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_sse42(crc, data, len);
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_final);
    }
#endif

#if CKS_ARCH_ARM
    // ARM
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_arm(crc, data, len);
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_final);
    }
#endif
}

// 3. 非对齐内存测试
TEST(CRC32C, UnalignedMemory)
{
    alignas(4) const uint8_t aligned_data[] = {0xAA, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31};
    const uint8_t* data = aligned_data + 1;
    const size_t len = sizeof(data);

    // Software Baseline
    uint32_t soft_val = crc32c_begin();
    soft_val = detail::crc32c_update_soft(soft_val, data, len);
    uint32_t soft_final = crc32c_end(soft_val);

#if CKS_ARCH_X86
    // SSE4.2
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_sse42(crc, data, len);
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_final);
    }
#endif

#if CKS_ARCH_ARM
    // ARM
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_arm(crc, data, len);
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_final);
    }
#endif
}

// 4. 分段校验一致性测试：验证 update 过程的中间状态
TEST(CRC32C, SegmentConsistency)
{
    const uint8_t data[] = "ParallelConsistencyCheck";
    const size_t len = sizeof(data) - 1;
    const size_t mid = 8;

    // Software Baseline (一次性完成)
    uint32_t soft_full = crc32c_end(detail::crc32c_update_soft(crc32c_begin(), data, len));

#if CKS_ARCH_X86
    // SSE4.2 分两段计算
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_sse42(crc, data, mid);
        crc = detail::crc32c_update_sse42(crc, data + mid, len - mid);
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_full);
    }
#endif

#if CKS_ARCH_ARM
    // ARM 分两段计算
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_arm(crc, data, mid);
        crc = detail::crc32c_update_arm(crc, data + mid, len - mid);
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_full);
    }
#endif
}

// 5. 大规模数据测试：1MB 随机数据压力测试
TEST(CRC32C, LargeBufferStress)
{
    std::vector<uint8_t> data(1024 * 1024);
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<uint8_t>((i * 13) ^ 0x5A);
    }

    // Software Baseline
    uint32_t soft_val = crc32c_begin();
    soft_val = detail::crc32c_update_soft(soft_val, data.data(), data.size());
    uint32_t soft_final = crc32c_end(soft_val);

#if CKS_ARCH_X86
    // SSE4.2
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_sse42(crc, data.data(), data.size());
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_final);
    }
#endif

#if CKS_ARCH_ARM
    // ARM
    {
        uint32_t crc = crc32c_begin();
        crc = detail::crc32c_update_arm(crc, data.data(), data.size());
        uint32_t result = crc32c_end(crc);
        EXPECT_EQ(result, soft_final);
    }
#endif
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}