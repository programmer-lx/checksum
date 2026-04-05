#include <gtest/gtest.h>
#include <string>
#include <cstring>
#include "checksum/crc32c.h"
#include "checksum/md5.h"
#include "checksum/sha256.h"
#include "checksum/xxhash3_128.h"
#include "checksum/xxhash3_64.h"
#include "checksum/checksum_str.h"


TEST(str, CRC32C)
{
    std::string data = "123456789";

    cks_CRC32C res = cks_crc32c_begin();
    res = cks_crc32c_update(res, data.data(), data.size());
    res = cks_crc32c_end(res);

    char str[CKS_HEX_CHAR_COUNT_CRC32C + 1] = {};
    cks_to_string(str, &res, sizeof(cks_CRC32C));

    char expected_str[CKS_HEX_CHAR_COUNT_CRC32C + 1] = "e3069283";
    EXPECT_TRUE(std::memcmp(str, expected_str, CKS_HEX_CHAR_COUNT_CRC32C + 1));
}

TEST(str, MD5)
{
    std::string data = "abcdefghijklmnopqrstuvwxyz";

    cks_MD5_Context ctx = cks_md5_begin();
    cks_md5_update(&ctx, data.data(), data.size());
    cks_MD5 res = cks_md5_end(&ctx);

    char str[CKS_HEX_CHAR_COUNT_MD5 + 1] = {};
    cks_to_string(str, &res, sizeof(cks_MD5));

    char expected_str[CKS_HEX_CHAR_COUNT_MD5 + 1] = "c3fcd3d76192e4007dfb496cca67e13b";
    EXPECT_TRUE(std::memcmp(str, expected_str, CKS_HEX_CHAR_COUNT_MD5 + 1));
}

TEST(str, SHA256)
{
    std::string data = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

    cks_SHA256_Context ctx = cks_sha256_begin();
    cks_sha256_update(&ctx, data.data(), data.size());
    cks_SHA256 res = cks_sha256_end(&ctx);

    char str[CKS_HEX_CHAR_COUNT_SHA256 + 1] = {};
    cks_to_string(str, &res, sizeof(cks_SHA256));

    char expected_str[CKS_HEX_CHAR_COUNT_SHA256 + 1] = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
    EXPECT_TRUE(std::memcmp(str, expected_str, CKS_HEX_CHAR_COUNT_SHA256 + 1));
}

TEST(str, xxHash3_64)
{
    std::string data = "abcdefghijklmnopqrstuvwxyz";

    cks_xxHash3_64_Context ctx = cks_xxhash3_64_begin();
    cks_xxhash3_64_update(&ctx, data.data(), data.size());
    cks_xxHash3_64 res = cks_xxhash3_64_end(&ctx);

    char str[CKS_HEX_CHAR_COUNT_XXHASH3_64 + 1] = {};
    cks_to_string(str, &res, sizeof(cks_xxHash3_64));

    char expected_str[CKS_HEX_CHAR_COUNT_XXHASH3_64 + 1] = "810f9ca067fbb90c";
    EXPECT_TRUE(std::memcmp(str, expected_str, CKS_HEX_CHAR_COUNT_XXHASH3_64 + 1));
}

TEST(str, xxHash3_128)
{
    std::string data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    cks_xxHash3_128_Context ctx = cks_xxhash3_128_begin();
    cks_xxhash3_128_update(&ctx, data.data(), data.size());
    cks_xxHash3_128 res = cks_xxhash3_128_end(&ctx);

    char str[CKS_HEX_CHAR_COUNT_XXHASH3_128 + 1] = {};
    cks_to_string(str, &res, sizeof(cks_xxHash3_128));

    char expected_str[CKS_HEX_CHAR_COUNT_XXHASH3_128 + 1] = "5bcb80b619500686a3c0560bd47a4ffb";
    EXPECT_TRUE(std::memcmp(str, expected_str, CKS_HEX_CHAR_COUNT_XXHASH3_128 + 1));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
