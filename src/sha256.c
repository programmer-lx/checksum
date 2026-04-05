#include "checksum/sha256.h"

#include <string.h>

#if CKS_ARCH_X86
    #include <immintrin.h>
    #include <wmmintrin.h>
#endif

#if CKS_ARCH_ARM
    #include <arm_neon.h>
    #include <arm_acle.h>
#endif

#include "checksum/detail/cpu.h"
#include "checksum/detail/call_once.h"

/* SHA-256 常量 K（FIPS 180-4） */
/* clang-format off */
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};
/* clang-format on */

static inline uint32_t rotr32(uint32_t x, int n)
{
    return (x >> n) | (x << (32 - n));
}

/* 处理单个 512 位（64字节）消息块（软件实现） */
static void sha256_process_block_soft(uint32_t state[8], const uint8_t block[64])
{
    uint32_t W[64];

    /* 消息调度 */
    for (int i = 0; i < 16; ++i)
    {
        W[i] = ((uint32_t)(block[i * 4 + 0]) << 24) | ((uint32_t)(block[i * 4 + 1]) << 16) |
               ((uint32_t)(block[i * 4 + 2]) << 8) | ((uint32_t)(block[i * 4 + 3]) << 0);
    }
    for (int i = 16; i < 64; ++i)
    {
        uint32_t s0 = rotr32(W[i - 15], 7) ^ rotr32(W[i - 15], 18) ^ (W[i - 15] >> 3);
        uint32_t s1 = rotr32(W[i - 2], 17) ^ rotr32(W[i - 2], 19) ^ (W[i - 2] >> 10);
        W[i] = W[i - 16] + s0 + W[i - 7] + s1;
    }

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int i = 0; i < 64; ++i)
    {
        uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t tmp1 = h + S1 + ch + K[i] + W[i];
        uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t tmp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + tmp1;
        d = c;
        c = b;
        b = a;
        a = tmp1 + tmp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

void cks_impl_sha256_update_soft(cks_SHA256_Context* ctx, const void* data, size_t len)
{
    const uint8_t* bytes = (const uint8_t*)data;

    while (len > 0)
    {
        uint32_t space = 64 - ctx->buffer_len;
        uint32_t copy = (len < space) ? (uint32_t)len : space;

        memcpy(ctx->buffer + ctx->buffer_len, bytes, copy);
        ctx->buffer_len += copy;
        ctx->bit_len += (uint64_t)copy * 8;
        bytes += copy;
        len -= copy;

        if (ctx->buffer_len == 64)
        {
            sha256_process_block_soft(ctx->state, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
}

cks_SHA256 cks_impl_sha256_end_soft(cks_SHA256_Context* ctx)
{
    uint32_t i = ctx->buffer_len;
    ctx->buffer[i++] = 0x80;

    if (i > 56)
    {
        while (i < 64)
            ctx->buffer[i++] = 0x00;
        sha256_process_block_soft(ctx->state, ctx->buffer);
        i = 0;
    }

    while (i < 56)
        ctx->buffer[i++] = 0x00;

    /* 大端序写入比特长度 */
    ctx->buffer[56] = (uint8_t)(ctx->bit_len >> 56);
    ctx->buffer[57] = (uint8_t)(ctx->bit_len >> 48);
    ctx->buffer[58] = (uint8_t)(ctx->bit_len >> 40);
    ctx->buffer[59] = (uint8_t)(ctx->bit_len >> 32);
    ctx->buffer[60] = (uint8_t)(ctx->bit_len >> 24);
    ctx->buffer[61] = (uint8_t)(ctx->bit_len >> 16);
    ctx->buffer[62] = (uint8_t)(ctx->bit_len >> 8);
    ctx->buffer[63] = (uint8_t)(ctx->bit_len >> 0);

    sha256_process_block_soft(ctx->state, ctx->buffer);

    cks_SHA256 result;
    for (int j = 0; j < 8; ++j)
    {
        result.bytes[j * 4 + 0] = (uint8_t)(ctx->state[j] >> 24);
        result.bytes[j * 4 + 1] = (uint8_t)(ctx->state[j] >> 16);
        result.bytes[j * 4 + 2] = (uint8_t)(ctx->state[j] >> 8);
        result.bytes[j * 4 + 3] = (uint8_t)(ctx->state[j] >> 0);
    }
    return result;
}

#if CKS_ARCH_X86

CKS_FUNC_ATTR_INTRINSICS_SHA256
static void sha256_process_block_sha(uint32_t state[8], const uint8_t block[64])
{
    /* 加载当前状态 */
    __m128i state0 = _mm_loadu_si128((__m128i*)&state[0]);
    __m128i state1 = _mm_loadu_si128((__m128i*)&state[4]);

    /* 整理成 SHA-NI 要求的寄存器格式 */
    __m128i tmp = _mm_shuffle_epi32(state0, 0xB1);
    state1 = _mm_shuffle_epi32(state1, 0x1B);
    state0 = _mm_alignr_epi8(tmp, state1, 8);
    state1 = _mm_blend_epi16(state1, tmp, 0xF0);

    __m128i abef_save = state0;
    __m128i cdgh_save = state1;

    /* 加载并转换消息块（大端序） */
    const __m128i* K128 = (const __m128i*)K;
    const __m128i MASK = _mm_set_epi64x(0x0c0d0e0f08090a0bULL, 0x0405060700010203ULL);

    __m128i msg0 = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(block + 0)), MASK);
    __m128i msg1 = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(block + 16)), MASK);
    __m128i msg2 = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(block + 32)), MASK);
    __m128i msg3 = _mm_shuffle_epi8(_mm_loadu_si128((__m128i*)(block + 48)), MASK);

    __m128i tmp0, tmp1, tmp2, tmp3;

    /* rounds 0-3 */
    tmp0 = _mm_add_epi32(msg0, _mm_load_si128(K128 + 0));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    tmp0 = _mm_shuffle_epi32(tmp0, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp0);

    /* rounds 4-7 */
    tmp1 = _mm_add_epi32(msg1, _mm_load_si128(K128 + 1));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp1);
    tmp1 = _mm_shuffle_epi32(tmp1, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp1);
    msg0 = _mm_sha256msg1_epu32(msg0, msg1);

    /* rounds 8-11 */
    tmp2 = _mm_add_epi32(msg2, _mm_load_si128(K128 + 2));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp2);
    tmp2 = _mm_shuffle_epi32(tmp2, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp2);
    msg1 = _mm_sha256msg1_epu32(msg1, msg2);

    /* rounds 12-15 */
    tmp3 = _mm_add_epi32(msg3, _mm_load_si128(K128 + 3));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp3);
    tmp0 = _mm_alignr_epi8(msg3, msg2, 4);
    msg0 = _mm_add_epi32(msg0, tmp0);
    msg0 = _mm_sha256msg2_epu32(msg0, msg3);
    tmp3 = _mm_shuffle_epi32(tmp3, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp3);
    msg2 = _mm_sha256msg1_epu32(msg2, msg3);

    /* rounds 16-19 */
    tmp0 = _mm_add_epi32(msg0, _mm_load_si128(K128 + 4));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    tmp1 = _mm_alignr_epi8(msg0, msg3, 4);
    msg1 = _mm_add_epi32(msg1, tmp1);
    msg1 = _mm_sha256msg2_epu32(msg1, msg0);
    tmp0 = _mm_shuffle_epi32(tmp0, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp0);
    msg3 = _mm_sha256msg1_epu32(msg3, msg0);

    /* rounds 20-23 */
    tmp1 = _mm_add_epi32(msg1, _mm_load_si128(K128 + 5));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp1);
    tmp2 = _mm_alignr_epi8(msg1, msg0, 4);
    msg2 = _mm_add_epi32(msg2, tmp2);
    msg2 = _mm_sha256msg2_epu32(msg2, msg1);
    tmp1 = _mm_shuffle_epi32(tmp1, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp1);
    msg0 = _mm_sha256msg1_epu32(msg0, msg1);

    /* rounds 24-27 */
    tmp2 = _mm_add_epi32(msg2, _mm_load_si128(K128 + 6));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp2);
    tmp3 = _mm_alignr_epi8(msg2, msg1, 4);
    msg3 = _mm_add_epi32(msg3, tmp3);
    msg3 = _mm_sha256msg2_epu32(msg3, msg2);
    tmp2 = _mm_shuffle_epi32(tmp2, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp2);
    msg1 = _mm_sha256msg1_epu32(msg1, msg2);

    /* rounds 28-31 */
    tmp3 = _mm_add_epi32(msg3, _mm_load_si128(K128 + 7));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp3);
    tmp0 = _mm_alignr_epi8(msg3, msg2, 4);
    msg0 = _mm_add_epi32(msg0, tmp0);
    msg0 = _mm_sha256msg2_epu32(msg0, msg3);
    tmp3 = _mm_shuffle_epi32(tmp3, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp3);
    msg2 = _mm_sha256msg1_epu32(msg2, msg3);

    /* rounds 32-35 */
    tmp0 = _mm_add_epi32(msg0, _mm_load_si128(K128 + 8));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    tmp1 = _mm_alignr_epi8(msg0, msg3, 4);
    msg1 = _mm_add_epi32(msg1, tmp1);
    msg1 = _mm_sha256msg2_epu32(msg1, msg0);
    tmp0 = _mm_shuffle_epi32(tmp0, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp0);
    msg3 = _mm_sha256msg1_epu32(msg3, msg0);

    /* rounds 36-39 */
    tmp1 = _mm_add_epi32(msg1, _mm_load_si128(K128 + 9));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp1);
    tmp2 = _mm_alignr_epi8(msg1, msg0, 4);
    msg2 = _mm_add_epi32(msg2, tmp2);
    msg2 = _mm_sha256msg2_epu32(msg2, msg1);
    tmp1 = _mm_shuffle_epi32(tmp1, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp1);
    msg0 = _mm_sha256msg1_epu32(msg0, msg1);

    /* rounds 40-43 */
    tmp2 = _mm_add_epi32(msg2, _mm_load_si128(K128 + 10));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp2);
    tmp3 = _mm_alignr_epi8(msg2, msg1, 4);
    msg3 = _mm_add_epi32(msg3, tmp3);
    msg3 = _mm_sha256msg2_epu32(msg3, msg2);
    tmp2 = _mm_shuffle_epi32(tmp2, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp2);
    msg1 = _mm_sha256msg1_epu32(msg1, msg2);

    /* rounds 44-47 */
    tmp3 = _mm_add_epi32(msg3, _mm_load_si128(K128 + 11));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp3);
    tmp0 = _mm_alignr_epi8(msg3, msg2, 4);
    msg0 = _mm_add_epi32(msg0, tmp0);
    msg0 = _mm_sha256msg2_epu32(msg0, msg3);
    tmp3 = _mm_shuffle_epi32(tmp3, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp3);
    msg2 = _mm_sha256msg1_epu32(msg2, msg3);

    /* rounds 48-51 */
    tmp0 = _mm_add_epi32(msg0, _mm_load_si128(K128 + 12));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    tmp1 = _mm_alignr_epi8(msg0, msg3, 4);
    msg1 = _mm_add_epi32(msg1, tmp1);
    msg1 = _mm_sha256msg2_epu32(msg1, msg0);
    tmp0 = _mm_shuffle_epi32(tmp0, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp0);
    msg3 = _mm_sha256msg1_epu32(msg3, msg0);

    /* rounds 52-55 */
    tmp1 = _mm_add_epi32(msg1, _mm_load_si128(K128 + 13));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp1);
    tmp2 = _mm_alignr_epi8(msg1, msg0, 4);
    msg2 = _mm_add_epi32(msg2, tmp2);
    msg2 = _mm_sha256msg2_epu32(msg2, msg1);
    tmp1 = _mm_shuffle_epi32(tmp1, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp1);

    /* rounds 56-59 */
    tmp2 = _mm_add_epi32(msg2, _mm_load_si128(K128 + 14));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp2);
    tmp3 = _mm_alignr_epi8(msg2, msg1, 4);
    msg3 = _mm_add_epi32(msg3, tmp3);
    msg3 = _mm_sha256msg2_epu32(msg3, msg2);
    tmp2 = _mm_shuffle_epi32(tmp2, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp2);

    /* rounds 60-63 */
    tmp3 = _mm_add_epi32(msg3, _mm_load_si128(K128 + 15));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp3);
    tmp3 = _mm_shuffle_epi32(tmp3, 0x0E);
    state0 = _mm_sha256rnds2_epu32(state0, state1, tmp3);

    /* 累加到保存的状态 */
    state0 = _mm_add_epi32(state0, abef_save);
    state1 = _mm_add_epi32(state1, cdgh_save);

    /* 还原为 state[0..7] 排列 */
    tmp0 = _mm_shuffle_epi32(state0, 0x1B);
    state1 = _mm_shuffle_epi32(state1, 0xB1);
    state0 = _mm_blend_epi16(tmp0, state1, 0xF0);
    state1 = _mm_alignr_epi8(state1, tmp0, 8);

    _mm_storeu_si128((__m128i*)&state[0], state0);
    _mm_storeu_si128((__m128i*)&state[4], state1);
}

