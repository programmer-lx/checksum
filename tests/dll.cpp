#include "checksum/crc32c.hpp"
#include "checksum/md5.hpp"
#include "checksum/sha256.hpp"

// 测试是否编译成了DLL
// BUILD_DLL宏只在checksum库内部可见
#if !(!defined(CKS_BUILD_STATIC) && !defined(CKS_BUILD_DLL))
    #error should be compiled as shared library
#endif

#pragma message("API macro = " CKS_STR(CKS_API))

using namespace cks;

void crc32c()
{
    CRC32C crc = crc32c_begin();
    crc = crc32c_update(crc, nullptr, 0);
    crc = crc32c_end(crc);

    [[maybe_unused]] volatile void* ptr = &crc;
}

void md5()
{
    MD5_Context md5 = md5_begin();
    md5_update(&md5, nullptr, 0);
    MD5 value = md5_end(&md5);

    [[maybe_unused]] volatile void* ptr = &value;

    {
        MD5 value2{};
        bool test = (value == value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
    {
        MD5 value2{};
        bool test = (value != value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
}

void sha256()
{
    SHA256_Context ctx = sha256_begin();
    sha256_update(&ctx, nullptr, 0);
    SHA256 value = sha256_end(&ctx);

    [[maybe_unused]] volatile void* ptr = &value;

    {
        SHA256 value2{};
        bool test = (value == value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
    {
        SHA256 value2{};
        bool test = (value != value2);
        [[maybe_unused]] volatile void* ptr2 = &test;
    }
}


int main()
{
    crc32c();
    md5();
    sha256();
    return 0;
}
