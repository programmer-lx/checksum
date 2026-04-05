#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include <string>
#include <utility>
#include "checksum/md5.h"
#include "utils.hpp"


// 1. 已知值测试（RFC 1321标准向量）
// https://www.rfc-editor.org/rfc/rfc1321
/*
MD5 test suite:
MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") = d174ab98d277d9f5a5611c2c9f419d9f
MD5 ("12345678901234567890123456789012345678901234567890123456789012345678901234567890") = 57edf4a22be3c955ac49da2e2107b67a
*/
TEST(MD5, Standard)
{
    // 测试向量: {输入, 期望的hex结果}
    std::pair<std::string, std::string> test_vectors[] = {
        {"", "d41d8cd98f00b204e9800998ecf8427e"},

        {"a", "0cc175b9c0f1b6a831c399e269772661"},

        {"message digest", "f96b697d7cb7938d525a2f31aaf161d0"},

        {"abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b"},

        {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", "d174ab98d277d9f5a5611c2c9f419d9f"},

        {"12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a"},

        {"The quick brown fox jumps over the lazy dog", "9e107d9d372bb6826bd81d3542a419d6"},

        {"The quick brown fox jumps over the lazy cog", "1055d3e698d289f2af8663725127bd4b"},

        {"abc", "900150983cd24fb0d6963f7d28e17f72"}
    };

    for (const auto& vec : test_vectors)
    {
        cks_MD5_Context ctx = cks_md5_begin();
        cks_md5_update(&ctx, vec.first.data(), vec.first.size());
        cks_MD5 result = cks_md5_end(&ctx);

        EXPECT_EQ(to_hex(result), vec.second);
    }
}

// 5. 分段一致性测试（一次性 vs 分段）
TEST(MD5, SegmentConsistency)
{
    // 测试数据
    std::vector<uint8_t> data(1024);
    for (size_t i = 0; i < data.size(); ++i)
    {
        data[i] = static_cast<uint8_t>((i * 11 + 23) & 0xFF);
    }

    // 一次性计算
    cks_MD5_Context soft_ctx = cks_md5_begin();
    cks_md5_update(&soft_ctx, data.data(), data.size());
    cks_MD5 soft_full = cks_md5_end(&soft_ctx);

    // 分段计算
    {
        cks_MD5_Context ctx = cks_md5_begin();
        size_t pos = 0;
        while (pos < data.size())
        {
            size_t chunk = (pos + 100 <= data.size()) ? 100 : (data.size() - pos);
            cks_md5_update(&ctx, data.data() + pos, chunk);
            pos += chunk;
        }
        cks_MD5 result = cks_md5_end(&ctx);
        EXPECT_TRUE(cks_md5_equal(&result, &soft_full));
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
