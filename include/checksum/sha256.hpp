#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring> // memcmp

#include "checksum/detail/base.hpp"

namespace cks
{
    // SHA256计算上下文（内部状态）
    struct SHA256_Context
    {
        uint32_t state[8];      // 8个32位哈希状态 (H0-H7)
        uint64_t bit_len;       // 总消息长度（位）
        uint8_t  buffer[64];    // 64字节输入缓冲区
        uint32_t buffer_len;    // 缓冲区中有效字节数
    };

    // SHA256最终结果（32字节哈希值）
    struct SHA256
    {
        uint8_t bytes[32];      // 256位哈希值，大端序存储

        bool operator==(const SHA256& other) const noexcept
        {
            return std::memcmp(bytes, other.bytes, sizeof(bytes)) == 0;
        }

        bool operator!=(const SHA256& other) const noexcept
        {
            return std::memcmp(bytes, other.bytes, sizeof(bytes)) != 0;
        }
    };

    // 初始化SHA256上下文
    CKS_FORCE_INLINE SHA256_Context CKS_CALL_CONV sha256_begin() noexcept
    {
        SHA256_Context ctx;
        // 初始哈希值H(0)（FIPS 180-4规范）
        ctx.state[0] = 0x6a09e667;
        ctx.state[1] = 0xbb67ae85;
        ctx.state[2] = 0x3c6ef372;
        ctx.state[3] = 0xa54ff53a;
        ctx.state[4] = 0x510e527f;
        ctx.state[5] = 0x9b05688c;
        ctx.state[6] = 0x1f83d9ab;
        ctx.state[7] = 0x5be0cd19;
        ctx.bit_len = 0;
        ctx.buffer_len = 0;
        return ctx;
    }

    namespace detail
    {
        // 软件实现
        void CKS_CALL_CONV sha256_update_soft(SHA256_Context* ctx, const void* data, size_t len) noexcept;
        SHA256 CKS_CALL_CONV sha256_end_soft(SHA256_Context* ctx) noexcept;

        // x86 SHA-NI硬件实现
        #if CKS_ARCH_X86
        CKS_FUNC_ATTR_INTRINSICS_SHA256
        void CKS_CALL_CONV sha256_update_sha(SHA256_Context* ctx, const void* data, size_t len) noexcept;

        CKS_FUNC_ATTR_INTRINSICS_SHA256
        SHA256 CKS_CALL_CONV sha256_end_sha(SHA256_Context* ctx) noexcept;
        #endif

        // ARM SHA2硬件实现
        #if CKS_ARCH_ARM
        CKS_FUNC_ATTR_INTRINSICS_ARM_SHA256
        void CKS_CALL_CONV sha256_update_arm(SHA256_Context* ctx, const void* data, size_t len) noexcept;

        CKS_FUNC_ATTR_INTRINSICS_ARM_SHA256
        SHA256 CKS_CALL_CONV sha256_end_arm(SHA256_Context* ctx) noexcept;
        #endif
    }

    // 更新哈希状态（处理数据块）
    void CKS_CALL_CONV sha256_update(SHA256_Context* ctx, const void* data, size_t len) noexcept;

    // 最终化处理，返回32字节哈希值
    SHA256 CKS_CALL_CONV sha256_end(SHA256_Context* ctx) noexcept;
}
