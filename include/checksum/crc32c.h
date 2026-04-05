#pragma once

#include <stdint.h>
#include <stddef.h>

#include "checksum/detail/base.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        uint32_t bytes;     // 32bit 大端序存储
    } cks_CRC32C;

    /* begin */
    CKS_API cks_CRC32C cks_crc32c_begin(void);

    /* update */
    CKS_API cks_CRC32C cks_crc32c_update(cks_CRC32C crc, const void* data, size_t len);

    /* end */
    CKS_API cks_CRC32C cks_crc32c_end(cks_CRC32C crc);

    
    static inline int cks_crc32c_equal(const cks_CRC32C* a, const cks_CRC32C* b)
    {
        return a->bytes == b->bytes;
    }

    /* 软件实现 */
    cks_CRC32C cks_impl_crc32c_update_soft(cks_CRC32C crc, const void* data, size_t size);

    /* x86 SSE4.2 */
#if CKS_ARCH_X86
    cks_CRC32C cks_impl_crc32c_update_sse42(cks_CRC32C crc, const void* data, size_t size);
#endif

    /* ARM CRC32 */
#if CKS_ARCH_ARM
    cks_CRC32C cks_impl_crc32c_update_arm(cks_CRC32C crc, const void* data, size_t size);
#endif

#ifdef __cplusplus
}
#endif
