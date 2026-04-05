#include "checksum/crc32c.h"
#include "checksum/md5.h"
#include "checksum/sha256.h"
#include "checksum/xxhash3_128.h"
#include "checksum/xxhash3_64.h"

// 测试是否编译成了DLL
// BUILD_DLL宏只在checksum库内部可见
#if !(!defined(CKS_BUILD_STATIC) && !defined(CKS_BUILD_DLL))
    #error should be compiled as shared library
#endif

#pragma message("API macro = " CKS_STR(CKS_API))

void crc32c()
{
    cks_CRC32C crc = cks_crc32c_begin();
    crc = cks_crc32c_update(crc, nullptr, 0);
    crc = cks_crc32c_end(crc);

    [[maybe_unused]] volatile void* ptr = &crc;

    {
        cks_CRC32C value2;
        memset(&value2, 0, sizeof(value2));
        int test = cks_crc32c_equal(&crc, &value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
}

void md5()
{
    cks_MD5_Context md5 = cks_md5_begin();
    cks_md5_update(&md5, nullptr, 0);
    cks_MD5 value = cks_md5_end(&md5);

    [[maybe_unused]] volatile void* ptr = &value;

    {
        cks_MD5 value2;
        memset(&value2, 0, sizeof(value2));
        int test = cks_md5_equal(&value, &value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
}

void sha256()
{
    cks_SHA256_Context ctx = cks_sha256_begin();
    cks_sha256_update(&ctx, nullptr, 0);
    cks_SHA256 value = cks_sha256_end(&ctx);

    [[maybe_unused]] volatile void* ptr = &value;

    {
        cks_SHA256 value2;
        memset(&value2, 0, sizeof(value2));
        int test = cks_sha256_equal(&value, &value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
}

void xxhash3_128()
{
    cks_xxHash3_128_Context ctx = cks_xxhash3_128_begin();
    cks_xxhash3_128_update(&ctx, nullptr, 0);
    cks_xxHash3_128 value = cks_xxhash3_128_end(&ctx);

    [[maybe_unused]] volatile void* ptr = &value;

    {
        cks_xxHash3_128 value2;
        memset(&value2, 0, sizeof(value2));
        int test = cks_xxhash3_128_equal(&value, &value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
}

void xxhash3_64()
{
    cks_xxHash3_64_Context ctx = cks_xxhash3_64_begin();
    cks_xxhash3_64_update(&ctx, nullptr, 0);
    cks_xxHash3_64 value = cks_xxhash3_64_end(&ctx);

    [[maybe_unused]] volatile void* ptr = &value;

    {
        cks_xxHash3_64 value2;
        memset(&value2, 0, sizeof(value2));
        int test = cks_xxhash3_64_equal(&value, &value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
}


int main()
{
    crc32c();
    md5();
    sha256();
    xxhash3_128();
    xxhash3_64();
    return 0;
}
