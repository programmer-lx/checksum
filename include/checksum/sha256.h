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
        uint32_t state[8];      /* 8个32位哈希状态 (H0-H7) */
        uint64_t bit_len;       /* 总消息长度（位） */
        uint8_t buffer[64];     /* 64字节输入缓冲区 */
        uint32_t buffer_len;    /* 缓冲区中有效字节数 */
    } cks_SHA256_Context;

    typedef struct
    {
        uint8_t bytes[32];      /* 256位哈希值，大端序存储 */
    } cks_SHA256;

    /* 初始化SHA256上下文 */
    CKS_API cks_SHA256_Context cks_sha256_begin(void);

    /* 更新哈希状态（处理数据块） */
    CKS_API void cks_sha256_update(cks_SHA256_Context* ctx, const void* data, size_t len);

    /* 最终化处理，返回32字节哈希值 */
    CKS_API cks_SHA256 cks_sha256_end(cks_SHA256_Context* ctx);


    static inline int cks_sha256_equal(const cks_SHA256* a, const cks_SHA256* b)
    {
        return memcmp(a->bytes, b->bytes, 32) == 0;
    }

    /* 软件实现 */
    void cks_impl_sha256_update_soft(cks_SHA256_Context* ctx, const void* data, size_t len);
    cks_SHA256 cks_impl_sha256_end_soft(cks_SHA256_Context* ctx);

/* x86 SHA-NI硬件实现 */
#if CKS_ARCH_X86
    void cks_impl_sha256_update_sha(cks_SHA256_Context* ctx, const void* data, size_t len);
    cks_SHA256 cks_impl_sha256_end_sha(cks_SHA256_Context* ctx);
#endif

/* ARM SHA2硬件实现 */
#if CKS_ARCH_ARM
    void cks_impl_sha256_update_arm(cks_SHA256_Context* ctx, const void* data, size_t len);
    cks_SHA256 cks_impl_sha256_end_arm(cks_SHA256_Context* ctx);
#endif

#ifdef __cplusplus
}
#endif
