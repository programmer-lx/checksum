#pragma once

#include <stdint.h>

typedef enum
{
    CKS_VENDOR_UNKNOWN = 0,
    CKS_VENDOR_INTEL,
    CKS_VENDOR_AMD
} cks_Vendor;

typedef struct
{
    /* ------------------ common info ------------------ */
    cks_Vendor   vendor;
    char         vendor_name[13];
    unsigned int logical_cores;
    unsigned int physical_cores;
    
    unsigned int hyper_threads          : 1;

    /* ------------------ x86 features ------------------ */
    unsigned int fxsr                   : 1;

    /* SSE family */
    unsigned int sse                    : 1;
    unsigned int sse2                   : 1;
    unsigned int sse3                   : 1;
    unsigned int ssse3                  : 1;
    unsigned int sse4_1                 : 1;
    unsigned int sse4_2                 : 1;

    /* XSAVE & OS_XSAVE */
    unsigned int xsave                  : 1;
    unsigned int os_xsave               : 1;

    /* AVX family */
    unsigned int avx                    : 1;
    unsigned int f16c                   : 1;
    unsigned int fma3                   : 1;
    unsigned int avx2                   : 1;
    unsigned int avx_vnni               : 1;
    unsigned int avx_vnni_int8          : 1;
    unsigned int avx_ne_convert         : 1;
    unsigned int avx_ifma               : 1;
    unsigned int avx_vnni_int16         : 1;
    unsigned int sha512                 : 1;
    unsigned int sm3                    : 1;
    unsigned int sm4                    : 1;

    /* AVX-512 family */
    unsigned int avx512_f               : 1;
    unsigned int avx512_bw              : 1;
    unsigned int avx512_cd              : 1;
    unsigned int avx512_dq              : 1;
    unsigned int avx512_ifma            : 1;
    unsigned int avx512_vl              : 1;
    unsigned int avx512_vpopcntdq       : 1;
    unsigned int avx512_bf16            : 1;
    unsigned int avx512_bitalg          : 1;
    unsigned int avx512_vbmi            : 1;
    unsigned int avx512_vbmi2           : 1;
    unsigned int avx512_vnni            : 1;
    unsigned int avx512_vp2intersect    : 1;
    unsigned int avx512_fp16            : 1;

    /* other */
    unsigned int popcnt                 : 1;
    unsigned int aes_ni                 : 1;
    unsigned int sha                    : 1;

    /* ------------------ arm features ------------------ */
    unsigned int neon                   : 1;
    unsigned int sve                    : 1;
    unsigned int arm_crc32              : 1;
    unsigned int arm_sha2               : 1;
} cks_CpuInfo;

cks_CpuInfo cks_cpu_info(void);
