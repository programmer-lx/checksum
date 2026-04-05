#include "checksum/detail/cpu.h"

#include <stdint.h>
#include <string.h>

#include "checksum/detail/os_detect.h"
#include "checksum/detail/arch.h"

#if CKS_ARCH_X86
    #if defined(_MSC_VER)
        #include <intrin.h>
    #else
        #include <cpuid.h>
    #endif
    #include <emmintrin.h>
#endif

#if CKS_ARCH_ARM
    #if CKS_OS_LINUX
        #include <sys/auxv.h>
        #include <asm/hwcap.h>
    #endif
#endif

#include "checksum/detail/common.h"
#include "checksum/detail/compiler.h"
#include "checksum/detail/attributes.h"

/* 宏替代模板函数 */
#define BIT_IS_OPEN(value, bit) (((value) & (bit)) != 0)

#if CKS_ARCH_X86

/* EAX1_ECX0_ECX */
static const uint32_t EAX1_ECX0_ECX_SSE3                 = (UINT32_C(1) <<  0);
static const uint32_t EAX1_ECX0_ECX_SSSE3                = (UINT32_C(1) <<  9);
static const uint32_t EAX1_ECX0_ECX_FMA3                 = (UINT32_C(1) << 12);
static const uint32_t EAX1_ECX0_ECX_SSE4_1               = (UINT32_C(1) << 19);
static const uint32_t EAX1_ECX0_ECX_SSE4_2               = (UINT32_C(1) << 20);
static const uint32_t EAX1_ECX0_ECX_POPCNT               = (UINT32_C(1) << 23);
static const uint32_t EAX1_ECX0_ECX_AES_NI               = (UINT32_C(1) << 25);
static const uint32_t EAX1_ECX0_ECX_XSAVE                = (UINT32_C(1) << 26);
static const uint32_t EAX1_ECX0_ECX_OS_XSAVE             = (UINT32_C(1) << 27);
static const uint32_t EAX1_ECX0_ECX_AVX                  = (UINT32_C(1) << 28);
static const uint32_t EAX1_ECX0_ECX_F16C                 = (UINT32_C(1) << 29);

/* EAX1_ECX0_EDX */
static const uint32_t EAX1_ECX0_EDX_FXSR                 = (UINT32_C(1) << 24);
static const uint32_t EAX1_ECX0_EDX_SSE                  = (UINT32_C(1) << 25);
static const uint32_t EAX1_ECX0_EDX_SSE2                 = (UINT32_C(1) << 26);

/* EAX7_ECX0_EBX */
static const uint32_t EAX7_ECX0_EBX_AVX2                 = (UINT32_C(1) <<  5);
static const uint32_t EAX7_ECX0_EBX_AVX512_F             = (UINT32_C(1) << 16);
static const uint32_t EAX7_ECX0_EBX_AVX512_DQ            = (UINT32_C(1) << 17);
static const uint32_t EAX7_ECX0_EBX_AVX512_IFMA          = (UINT32_C(1) << 21);
static const uint32_t EAX7_ECX0_EBX_AVX512_CD            = (UINT32_C(1) << 28);
static const uint32_t EAX7_ECX0_EBX_SHA                  = (UINT32_C(1) << 29);
static const uint32_t EAX7_ECX0_EBX_AVX512_BW            = (UINT32_C(1) << 30);
static const uint32_t EAX7_ECX0_EBX_AVX512_VL            = (UINT32_C(1) << 31);

/* EAX7_ECX0_ECX */
static const uint32_t EAX7_ECX0_ECX_AVX512_VBMI          = (UINT32_C(1) <<  1);
static const uint32_t EAX7_ECX0_ECX_AVX512_VBMI2         = (UINT32_C(1) <<  6);
static const uint32_t EAX7_ECX0_ECX_AVX512_VNNI          = (UINT32_C(1) << 11);
static const uint32_t EAX7_ECX0_ECX_AVX512_BITALG        = (UINT32_C(1) << 12);
static const uint32_t EAX7_ECX0_ECX_AVX512_VPOPCNTDQ     = (UINT32_C(1) << 14);

