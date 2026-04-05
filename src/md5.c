#include "checksum/md5.h"

#include <string.h>

#if CKS_ARCH_X86
    #include <immintrin.h>
#endif

#if CKS_ARCH_ARM
    #include <arm_neon.h>
#endif

#include "checksum/detail/cpu.h"

/* MD5常量K[64]（RFC 1321规范） */
/* clang-format off */
static const uint32_t MD5_K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

/* 每轮循环左移位数 */
static const uint32_t MD5_S[64] = {
    7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,
    5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,
    4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,
    6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21
};
/* clang-format on */

/* 循环左移 */
#define ROL(x, n) (((x) << (n)) | ((x) >> (32 - (n))))

/* MD5非线性函数 */
#define F(x, y, z) (((x) & (y)) | (~(x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & ~(z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | ~(z)))

/* MD5压缩函数（处理单个512位块） */
static void md5_transform(uint32_t state[4], const uint8_t block[64])
{
    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t x[16];

    /* 将字节转换为32位字（小端序） */
    for (int i = 0; i < 16; ++i)
    {
        x[i] = (uint32_t)(block[i * 4 + 0]) | ((uint32_t)(block[i * 4 + 1]) << 8) |
               ((uint32_t)(block[i * 4 + 2]) << 16) | ((uint32_t)(block[i * 4 + 3]) << 24);
    }

    /* 64轮运算 */
    for (int i = 0; i < 64; ++i)
    {
        uint32_t f, g;
        if (i < 16)
        {
            f = F(b, c, d);
            g = i;
        }
        else if (i < 32)
        {
            f = G(b, c, d);
            g = (5 * i + 1) & 15;
        }
        else if (i < 48)
        {
            f = H(b, c, d);
            g = (3 * i + 5) & 15;
        }
        else
        {
            f = I(b, c, d);
            g = (7 * i) & 15;
        }

        uint32_t temp = d;
        d = c;
        c = b;
        b = b + ROL(a + f + MD5_K[i] + x[g], MD5_S[i]);
        a = temp;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

cks_MD5_Context cks_md5_begin(void)
{
    cks_MD5_Context ctx;
    /* 初始哈希值（RFC 1321规范） */
    ctx.state[0] = 0x67452301;
    ctx.state[1] = 0xefcdab89;
    ctx.state[2] = 0x98badcfe;
    ctx.state[3] = 0x10325476;
    ctx.bit_len = 0;
    ctx.buffer_len = 0;
    return ctx;
}

void cks_md5_update(cks_MD5_Context* ctx, const void* data, size_t len)
{
    if (!data || len == 0)
        return;

    const uint8_t* bytes = (const uint8_t*)data;

    /* 更新总消息长度 */
    ctx->bit_len += (uint64_t)len * 8;

    /* 如果缓冲区有数据，尝试填满64字节块 */
    if (ctx->buffer_len > 0)
    {
        size_t needed = 64 - ctx->buffer_len;
        size_t copy_len = (len < needed) ? len : needed;

        memcpy(ctx->buffer + ctx->buffer_len, bytes, copy_len);
        ctx->buffer_len += (uint32_t)copy_len;
        bytes += copy_len;
        len -= copy_len;

        /* 如果缓冲区满了，处理它 */
        if (ctx->buffer_len == 64)
        {
            md5_transform(ctx->state, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }

    /* 处理完整的64字节块 */
    while (len >= 64)
    {
        md5_transform(ctx->state, bytes);
        bytes += 64;
        len -= 64;
    }

    /* 保存剩余数据到缓冲区 */
    if (len > 0)
    {
        memcpy(ctx->buffer, bytes, len);
        ctx->buffer_len = (uint32_t)len;
    }
}

cks_MD5 cks_md5_end(cks_MD5_Context* ctx)
{
    /* 1. 添加0x80标记 */
    ctx->buffer[ctx->buffer_len++] = 0x80;

    /* 2. 如果剩余空间不足8字节，先填充0x00并处理 */
    if (ctx->buffer_len > 56)
    {
        memset(ctx->buffer + ctx->buffer_len, 0, 64 - ctx->buffer_len);
        md5_transform(ctx->state, ctx->buffer);
        ctx->buffer_len = 0;
    }

    /* 3. 填充0x00到位置56 */
    memset(ctx->buffer + ctx->buffer_len, 0, 56 - ctx->buffer_len);

    /* 4. 追加原始消息长度（64位小端序）- MD5使用小端序 */
    uint64_t bit_len = ctx->bit_len;
    ctx->buffer[56] = (uint8_t)(bit_len & 0xFF);
    ctx->buffer[57] = (uint8_t)((bit_len >> 8) & 0xFF);
    ctx->buffer[58] = (uint8_t)((bit_len >> 16) & 0xFF);
    ctx->buffer[59] = (uint8_t)((bit_len >> 24) & 0xFF);
    ctx->buffer[60] = (uint8_t)((bit_len >> 32) & 0xFF);
    ctx->buffer[61] = (uint8_t)((bit_len >> 40) & 0xFF);
    ctx->buffer[62] = (uint8_t)((bit_len >> 48) & 0xFF);
    ctx->buffer[63] = (uint8_t)((bit_len >> 56) & 0xFF);

    /* 5. 处理最后一块 */
    md5_transform(ctx->state, ctx->buffer);

    /* 6. 将state[4]转换为16字节小端序数组 - MD5输出是小端序 */
    cks_MD5 hash;
    for (int i = 0; i < 4; ++i)
    {
        hash.bytes[i * 4 + 0] = (uint8_t)(ctx->state[i] & 0xFF);
        hash.bytes[i * 4 + 1] = (uint8_t)((ctx->state[i] >> 8) & 0xFF);
        hash.bytes[i * 4 + 2] = (uint8_t)((ctx->state[i] >> 16) & 0xFF);
        hash.bytes[i * 4 + 3] = (uint8_t)((ctx->state[i] >> 24) & 0xFF);
    }

    return hash;
}
