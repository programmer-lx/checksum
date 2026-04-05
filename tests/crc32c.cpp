#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include <string>
#include <utility>
#include "checksum/crc32c.h"

// 已知值测试
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

        // soft
        cks_CRC32C soft_res = cks_crc32c_end(cks_impl_crc32c_update_soft(cks_crc32c_begin(), data, len));
        cks_CRC32C expected;
        expected.bytes = values[i].second;

        EXPECT_TRUE(cks_crc32c_equal(&soft_res, &expected));

        // 通用接口
        {
            cks_CRC32C res = cks_crc32c_begin();
            res = cks_crc32c_update(res, data, len);
            res = cks_crc32c_end(res);
            
            cks_CRC32C expected2;
            expected2.bytes = values[i].second;
            EXPECT_TRUE(cks_crc32c_equal(&res, &expected2));
        }

        #if CKS_ARCH_X86
        cks_CRC32C sse_res = cks_crc32c_end(cks_impl_crc32c_update_sse42(cks_crc32c_begin(), data, len));
        EXPECT_TRUE(cks_crc32c_equal(&sse_res, &expected));
        #endif

        #if CKS_ARCH_ARM
        cks_CRC32C arm_res = cks_crc32c_end(cks_impl_crc32c_update_arm(cks_crc32c_begin(), data, len));
        EXPECT_TRUE(cks_crc32c_equal(&arm_res, &expected));
        #endif
    }
}



// 1. 空 Buffer 测试：验证 size=0 时各后端的处理
TEST(CRC32C, EmptyBuffer)
{
    // Software Baseline
    cks_CRC32C soft_val = cks_crc32c_begin();
    soft_val = cks_impl_crc32c_update_soft(soft_val, nullptr, 0);
    cks_CRC32C soft_final = cks_crc32c_end(soft_val);

    cks_CRC32C expected;
    expected.bytes = 0x00000000;
    EXPECT_TRUE(cks_crc32c_equal(&soft_final, &expected));

#if CKS_ARCH_X86
    // SSE4.2
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_sse42(crc, nullptr, 0);
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_final));
    }
#endif

#if CKS_ARCH_ARM
    // ARM CRC32
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_arm(crc, nullptr, 0);
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_final));
    }
#endif
}

// 2. 小 Buffer 与步长测试：验证 11 字节（跨越 8-byte 和剩余字节逻辑）
TEST(CRC32C, SmallBuffer)
{
    const uint8_t data[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31};
    const size_t len = sizeof(data);

    // Software Baseline
    cks_CRC32C soft_val = cks_crc32c_begin();
    soft_val = cks_impl_crc32c_update_soft(soft_val, data, len);
    cks_CRC32C soft_final = cks_crc32c_end(soft_val);

#if CKS_ARCH_X86
    // SSE4.2
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_sse42(crc, data, len);
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_final));
    }
#endif

#if CKS_ARCH_ARM
    // ARM
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_arm(crc, data, len);
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_final));
    }
#endif
}

// 3. 非对齐内存测试
TEST(CRC32C, UnalignedMemory)
{
    alignas(4) const uint8_t aligned_data[] = {0xAA, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x31};
    const uint8_t* data = aligned_data + 1;
    const size_t len = sizeof(aligned_data) - 1;

    // Software Baseline
    cks_CRC32C soft_val = cks_crc32c_begin();
    soft_val = cks_impl_crc32c_update_soft(soft_val, data, len);
    cks_CRC32C soft_final = cks_crc32c_end(soft_val);

#if CKS_ARCH_X86
    // SSE4.2
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_sse42(crc, data, len);
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_final));
    }
#endif

#if CKS_ARCH_ARM
    // ARM
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_arm(crc, data, len);
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_final));
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
    cks_CRC32C soft_full = cks_crc32c_end(cks_impl_crc32c_update_soft(cks_crc32c_begin(), data, len));

#if CKS_ARCH_X86
    // SSE4.2 分两段计算
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_sse42(crc, data, mid);
        crc = cks_impl_crc32c_update_sse42(crc, data + mid, len - mid);
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_full));
    }
#endif

#if CKS_ARCH_ARM
    // ARM 分两段计算
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_arm(crc, data, mid);
        crc = cks_impl_crc32c_update_arm(crc, data + mid, len - mid);
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_full));
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
    cks_CRC32C soft_val = cks_crc32c_begin();
    soft_val = cks_impl_crc32c_update_soft(soft_val, data.data(), data.size());
    cks_CRC32C soft_final = cks_crc32c_end(soft_val);

#if CKS_ARCH_X86
    // SSE4.2
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_sse42(crc, data.data(), data.size());
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_final));
    }
#endif

#if CKS_ARCH_ARM
    // ARM
    {
        cks_CRC32C crc = cks_crc32c_begin();
        crc = cks_impl_crc32c_update_arm(crc, data.data(), data.size());
        cks_CRC32C result = cks_crc32c_end(crc);
        EXPECT_TRUE(cks_crc32c_equal(&result, &soft_final));
    }
#endif
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