/* EAX7_ECX0_EDX */
static const uint32_t EAX7_ECX0_EDX_AVX512_VP2INTERSECT  = (UINT32_C(1) <<  8);
static const uint32_t EAX7_ECX0_EDX_AVX512_FP16          = (UINT32_C(1) << 23);

/* EAX7_ECX1_EAX */
static const uint32_t EAX7_ECX1_EAX_SHA512               = (UINT32_C(1) <<  0);
static const uint32_t EAX7_ECX1_EAX_SM3                  = (UINT32_C(1) <<  1);
static const uint32_t EAX7_ECX1_EAX_SM4                  = (UINT32_C(1) <<  2);
static const uint32_t EAX7_ECX1_EAX_AVX_VNNI             = (UINT32_C(1) <<  4);
static const uint32_t EAX7_ECX1_EAX_AVX512_BF16          = (UINT32_C(1) <<  5);
static const uint32_t EAX7_ECX1_EAX_AVX_IFMA             = (UINT32_C(1) << 23);

/* EAX7_ECX1_EDX */
static const uint32_t EAX7_ECX1_EDX_AVX_VNNI_INT8        = (UINT32_C(1) <<  4);
static const uint32_t EAX7_ECX1_EDX_AVX_NE_CONVERT       = (UINT32_C(1) <<  5);
static const uint32_t EAX7_ECX1_EDX_AVX_VNNI_INT16       = (UINT32_C(1) << 10);

/* XSAVE */
static const uint64_t XSAVE_XMM                          = (UINT64_C(1) << 1);
static const uint64_t XSAVE_YMM                          = (UINT64_C(1) << 2);
static const uint64_t XSAVE_K0_K7                        = (UINT64_C(1) << 5);
static const uint64_t XSAVE_ZMM_LOW_256                  = (UINT64_C(1) << 6);
static const uint64_t XSAVE_ZMM_HIGH_256                 = (UINT64_C(1) << 7);

/* leaf: EAX, sub_leaf: ECX */
static void cpuid(const uint32_t leaf, const uint32_t sub_leaf, uint32_t abcd[4])
{
#if defined(_MSC_VER)
    int regs[4];
    __cpuidex(regs, (int)leaf, (int)sub_leaf);
    for (int i = 0; i < 4; ++i)
    {
        abcd[i] = (uint32_t)regs[i];
    }
#else
    uint32_t a, b, c, d;
    __cpuid_count(leaf, sub_leaf, a, b, c, d);
    abcd[0] = a;
    abcd[1] = b;
    abcd[2] = c;
    abcd[3] = d;
#endif
}

static uint64_t xgetbv(uint32_t idx)
{
#if defined(_MSC_VER)
    return _xgetbv(idx);
#else
    uint32_t eax, edx;
    __asm__ __volatile__ ("xgetbv" : "=a"(eax), "=d"(edx) : "c"(idx));
    return ((uint64_t)edx << 32) | eax;
#endif
}

