#include "checksum/crc32c.h"

#include <string.h>

#if CKS_ARCH_X86
    #include <nmmintrin.h>
#endif

#if CKS_ARCH_ARM
    #include <arm_acle.h>
#endif

#include "checksum/detail/cpu.h"
#include "checksum/detail/call_once.h"

/* CRC32C 表（运行时初始化） */
static uint32_t crc32c_table[256];
static int crc32c_table_initialized = 0;

static void crc32c_init_table(void)
{
    if (crc32c_table_initialized)
        return;
    const uint32_t POLY = 0x82F63B78;
    for (uint32_t i = 0; i < 256; i++)
    {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
        {
            c = (c & 1) ? ((c >> 1) ^ POLY) : (c >> 1);
        }
        crc32c_table[i] = c;
    }
    crc32c_table_initialized = 1;
}

cks_CRC32C cks_impl_crc32c_update_soft(cks_CRC32C crc, const void* data, size_t size)
{
    crc32c_init_table();
    const uint8_t* bytes = (const uint8_t*)data;

    for (size_t i = 0; i < size; i++)
    {
        uint8_t index = (crc.bytes ^ bytes[i]) & 0xff;
        crc.bytes = (crc.bytes >> 8) ^ crc32c_table[index];
    }

    return crc;
}

#if CKS_ARCH_X86

CKS_FUNC_ATTR_INTRINSICS_SSE4_2
cks_CRC32C cks_impl_crc32c_update_sse42(cks_CRC32C crc, const void* data, size_t size)
{
    const uint8_t* bytes = (const uint8_t*)data;

    size_t i = 0;

    #if CKS_ARCH_X86_64
    for (; i + 8 <= size; i += 8)
    {
        uint64_t v;
        memcpy(&v, bytes + i, 8);
        crc.bytes = (uint32_t)_mm_crc32_u64(crc.bytes, v);
    }
    #endif

    for (; i + 4 <= size; i += 4)
    {
        uint32_t v;
        memcpy(&v, bytes + i, 4);
        crc.bytes = _mm_crc32_u32(crc.bytes, v);
    }

    for (; i < size; ++i)
    {
        crc.bytes = _mm_crc32_u8(crc.bytes, bytes[i]);
    }

    return crc;
}

#endif

#if CKS_ARCH_ARM

CKS_FUNC_ATTR_INTRINSICS_ARM_CRC32
cks_CRC32C cks_impl_crc32c_update_arm(cks_CRC32C crc, const void* data, size_t size)
{
    const uint8_t* bytes = (const uint8_t*)data;

    size_t i = 0;

    #if CKS_ARCH_ARM_64
    for (; i + 8 <= size; i += 8)
    {
        uint64_t v;
        memcpy(&v, bytes + i, 8);
        crc.bytes = __crc32cd(crc.bytes, v);
    }
    #endif

    for (; i + 4 <= size; i += 4)
    {
        uint32_t v;
        memcpy(&v, bytes + i, 4);
        crc.bytes = __crc32cw(crc.bytes, v);
    }

    for (; i < size; ++i)
    {
        crc.bytes = __crc32cb(crc.bytes, bytes[i]);
    }

    return crc;
}

#endif

typedef struct
{
    cks_CRC32C (*update)(cks_CRC32C, const void*, size_t);
} CRC32C_Dispatch;

static CRC32C_Dispatch g_disp;
static cks_once_flag_t g_disp_once = CKS_ONCE_FLAG_INIT;

static void crc32c_dispatch_init(void)
{
    const cks_CpuInfo info = cks_cpu_info();
#if CKS_ARCH_X86
    if (info.sse4_2)
    {
        CRC32C_Dispatch disp = { cks_impl_crc32c_update_sse42 };
        g_disp = disp;
        return;
    }
#endif

#if CKS_ARCH_ARM
    if (info.arm_crc32)
    {
        CRC32C_Dispatch disp = { cks_impl_crc32c_update_arm };
        g_disp = disp;
        return;
    }
#endif

    CRC32C_Dispatch disp = { cks_impl_crc32c_update_soft };
    g_disp = disp;
}


cks_CRC32C cks_crc32c_begin(void)
{
    /* init dispatcher */
    cks_call_once(&g_disp_once, crc32c_dispatch_init);

    cks_CRC32C crc;
    crc.bytes = UINT32_C(0xffffffff);
    return crc;
}

cks_CRC32C cks_crc32c_update(cks_CRC32C crc, const void* data, size_t len)
{
    return g_disp.update(crc, data, len);
}

cks_CRC32C cks_crc32c_end(cks_CRC32C crc)
{
    cks_CRC32C result;
    result.bytes = crc.bytes ^ UINT32_C(0xffffffff);
    return result;
}