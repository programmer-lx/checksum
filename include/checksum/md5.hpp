#pragma once

#include <cstdint>
#include <cstddef>

#include "checksum/detail/base.hpp"

namespace cks
{
    // MD5计算上下文（内部状态）
    struct MD5_Context
    {
        uint32_t state[4];      // 4个32位哈希状态 (A, B, C, D)
        uint64_t bit_len;       // 总消息长度（位）
        uint8_t  buffer[64];    // 64字节输入缓冲区
        uint32_t buffer_len;    // 缓冲区中有效字节数
    };

    // MD5最终结果（16字节哈希值）
    struct MD5
    {
        uint8_t bytes[16];      // 128位哈希值，小端序存储

        bool operator==(const MD5& other) const noexcept
        {
            for (int i = 0; i < 16; ++i)
            {
                if (bytes[i] != other.bytes[i])
                    return false;
            }
            return true;
        }

        bool operator!=(const MD5& other) const noexcept
        {
            return !(*this == other);
        }
    };

    // 初始化MD5上下文
    CKS_FORCE_INLINE MD5_Context CKS_CALL_CONV md5_begin() noexcept
    {
        MD5_Context ctx;
        // 初始哈希值（RFC 1321规范）
        ctx.state[0] = 0x67452301;
        ctx.state[1] = 0xefcdab89;
        ctx.state[2] = 0x98badcfe;
        ctx.state[3] = 0x10325476;
        ctx.bit_len = 0;
        ctx.buffer_len = 0;
        return ctx;
    }

    // 更新哈希状态
    void CKS_CALL_CONV md5_update(MD5_Context* ctx, const void* data, size_t len) noexcept;

    // 最终化处理，返回16字节哈希值
    MD5 CKS_CALL_CONV md5_end(MD5_Context* ctx) noexcept;
}
