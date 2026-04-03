#include "checksum/detail/cpu.hpp"

#include <cstdint>
#include <cstring>

#include "checksum/detail/arch.hpp"

#if CKS_ARCH_X86
    #if defined(_MSC_VER)
        #include <intrin.h>
    #else
        #include <cpuid.h>
    #endif
    #include <emmintrin.h>
#endif

#include "checksum/detail/common.hpp"
#include "checksum/detail/compiler.hpp"
#include "checksum/detail/attributes.hpp"

namespace cks::cpu
{
    namespace
    {
        template<typename T>
        static constexpr bool bit_is_open(T value, T bit) noexcept
        {
            return (value & bit) != 0;
        }
    } // namespace

#if CKS_ARCH_X86
    namespace
    {
        namespace EAX1_ECX0_ECX
        {
            // see https://en.wikipedia.org/wiki/CPUID

            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SSE3                 = (UINT32_C(1) <<  0); // EAX 1 ECX 0, ECX  0
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SSSE3                = (UINT32_C(1) <<  9); // EAX 1 ECX 0, ECX  9
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t FMA3                 = (UINT32_C(1) << 12); // EAX 1 ECX 0, ECX 12
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SSE4_1               = (UINT32_C(1) << 19); // EAX 1 ECX 0, ECX 19
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SSE4_2               = (UINT32_C(1) << 20); // EAX 1 ECX 0, ECX 20
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t POPCNT               = (UINT32_C(1) << 23); // EAX 1 ECX 0, ECX 23
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AES_NI               = (UINT32_C(1) << 25); // EAX 1 ECX 0, ECX 25
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t XSAVE                = (UINT32_C(1) << 26); // EAX 1 ECX 0, ECX 26
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t OS_XSAVE             = (UINT32_C(1) << 27); // EAX 1 ECX 0, ECX 27
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX                  = (UINT32_C(1) << 28); // EAX 1 ECX 0, ECX 28
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t F16C                 = (UINT32_C(1) << 29); // EAX 1 ECX 0, ECX 29
        }
        
        namespace EAX1_ECX0_EDX
        {
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t FXSR                 = (UINT32_C(1) << 24); // EAX 1 ECX 0, EDX 24
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SSE                  = (UINT32_C(1) << 25); // EAX 1 ECX 0, EDX 25
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SSE2                 = (UINT32_C(1) << 26); // EAX 1 ECX 0, EDX 26
        }

        namespace EAX7_ECX0_EBX
        {
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX2                 = (UINT32_C(1) <<  5); // EAX 7 ECX 0, EBX  5
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_F             = (UINT32_C(1) << 16); // EAX 7 ECX 0, EBX 16
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_DQ            = (UINT32_C(1) << 17); // EAX 7 ECX 0, EBX 17
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_IFMA          = (UINT32_C(1) << 21); // EAX 7 ECX 0, EBX 21
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_CD            = (UINT32_C(1) << 28); // EAX 7 ECX 0, EBX 28
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SHA                  = (UINT32_C(1) << 29); // EAX 7 ECX 0, EBX 29
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_BW            = (UINT32_C(1) << 30); // EAX 7 ECX 0, EBX 30
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_VL            = (UINT32_C(1) << 31); // EAX 7 ECX 0, EBX 31
        }
        
        namespace EAX7_ECX0_ECX
        {
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_VBMI          = (UINT32_C(1) <<  1); // EAX 7 ECX 0, ECX  1
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_VBMI2         = (UINT32_C(1) <<  6); // EAX 7 ECX 0, ECX  6
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_VNNI          = (UINT32_C(1) << 11); // EAX 7 ECX 0, ECX 11
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_BITALG        = (UINT32_C(1) << 12); // EAX 7 ECX 0, ECX 12
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_VPOPCNTDQ     = (UINT32_C(1) << 14); // EAX 7 ECX 0, ECX 14
        }
        
        namespace EAX7_ECX0_EDX
        {
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_VP2INTERSECT  = (UINT32_C(1) <<  8); // EAX 7 ECX 0, EDX  8
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_FP16          = (UINT32_C(1) << 23); // EAX 7 ECX 0, EDX 23
        }

