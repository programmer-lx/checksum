#include "checksum/crc32c.hpp"

// 测试是否编译成了静态库
#if !(defined(CKS_BUILD_STATIC) && !defined(CKS_BUILD_DLL))
    #error should be compiled as static library
#endif

int main()
{
    return 0;
}
