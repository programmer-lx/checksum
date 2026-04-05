#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "checksum/detail/base.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        void* internal_state; /* XXH3_state_t* 指针 */
    } cks_xxHash3_128_Context;

    typedef struct
    {
        uint8_t bytes[16]; /* 128位哈希值，大端序 */
    } cks_xxHash3_128;

    /* 初始化（无seed） */
    CKS_API cks_xxHash3_128_Context cks_xxhash3_128_begin(void);

    /* 初始化（带64位seed） */
    CKS_API cks_xxHash3_128_Context cks_xxhash3_128_begin_with_seed(uint64_t seed);

    /* 更新数据 */
    CKS_API void cks_xxhash3_128_update(cks_xxHash3_128_Context* ctx, const void* data, size_t len);

    /* 结束并获取哈希 */
    CKS_API cks_xxHash3_128 cks_xxhash3_128_end(cks_xxHash3_128_Context* ctx);

    /* 比较哈希值 */
    static inline int cks_xxhash3_128_equal(const cks_xxHash3_128* a, const cks_xxHash3_128* b)
    {
        return memcmp(a->bytes, b->bytes, 16) == 0;
    }

#ifdef __cplusplus
}
#endif
