#pragma once

namespace cks::cpu
{
    enum class Vendor
    {
        Unknown = 0,
        Intel,
        AMD
    };

    struct Info
    {
        // ------------------ common info ------------------
        Vendor   vendor                     = Vendor::Unknown;
        char     vendor_name[13]            = {};
        unsigned logical_cores              = 0;
        unsigned physical_cores             = 0;

        unsigned hyper_threads          : 1 = 0;

        // ------------------ x86 features ------------------
        unsigned fxsr                   : 1 = 0;

        // SSE family
        unsigned sse                    : 1 = 0;
        unsigned sse2                   : 1 = 0;
        unsigned sse3                   : 1 = 0;
        unsigned ssse3                  : 1 = 0;
        unsigned sse4_1                 : 1 = 0;
        unsigned sse4_2                 : 1 = 0;

        // XSAVE & OS_XSAVE
        unsigned xsave                  : 1 = 0;
        unsigned os_xsave               : 1 = 0;

        // AVX family
        unsigned avx                    : 1 = 0;
        unsigned f16c                   : 1 = 0;
        unsigned fma3                   : 1 = 0;
        unsigned avx2                   : 1 = 0;
        unsigned avx_vnni               : 1 = 0;
        unsigned avx_vnni_int8          : 1 = 0;
        unsigned avx_ne_convert         : 1 = 0;
        unsigned avx_ifma               : 1 = 0;
        unsigned avx_vnni_int16         : 1 = 0;
        unsigned sha512                 : 1 = 0;
        unsigned sm3                    : 1 = 0;
        unsigned sm4                    : 1 = 0;

        // AVX-512 family
        unsigned avx512_f               : 1 = 0;
        unsigned avx512_bw              : 1 = 0;
        unsigned avx512_cd              : 1 = 0;
        unsigned avx512_dq              : 1 = 0;
        unsigned avx512_ifma            : 1 = 0;
        unsigned avx512_vl              : 1 = 0;
        unsigned avx512_vpopcntdq       : 1 = 0;
        unsigned avx512_bf16            : 1 = 0;
        unsigned avx512_bitalg          : 1 = 0;
        unsigned avx512_vbmi            : 1 = 0;
        unsigned avx512_vbmi2           : 1 = 0;
        unsigned avx512_vnni            : 1 = 0;
        unsigned avx512_vp2intersect    : 1 = 0;
        unsigned avx512_fp16            : 1 = 0;

        // other
        unsigned popcnt                 : 1 = 0;
        unsigned aes_ni                 : 1 = 0;
        unsigned sha                    : 1 = 0;

        // ------------------ arm features ------------------
        unsigned neon                   : 1 = 0;
        unsigned sve                    : 1 = 0;
        unsigned arm_crc32              : 1 = 0;
        unsigned arm_sha2               : 1 = 0;
    };

    Info info() noexcept;

    const Info& get_singleton_info() noexcept;
}