cks_CpuInfo cks_cpu_info(void)
{
    cks_CpuInfo result;
    memset(&result, 0, sizeof(result));

    uint32_t abcd[4];

    cpuid(0, 0, abcd);
    const uint32_t max_leaf = abcd[0];

    uint64_t xcr0 = 0;
    uint32_t eax1_ecx0_edx = 0;

    /* ------------------ EAX 0 ------------------ */
    {
        const uint32_t ebx = abcd[1];
        const uint32_t ecx = abcd[2];
        const uint32_t edx = abcd[3];
        memcpy(result.vendor_name, &ebx, sizeof(uint32_t));
        memcpy(result.vendor_name + sizeof(uint32_t), &edx, sizeof(uint32_t));
        memcpy(result.vendor_name + 2 * sizeof(uint32_t), &ecx, sizeof(uint32_t));
        result.vendor_name[12] = 0;

        /* Intel "GenuineIntel" */
        if (ebx == 0x756e6547 && edx == 0x49656e69 && ecx == 0x6c65746e)
        {
            result.vendor = CKS_VENDOR_INTEL;
        }

        /* AMD "AuthenticAMD" */
        if (ebx == 0x68747541 && edx == 0x69746e65 && ecx == 0x444d4163)
        {
            result.vendor = CKS_VENDOR_AMD;
        }
    }

    /* ------------------ EAX 1 ------------------ */
    if (max_leaf >= 1)
    {
        cpuid(1, 0, abcd);
        const uint32_t ebx = abcd[1];
        const uint32_t ecx = abcd[2];
        const uint32_t edx = abcd[3];
        eax1_ecx0_edx = edx;

        result.logical_cores = (ebx >> 16) & 0xff;

        /* FXSR */
        result.fxsr = BIT_IS_OPEN(edx, EAX1_ECX0_EDX_FXSR);

        /* SSE family */
        result.sse = result.fxsr && BIT_IS_OPEN(edx, EAX1_ECX0_EDX_SSE);
        result.sse2 = result.sse && BIT_IS_OPEN(edx, EAX1_ECX0_EDX_SSE2);
        result.sse3 = result.sse2 && BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_SSE3);
        result.ssse3 = result.sse3 && BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_SSSE3);
        result.sse4_1 = result.ssse3 && BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_SSE4_1);
        result.sse4_2 = result.sse4_1 && BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_SSE4_2);

        /* XSAVE */
        result.xsave = BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_XSAVE);
        result.os_xsave = BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_OS_XSAVE);
        if (result.xsave && result.os_xsave)
        {
            xcr0 = xgetbv(0);
        }

        /* AVX family */
        const int os_support_avx = BIT_IS_OPEN(xcr0, XSAVE_XMM) && BIT_IS_OPEN(xcr0, XSAVE_YMM);

        result.avx = result.sse4_1 && BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_AVX) && os_support_avx;
        result.f16c = result.avx && BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_F16C);
        result.fma3 = result.avx && BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_FMA3);

        /* other */
        result.aes_ni = BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_AES_NI);
        result.popcnt = BIT_IS_OPEN(ecx, EAX1_ECX0_ECX_POPCNT);
    }

    /* ------------------ EAX 4 ------------------ */
    if (max_leaf >= 4)
    {
        cpuid(4, 0, abcd);
        const uint32_t eax = abcd[0];

        if (result.vendor == CKS_VENDOR_INTEL)
        {
            result.physical_cores = ((eax >> 26) & 0x3f) + 1;
        }
    }

    /* ------------------ EAX 7 ------------------ */
    if (max_leaf >= 7)
    {
        cpuid(7, 0, abcd);
        const uint32_t eax7_subleaf_count = abcd[0];
        {
            const uint32_t ebx = abcd[1];
            const uint32_t ecx = abcd[2];
            const uint32_t edx = abcd[3];

            result.avx2 = result.avx && BIT_IS_OPEN(ebx, EAX7_ECX0_EBX_AVX2);

            /* AVX-512 family */
            const int os_support_avx_512 = BIT_IS_OPEN(xcr0, XSAVE_XMM) &&
                                            BIT_IS_OPEN(xcr0, XSAVE_YMM) &&
                                            BIT_IS_OPEN(xcr0, XSAVE_K0_K7) &&
                                            BIT_IS_OPEN(xcr0, XSAVE_ZMM_LOW_256) &&
                                            BIT_IS_OPEN(xcr0, XSAVE_ZMM_HIGH_256);

            result.avx512_f = result.avx2 && result.fma3 && result.f16c &&
                               BIT_IS_OPEN(ebx, EAX7_ECX0_EBX_AVX512_F) && os_support_avx_512;
            result.avx512_bw = result.avx512_f && BIT_IS_OPEN(ebx, EAX7_ECX0_EBX_AVX512_BW);
            result.avx512_cd = result.avx512_f && BIT_IS_OPEN(ebx, EAX7_ECX0_EBX_AVX512_CD);
            result.avx512_dq = result.avx512_f && BIT_IS_OPEN(ebx, EAX7_ECX0_EBX_AVX512_DQ);
            result.avx512_ifma = result.avx512_f && BIT_IS_OPEN(ebx, EAX7_ECX0_EBX_AVX512_IFMA);
            result.avx512_vl = result.avx512_f && BIT_IS_OPEN(ebx, EAX7_ECX0_EBX_AVX512_VL);

            result.sha = BIT_IS_OPEN(ebx, EAX7_ECX0_EBX_SHA);

            result.avx512_vpopcntdq = result.avx512_f && BIT_IS_OPEN(ecx, EAX7_ECX0_ECX_AVX512_VPOPCNTDQ);
            result.avx512_bitalg = result.avx512_f && BIT_IS_OPEN(ecx, EAX7_ECX0_ECX_AVX512_BITALG);
            result.avx512_vbmi = result.avx512_f && BIT_IS_OPEN(ecx, EAX7_ECX0_ECX_AVX512_VBMI);
            result.avx512_vbmi2 = result.avx512_f && BIT_IS_OPEN(ecx, EAX7_ECX0_ECX_AVX512_VBMI2);
            result.avx512_vnni = result.avx512_f && BIT_IS_OPEN(ecx, EAX7_ECX0_ECX_AVX512_VNNI);

            result.avx512_vp2intersect = result.avx512_f && BIT_IS_OPEN(edx, EAX7_ECX0_EDX_AVX512_VP2INTERSECT);
            result.avx512_fp16 = result.avx512_f && BIT_IS_OPEN(edx, EAX7_ECX0_EDX_AVX512_FP16);
        }

        /* EAX 7 ECX 1 */
        if (eax7_subleaf_count >= 1)
        {
            cpuid(7, 1, abcd);
            const uint32_t eax = abcd[0];
            const uint32_t edx = abcd[3];

            result.avx_vnni = result.avx2 && BIT_IS_OPEN(eax, EAX7_ECX1_EAX_AVX_VNNI);
            result.avx_ifma = result.avx2 && BIT_IS_OPEN(eax, EAX7_ECX1_EAX_AVX_IFMA);

            result.avx512_bf16 = result.avx512_f && BIT_IS_OPEN(eax, EAX7_ECX1_EAX_AVX512_BF16);

            result.sha512 = result.avx2 && BIT_IS_OPEN(eax, EAX7_ECX1_EAX_SHA512);
            result.sm3 = result.avx2 && BIT_IS_OPEN(eax, EAX7_ECX1_EAX_SM3);
            result.sm4 = result.avx2 && BIT_IS_OPEN(eax, EAX7_ECX1_EAX_SM4);

            result.avx_vnni_int8 = result.avx2 && BIT_IS_OPEN(edx, EAX7_ECX1_EDX_AVX_VNNI_INT8);
            result.avx_ne_convert = result.avx2 && BIT_IS_OPEN(edx, EAX7_ECX1_EDX_AVX_NE_CONVERT);
            result.avx_vnni_int16 = result.avx2 && BIT_IS_OPEN(edx, EAX7_ECX1_EDX_AVX_VNNI_INT16);
        }
    }

    /* ------------------------------------ ext ------------------------------------ */
    cpuid(0x80000000, 0, abcd);
    const uint32_t max_ext_leaf = abcd[0];

    /* ------------------ EAX 0x8000'0008 ------------------ */
    if (max_ext_leaf >= 0x80000008)
    {
        if (result.vendor == CKS_VENDOR_AMD)
        {
            cpuid(0x80000008, 0, abcd);
            const uint32_t ecx = abcd[2];
            result.physical_cores = (ecx & 0xff) + 1;
        }
    }

    if (max_leaf >= 1)
    {
        result.hyper_threads = (eax1_ecx0_edx & (UINT32_C(1) << 28)) && (result.physical_cores < result.logical_cores);
    }

    return result;
}

#elif CKS_ARCH_ARM

cks_CpuInfo cks_cpu_info(void)
{
    cks_CpuInfo result;
    memset(&result, 0, sizeof(result));

#if CKS_OS_LINUX
    unsigned long hwcap = getauxval(AT_HWCAP);

    result.neon = (hwcap & HWCAP_ASIMD) ? 1 : 0;
    result.arm_sha2 = (hwcap & HWCAP_SHA2) ? 1 : 0;
    result.arm_crc32 = (hwcap & HWCAP_CRC32) ? 1 : 0;
    result.sve = (hwcap & HWCAP_SVE) ? 1 : 0;
#endif

    return result;
}

#endif