        namespace EAX7_ECX1_EAX
        {
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SHA512               = (UINT32_C(1) <<  0); // EAX 7 ECX 1, EAX  0
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SM3                  = (UINT32_C(1) <<  1); // EAX 7 ECX 1, EAX  1
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t SM4                  = (UINT32_C(1) <<  2); // EAX 7 ECX 1, EAX  2
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX_VNNI             = (UINT32_C(1) <<  4); // EAX 7 ECX 1, EAX  4
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX512_BF16          = (UINT32_C(1) <<  5); // EAX 7 ECX 1, EAX  5
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX_IFMA             = (UINT32_C(1) << 23); // EAX 7 ECX 1, EAX 23
        }
        
        namespace EAX7_ECX1_EDX
        {
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX_VNNI_INT8        = (UINT32_C(1) <<  4); // EAX 7 ECX 1, EDX  4
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX_NE_CONVERT       = (UINT32_C(1) <<  5); // EAX 7 ECX 1, EDX  5
            CKS_HEADER_GLOBAL_CONSTEXPR uint32_t AVX_VNNI_INT16       = (UINT32_C(1) << 10); // EAX 7 ECX 1, EDX 10
        }

        namespace XSAVE
        {
            // see https://en.wikipedia.org/wiki/CPUID XSAVE State-components

            CKS_HEADER_GLOBAL_CONSTEXPR uint64_t XMM                  = (UINT64_C(1) << 1); // XMM0-XMM15 and MXCSR
            CKS_HEADER_GLOBAL_CONSTEXPR uint64_t YMM                  = (UINT64_C(1) << 2); // YMM0-YMM15
            CKS_HEADER_GLOBAL_CONSTEXPR uint64_t K0_K7                = (UINT64_C(1) << 5); // opmask registers k0-k7
            CKS_HEADER_GLOBAL_CONSTEXPR uint64_t ZMM_LOW_256          = (UINT64_C(1) << 6); // ZMM0-ZMM15
            CKS_HEADER_GLOBAL_CONSTEXPR uint64_t ZMM_HIGH_256         = (UINT64_C(1) << 7); // ZMM16-ZMM31
        }

        // leaf: EAX, sub_leaf: ECX
        static void cpuid(const uint32_t leaf, const uint32_t sub_leaf, uint32_t abcd[4]) noexcept
        {
#if defined(_MSC_VER)
            int regs[4];
            __cpuidex(regs, static_cast<int>(leaf), static_cast<int>(sub_leaf));
            for (int i = 0; i < 4; ++i)
            {
                abcd[i] = static_cast<uint32_t>(regs[i]);
            }
#else
            uint32_t a;
            uint32_t b;
            uint32_t c;
            uint32_t d;
            __cpuid_count(leaf, sub_leaf, a, b, c, d);
            abcd[0] = a;
            abcd[1] = b;
            abcd[2] = c;
            abcd[3] = d;
#endif
        }

        static uint64_t xgetbv(uint32_t idx) noexcept
        {
#if defined(_MSC_VER)
            return _xgetbv(idx);
#else
            uint32_t eax, edx;
            __asm__ __volatile__ ("xgetbv" : "=a"(eax), "=d"(edx) : "c"(idx));
            return (static_cast<uint64_t>(edx) << 32) | eax;
#endif
        }
    } // namespace

