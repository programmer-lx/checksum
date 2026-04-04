#include <gtest/gtest.h>
#include <vector>
#include <cstring>
#include <string>
#include <utility>
#include "checksum/md5.hpp"

using namespace cks;

// 辅助函数：将MD5结果转换为hex字符串
static std::string to_hex(const MD5& hash)
{
    const char hex_chars[] = "0123456789abcdef";
    std::string result;
    result.reserve(32);
    for (int i = 0; i < 16; ++i)
    {
        result += hex_chars[(hash.bytes[i] >> 4) & 0xF];
        result += hex_chars[hash.bytes[i] & 0xF];
    }
    return result;
}

// 1. 已知值测试（RFC 1321标准向量）
TEST(MD5, Standard)
{
    // 测试向量: {输入, 期望的hex结果}
    std::pair<std::string, std::string> test_vectors[] = {
        {"", "d41d8cd98f00b204e9800998ecf8427e"},

        {"The quick brown fox jumps over the lazy dog", "9e107d9d372bb6826bd81d3542a419d6"},

        {"The quick brown fox jumps over the lazy cog", "1055d3e698d289f2af8663725127bd4b"},

        {"abc", "900150983cd24fb0d6963f7d28e17f72"},

        {"abcdoiuh aoiuehf ioashoiufhaosiue fhaioush fisdnoicaosinsidiksdish iufas hiufh aiuwshfiuahefiuaho ufho uhas ifs",
            "86a6c3e60070e7d7990883af0934d5df"}
    };

    for (const auto& vec : test_vectors)
    {
        MD5_Context ctx = md5_begin();
        md5_update(&ctx, vec.first.data(), vec.first.size());
        MD5 result = md5_end(&ctx);

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
    MD5_Context soft_ctx = md5_begin();
    md5_update(&soft_ctx, data.data(), data.size());
    MD5 soft_full = md5_end(&soft_ctx);

    // 分段计算
    {
        MD5_Context ctx = md5_begin();
        size_t pos = 0;
        while (pos < data.size())
        {
            size_t chunk = (pos + 100 <= data.size()) ? 100 : (data.size() - pos);
            md5_update(&ctx, data.data() + pos, chunk);
            pos += chunk;
        }
        MD5 result = md5_end(&ctx);
        EXPECT_EQ(result, soft_full);
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
