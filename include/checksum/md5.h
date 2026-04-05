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
        uint32_t state[4];      /* 4个32位哈希状态 (A, B, C, D) */
        uint64_t bit_len;       /* 总消息长度（位） */
        uint8_t buffer[64];     /* 64字节输入缓冲区 */
        uint32_t buffer_len;    /* 缓冲区中有效字节数 */
    } cks_MD5_Context;

    typedef struct
    {
        uint8_t bytes[16];      /* 128位哈希值，小端序存储 */
    } cks_MD5;

    /* 初始化MD5上下文 */
    CKS_API cks_MD5_Context cks_md5_begin(void);

    /* 更新哈希状态 */
    CKS_API void cks_md5_update(cks_MD5_Context* ctx, const void* data, size_t len);

    /* 最终化处理，返回16字节哈希值 */
    CKS_API cks_MD5 cks_md5_end(cks_MD5_Context* ctx);

    
    static inline int cks_md5_equal(const cks_MD5* a, const cks_MD5* b)
    {
        return memcmp(a->bytes, b->bytes, 16) == 0;
    }

#ifdef __cplusplus
}
#endif