    Info info() noexcept
    {
        Info result{};

        uint32_t abcd[4]; // eax, ebx, ecx, edx

        cpuid(0, 0, abcd);
        // 如果 max_leaf == 13，则可以调用 cpuid 查询的EAX的范围是 [0, 13]
        const uint32_t max_leaf = abcd[0];

        uint64_t xcr0 = 0;
        uint32_t eax1_ecx0_edx = 0; // EAX 1, ECX 0 的 EDX 值

        // ------------------ EAX 0 ------------------
        // if (max_leaf >= 0)
        {
            // vendor name
            const uint32_t ebx = abcd[1];
            const uint32_t ecx = abcd[2];
            const uint32_t edx = abcd[3];
            std::memcpy(result.vendor_name, &ebx, sizeof(uint32_t));
            std::memcpy(result.vendor_name + sizeof(uint32_t), &edx, sizeof(uint32_t));
            std::memcpy(result.vendor_name + 2 * sizeof(uint32_t), &ecx, sizeof(uint32_t));
            result.vendor_name[12] = 0;

            // vendor enum
            // Intel "GenuineIntel"
            if (ebx == 0x756e6547 && edx == 0x49656e69 && ecx == 0x6c65746e)
            {
                result.vendor = Vendor::Intel;
            }

            // AMD "AuthenticAMD"
            if (ebx == 0x68747541 && edx == 0x69746e65 && ecx == 0x444d4163)
            {
                result.vendor = Vendor::AMD;
            }
        }

        // ------------------ EAX 1 ------------------
        if (max_leaf >= 1)
        {
            // EAX 1, ECX 0
            cpuid(1, 0, abcd);
            const uint32_t ebx = abcd[1];
            const uint32_t ecx = abcd[2];
            const uint32_t edx = abcd[3];
            eax1_ecx0_edx = edx;

            result.logical_cores = (ebx >> 16) & 0xff; // EBX[23:16]

            // ------------------------- FXSR -------------------------
            result.fxsr = bit_is_open(edx, EAX1_ECX0_EDX::FXSR);

            // ------------------------- SSE family -------------------------
            result.sse = result.fxsr && bit_is_open(edx, EAX1_ECX0_EDX::SSE);
            result.sse2 = result.sse && bit_is_open(edx, EAX1_ECX0_EDX::SSE2);
            result.sse3 = result.sse2 && bit_is_open(ecx, EAX1_ECX0_ECX::SSE3);
            result.ssse3 = result.sse3 && bit_is_open(ecx, EAX1_ECX0_ECX::SSSE3);
            result.sse4_1 = result.ssse3 && bit_is_open(ecx, EAX1_ECX0_ECX::SSE4_1);
            result.sse4_2 = result.sse4_1 && bit_is_open(ecx, EAX1_ECX0_ECX::SSE4_2);

            // ------------------------- XSAVE -------------------------
            result.xsave = bit_is_open(ecx, EAX1_ECX0_ECX::XSAVE);
            result.os_xsave = bit_is_open(ecx, EAX1_ECX0_ECX::OS_XSAVE);
            // 只有在 xsave 和 os_xsave 为 true 的时候，才能进行 xgetbv 检查，AVX指令集才可用
            if (result.xsave && result.os_xsave)
            {
                xcr0 = xgetbv(0);
            }

            // ------------------------- AVX family -------------------------
            const bool os_support_avx = bit_is_open(xcr0, XSAVE::XMM) &&
                                        bit_is_open(xcr0, XSAVE::YMM);

            result.avx = result.sse4_1 && bit_is_open(ecx, EAX1_ECX0_ECX::AVX) && os_support_avx;
            result.f16c = result.avx && bit_is_open(ecx, EAX1_ECX0_ECX::F16C);
            result.fma3 = result.avx && bit_is_open(ecx, EAX1_ECX0_ECX::FMA3);

            // other
            result.aes_ni = bit_is_open(ecx, EAX1_ECX0_ECX::AES_NI);
            result.popcnt = bit_is_open(ecx, EAX1_ECX0_ECX::POPCNT);
        }

        // ------------------ EAX 4 ------------------
        if (max_leaf >= 4)
        {
            cpuid(4, 0, abcd);
            const uint32_t eax = abcd[0];

            if (result.vendor == Vendor::Intel)
            {
                result.physical_cores = ((eax >> 26) & 0x3f) + 1; // EAX[31:26] + 1
            }
        }

        // ------------------ EAX 7 ------------------
        if (max_leaf >= 7)
        {
            // EAX 7, ECX 0
            cpuid(7, 0, abcd);
            const uint32_t eax7_subleaf_count = abcd[0];
            {
                const uint32_t ebx = abcd[1];
                const uint32_t ecx = abcd[2];
                const uint32_t edx = abcd[3];

                result.avx2 = result.avx && bit_is_open(ebx, EAX7_ECX0_EBX::AVX2);


                // ------------------------- AVX-512 family -------------------------
                const bool os_support_avx_512 = bit_is_open(xcr0, XSAVE::XMM) &&
                                                bit_is_open(xcr0, XSAVE::YMM) &&
                                                bit_is_open(xcr0, XSAVE::K0_K7) &&
                                                bit_is_open(xcr0, XSAVE::ZMM_LOW_256) &&
                                                bit_is_open(xcr0, XSAVE::ZMM_HIGH_256);

                // ebx
                result.avx512_f = result.avx2 &&
                                    result.fma3 &&
                                    result.f16c &&
                                    bit_is_open(ebx, EAX7_ECX0_EBX::AVX512_F) &&
                                    os_support_avx_512;
                result.avx512_bw = result.avx512_f && bit_is_open(ebx, EAX7_ECX0_EBX::AVX512_BW);
                result.avx512_cd = result.avx512_f && bit_is_open(ebx, EAX7_ECX0_EBX::AVX512_CD);
                result.avx512_dq = result.avx512_f && bit_is_open(ebx, EAX7_ECX0_EBX::AVX512_DQ);
                result.avx512_ifma = result.avx512_f && bit_is_open(ebx, EAX7_ECX0_EBX::AVX512_IFMA);
                result.avx512_vl = result.avx512_f && bit_is_open(ebx, EAX7_ECX0_EBX::AVX512_VL);

                result.sha = bit_is_open(ebx, EAX7_ECX0_EBX::SHA);

                // ecx
                result.avx512_vpopcntdq = result.avx512_f && bit_is_open(ecx, EAX7_ECX0_ECX::AVX512_VPOPCNTDQ);
                result.avx512_bitalg = result.avx512_f && bit_is_open(ecx, EAX7_ECX0_ECX::AVX512_BITALG);
                result.avx512_vbmi = result.avx512_f && bit_is_open(ecx, EAX7_ECX0_ECX::AVX512_VBMI);
                result.avx512_vbmi2 = result.avx512_f && bit_is_open(ecx, EAX7_ECX0_ECX::AVX512_VBMI2);
                result.avx512_vnni = result.avx512_f && bit_is_open(ecx, EAX7_ECX0_ECX::AVX512_VNNI);

                // edx
                result.avx512_vp2intersect = result.avx512_f && bit_is_open(edx, EAX7_ECX0_EDX::AVX512_VP2INTERSECT);
                result.avx512_fp16 = result.avx512_f && bit_is_open(edx, EAX7_ECX0_EDX::AVX512_FP16);
            }

            // EAX 7 ECX 1
            if (eax7_subleaf_count >= 1)
            {
                cpuid(7, 1, abcd);
                const uint32_t eax = abcd[0];
                const uint32_t edx = abcd[3];

                // eax
                result.avx_vnni = result.avx2 && bit_is_open(eax, EAX7_ECX1_EAX::AVX_VNNI);
                result.avx_ifma = result.avx2 && bit_is_open(eax, EAX7_ECX1_EAX::AVX_IFMA);

                result.avx512_bf16 = result.avx512_f && bit_is_open(eax, EAX7_ECX1_EAX::AVX512_BF16);

                result.sha512 = result.avx2 && bit_is_open(eax, EAX7_ECX1_EAX::SHA512);
                result.sm3 = result.avx2 && bit_is_open(eax, EAX7_ECX1_EAX::SM3);
                result.sm4 = result.avx2 && bit_is_open(eax, EAX7_ECX1_EAX::SM4);

                // edx
                result.avx_vnni_int8 = result.avx2 && bit_is_open(edx, EAX7_ECX1_EDX::AVX_VNNI_INT8);
                result.avx_ne_convert = result.avx2 && bit_is_open(edx, EAX7_ECX1_EDX::AVX_NE_CONVERT);
                result.avx_vnni_int16 = result.avx2 && bit_is_open(edx, EAX7_ECX1_EDX::AVX_VNNI_INT16);
            }
        }

        // ------------------------------------ ext ------------------------------------
        cpuid(0x80000000, 0, abcd);
        const uint32_t max_ext_leaf = abcd[0];

        // ------------------ EAX 0x8000'0008 ------------------
        if (max_ext_leaf >= 0x80000008)
        {
            if (result.vendor == Vendor::AMD)
            {
                cpuid(0x80000008, 0, abcd);
                const uint32_t ecx = abcd[2];
                result.physical_cores = (ecx & 0xff) + 1; // ECX[7:0] + 1
            }
        }

        if (max_leaf >= 1)
        {
            result.hyper_threads = (eax1_ecx0_edx & (UINT32_C(1) << 28)) && (result.physical_cores < result.logical_cores);
        }

        return result;
    }
#elif CKS_ARCH_ARM

    Info info() noexcept
    {
        return {};
    }

#endif

    const Info& get_singleton_info() noexcept
    {
        static Info i = info();
        return i;
    }
}