CKS_FUNC_ATTR_INTRINSICS_SHA256
void cks_impl_sha256_update_sha(cks_SHA256_Context* ctx, const void* data, size_t len)
{
    const uint8_t* bytes = (const uint8_t*)data;

    while (len > 0)
    {
        uint32_t space = 64 - ctx->buffer_len;
        uint32_t copy = (len < space) ? (uint32_t)len : space;

        memcpy(ctx->buffer + ctx->buffer_len, bytes, copy);
        ctx->buffer_len += copy;
        ctx->bit_len += (uint64_t)copy * 8;
        bytes += copy;
        len -= copy;

        if (ctx->buffer_len == 64)
        {
            sha256_process_block_sha(ctx->state, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
}

CKS_FUNC_ATTR_INTRINSICS_SHA256
cks_SHA256 cks_impl_sha256_end_sha(cks_SHA256_Context* ctx)
{
    uint32_t i = ctx->buffer_len;
    ctx->buffer[i++] = 0x80;

    if (i > 56)
    {
        while (i < 64)
            ctx->buffer[i++] = 0x00;
        sha256_process_block_sha(ctx->state, ctx->buffer);
        i = 0;
    }

    while (i < 56)
        ctx->buffer[i++] = 0x00;

    ctx->buffer[56] = (uint8_t)(ctx->bit_len >> 56);
    ctx->buffer[57] = (uint8_t)(ctx->bit_len >> 48);
    ctx->buffer[58] = (uint8_t)(ctx->bit_len >> 40);
    ctx->buffer[59] = (uint8_t)(ctx->bit_len >> 32);
    ctx->buffer[60] = (uint8_t)(ctx->bit_len >> 24);
    ctx->buffer[61] = (uint8_t)(ctx->bit_len >> 16);
    ctx->buffer[62] = (uint8_t)(ctx->bit_len >> 8);
    ctx->buffer[63] = (uint8_t)(ctx->bit_len >> 0);

    sha256_process_block_sha(ctx->state, ctx->buffer);

    cks_SHA256 result;
    for (int j = 0; j < 8; ++j)
    {
        result.bytes[j * 4 + 0] = (uint8_t)(ctx->state[j] >> 24);
        result.bytes[j * 4 + 1] = (uint8_t)(ctx->state[j] >> 16);
        result.bytes[j * 4 + 2] = (uint8_t)(ctx->state[j] >> 8);
        result.bytes[j * 4 + 3] = (uint8_t)(ctx->state[j] >> 0);
    }
    return result;
}

#endif /* CKS_ARCH_X86 */

#if CKS_ARCH_ARM

CKS_FUNC_ATTR_INTRINSICS_ARM_SHA256
static void sha256_process_block_arm(uint32_t state[8], const uint8_t block[64])
{
    uint32x4_t STATE0 = vld1q_u32(&state[0]);
    uint32x4_t STATE1 = vld1q_u32(&state[4]);

    uint32x4_t ABCD_SAVE = STATE0;
    uint32x4_t EFGH_SAVE = STATE1;

    /* 加载消息，ARM 是小端序，需要字节反转 */
    uint32x4_t MSG0 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(block + 0)));
    uint32x4_t MSG1 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(block + 16)));
    uint32x4_t MSG2 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(block + 32)));
    uint32x4_t MSG3 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(block + 48)));

    uint32x4_t TMP0, TMP1, TMP2, TMP3;

    /* rounds 0-3 */
    TMP0 = vaddq_u32(MSG0, vld1q_u32(&K[0]));
    TMP2 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
    STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP0);
    MSG0 = vsha256su0q_u32(MSG0, MSG1);

    /* rounds 4-7 */
    TMP1 = vaddq_u32(MSG1, vld1q_u32(&K[4]));
    TMP2 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
    STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
    MSG0 = vsha256su1q_u32(MSG0, MSG2, MSG3);
    MSG1 = vsha256su0q_u32(MSG1, MSG2);

    /* rounds 8-11 */
    TMP2 = vaddq_u32(MSG2, vld1q_u32(&K[8]));
    TMP3 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP2);
    STATE1 = vsha256h2q_u32(STATE1, TMP3, TMP2);
    MSG1 = vsha256su1q_u32(MSG1, MSG3, MSG0);
    MSG2 = vsha256su0q_u32(MSG2, MSG3);

    /* rounds 12-15 */
    TMP3 = vaddq_u32(MSG3, vld1q_u32(&K[12]));
    TMP0 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP3);
    STATE1 = vsha256h2q_u32(STATE1, TMP0, TMP3);
    MSG2 = vsha256su1q_u32(MSG2, MSG0, MSG1);
    MSG3 = vsha256su0q_u32(MSG3, MSG0);

    /* rounds 16-19 */
    TMP0 = vaddq_u32(MSG0, vld1q_u32(&K[16]));
    TMP1 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
    STATE1 = vsha256h2q_u32(STATE1, TMP1, TMP0);
    MSG3 = vsha256su1q_u32(MSG3, MSG1, MSG2);
    MSG0 = vsha256su0q_u32(MSG0, MSG1);

    /* rounds 20-23 */
    TMP1 = vaddq_u32(MSG1, vld1q_u32(&K[20]));
    TMP2 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
    STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
    MSG0 = vsha256su1q_u32(MSG0, MSG2, MSG3);
    MSG1 = vsha256su0q_u32(MSG1, MSG2);

    /* rounds 24-27 */
    TMP2 = vaddq_u32(MSG2, vld1q_u32(&K[24]));
    TMP3 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP2);
    STATE1 = vsha256h2q_u32(STATE1, TMP3, TMP2);
    MSG1 = vsha256su1q_u32(MSG1, MSG3, MSG0);
    MSG2 = vsha256su0q_u32(MSG2, MSG3);

    /* rounds 28-31 */
    TMP3 = vaddq_u32(MSG3, vld1q_u32(&K[28]));
    TMP0 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP3);
    STATE1 = vsha256h2q_u32(STATE1, TMP0, TMP3);
    MSG2 = vsha256su1q_u32(MSG2, MSG0, MSG1);
    MSG3 = vsha256su0q_u32(MSG3, MSG0);

    /* rounds 32-35 */
    TMP0 = vaddq_u32(MSG0, vld1q_u32(&K[32]));
    TMP1 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
    STATE1 = vsha256h2q_u32(STATE1, TMP1, TMP0);
    MSG3 = vsha256su1q_u32(MSG3, MSG1, MSG2);
    MSG0 = vsha256su0q_u32(MSG0, MSG1);

    /* rounds 36-39 */
    TMP1 = vaddq_u32(MSG1, vld1q_u32(&K[36]));
    TMP2 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
    STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);
    MSG0 = vsha256su1q_u32(MSG0, MSG2, MSG3);
    MSG1 = vsha256su0q_u32(MSG1, MSG2);

    /* rounds 40-43 */
    TMP2 = vaddq_u32(MSG2, vld1q_u32(&K[40]));
    TMP3 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP2);
    STATE1 = vsha256h2q_u32(STATE1, TMP3, TMP2);
    MSG1 = vsha256su1q_u32(MSG1, MSG3, MSG0);
    MSG2 = vsha256su0q_u32(MSG2, MSG3);

    /* rounds 44-47 */
    TMP3 = vaddq_u32(MSG3, vld1q_u32(&K[44]));
    TMP0 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP3);
    STATE1 = vsha256h2q_u32(STATE1, TMP0, TMP3);
    MSG2 = vsha256su1q_u32(MSG2, MSG0, MSG1);
    MSG3 = vsha256su0q_u32(MSG3, MSG0);

    /* rounds 48-51 */
    TMP0 = vaddq_u32(MSG0, vld1q_u32(&K[48]));
    TMP1 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP0);
    STATE1 = vsha256h2q_u32(STATE1, TMP1, TMP0);
    MSG3 = vsha256su1q_u32(MSG3, MSG1, MSG2);

    /* rounds 52-55 */
    TMP1 = vaddq_u32(MSG1, vld1q_u32(&K[52]));
    TMP2 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP1);
    STATE1 = vsha256h2q_u32(STATE1, TMP2, TMP1);

    /* rounds 56-59 */
    TMP2 = vaddq_u32(MSG2, vld1q_u32(&K[56]));
    TMP3 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP2);
    STATE1 = vsha256h2q_u32(STATE1, TMP3, TMP2);

    /* rounds 60-63 */
    TMP3 = vaddq_u32(MSG3, vld1q_u32(&K[60]));
    TMP0 = STATE0;
    STATE0 = vsha256hq_u32(STATE0, STATE1, TMP3);
    STATE1 = vsha256h2q_u32(STATE1, TMP0, TMP3);

    /* 累加 */
    STATE0 = vaddq_u32(STATE0, ABCD_SAVE);
    STATE1 = vaddq_u32(STATE1, EFGH_SAVE);

    vst1q_u32(&state[0], STATE0);
    vst1q_u32(&state[4], STATE1);
}

CKS_FUNC_ATTR_INTRINSICS_ARM_SHA256
void cks_impl_sha256_update_arm(cks_SHA256_Context* ctx, const void* data, size_t len)
{
    const uint8_t* bytes = (const uint8_t*)data;

    while (len > 0)
    {
        uint32_t space = 64 - ctx->buffer_len;
        uint32_t copy = (len < space) ? (uint32_t)len : space;

        memcpy(ctx->buffer + ctx->buffer_len, bytes, copy);
        ctx->buffer_len += copy;
        ctx->bit_len += (uint64_t)copy * 8;
        bytes += copy;
        len -= copy;

        if (ctx->buffer_len == 64)
        {
            sha256_process_block_arm(ctx->state, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
}

CKS_FUNC_ATTR_INTRINSICS_ARM_SHA256
cks_SHA256 cks_impl_sha256_end_arm(cks_SHA256_Context* ctx)
{
    uint32_t i = ctx->buffer_len;
    ctx->buffer[i++] = 0x80;

    if (i > 56)
    {
        while (i < 64)
            ctx->buffer[i++] = 0x00;
        sha256_process_block_arm(ctx->state, ctx->buffer);
        i = 0;
    }

    while (i < 56)
        ctx->buffer[i++] = 0x00;

    ctx->buffer[56] = (uint8_t)(ctx->bit_len >> 56);
    ctx->buffer[57] = (uint8_t)(ctx->bit_len >> 48);
    ctx->buffer[58] = (uint8_t)(ctx->bit_len >> 40);
    ctx->buffer[59] = (uint8_t)(ctx->bit_len >> 32);
    ctx->buffer[60] = (uint8_t)(ctx->bit_len >> 24);
    ctx->buffer[61] = (uint8_t)(ctx->bit_len >> 16);
    ctx->buffer[62] = (uint8_t)(ctx->bit_len >> 8);
    ctx->buffer[63] = (uint8_t)(ctx->bit_len >> 0);

    sha256_process_block_arm(ctx->state, ctx->buffer);

    cks_SHA256 result;
    for (int j = 0; j < 8; ++j)
    {
        result.bytes[j * 4 + 0] = (uint8_t)(ctx->state[j] >> 24);
        result.bytes[j * 4 + 1] = (uint8_t)(ctx->state[j] >> 16);
        result.bytes[j * 4 + 2] = (uint8_t)(ctx->state[j] >> 8);
        result.bytes[j * 4 + 3] = (uint8_t)(ctx->state[j] >> 0);
    }
    return result;
}

#endif /* CKS_ARCH_ARM */

/* 分发器结构体 */
typedef struct
{
    void (*update)(cks_SHA256_Context*, const void*, size_t);
    cks_SHA256 (*end)(cks_SHA256_Context*);
} SHA256Dispatch;

static SHA256Dispatch g_disp;
static cks_once_flag_t g_disp_once = CKS_ONCE_FLAG_INIT;

static void sha256_dispatch_init(void)
{
    cks_CpuInfo info = cks_cpu_info();

#if CKS_ARCH_X86
    if (info.sha && info.sse4_1)
    {
        SHA256Dispatch disp = { cks_impl_sha256_update_sha, cks_impl_sha256_end_sha };
        g_disp = disp;
        return;
    }
#endif

#if CKS_ARCH_ARM
    if (info.arm_sha2)
    {
        SHA256Dispatch disp = { cks_impl_sha256_update_arm, cks_impl_sha256_end_arm };
        g_disp = disp;
        return;
    }
#endif

    SHA256Dispatch disp = { cks_impl_sha256_update_soft, cks_impl_sha256_end_soft };
    g_disp = disp;
}

cks_SHA256_Context cks_sha256_begin(void)
{
    /* init dispatcher */
    cks_call_once(&g_disp_once, sha256_dispatch_init);

    cks_SHA256_Context ctx;
    /* 初始哈希值H(0)（FIPS 180-4规范） */
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

void cks_sha256_update(cks_SHA256_Context* ctx, const void* data, size_t len)
{
    g_disp.update(ctx, data, len);
}

cks_SHA256 cks_sha256_end(cks_SHA256_Context* ctx)
{
    return g_disp.end(ctx);
}
