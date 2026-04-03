#pragma once

#include "checksum/detail/compiler.hpp"

// inline | noinline | flatten
#if CKS_COMPILER_MSVC
    #define CKS_FORCE_INLINE      __forceinline
    #define CKS_FLATTEN
    #define CKS_NOINLINE          __declspec(noinline)
#elif CKS_COMPILER_GCC || CKS_COMPILER_CLANG || CKS_COMPILER_CLANG_CL
    #define CKS_FORCE_INLINE      inline __attribute__((always_inline))
    #define CKS_FLATTEN           __attribute__((flatten))
    #define CKS_NOINLINE          __attribute__((noinline))
#else
    #define CKS_FORCE_INLINE      inline
    #define CKS_FLATTEN
    #define CKS_NOINLINE
#endif

// restrict
#if CKS_COMPILER_MSVC
    #define CKS_RESTRICT __restrict
#elif CKS_COMPILER_GCC || CKS_COMPILER_CLANG || CKS_COMPILER_CLANG_CL
    #define CKS_RESTRICT __restrict__
#else
    #define CKS_RESTRICT
#endif

// dll import export
#if CKS_COMPILER_MSVC
    #define CKS_DLL_IMPORT        __declspec(dllimport)
    #define CKS_DLL_EXPORT        __declspec(dllexport)
    #define CKS_DLL_LOCAL
#else
    #if CKS_COMPILER_GCC || CKS_COMPILER_CLANG || CKS_COMPILER_CLANG_CL
        #define CKS_DLL_IMPORT    __attribute__((visibility("default")))
        #define CKS_DLL_EXPORT    __attribute__((visibility("default")))
        #define CKS_DLL_LOCAL     __attribute__((visibility("hidden")))
    #else
        #define CKS_DLL_IMPORT
        #define CKS_DLL_EXPORT
        #define CKS_DLL_LOCAL
    #endif
#endif

#ifdef __cplusplus
    #define CKS_EXTERN_C extern "C"
#else
    #define CKS_EXTERN_C extern
#endif

#define CKS_DLL_C_IMPORT CKS_EXTERN_C CKS_DLL_IMPORT
#define CKS_DLL_C_EXPORT CKS_EXTERN_C CKS_DLL_EXPORT


// pragma
#define CKS_DIAGNOSTICS_PUSH
#define CKS_DIAGNOSTICS_POP

#define CKS_IGNORE_WARNING_MSVC(warnings)
#define CKS_IGNORE_WARNING_GCC(warnings)
#define CKS_IGNORE_WARNING_CLANG(warnings)

#if CKS_COMPILER_MSVC // MSVC
    #define CKS_PRAGMA(tokens) __pragma(tokens)

    #undef CKS_DIAGNOSTICS_PUSH
    #define CKS_DIAGNOSTICS_PUSH CKS_PRAGMA(warning(push))

    #undef CKS_DIAGNOSTICS_POP
    #define CKS_DIAGNOSTICS_POP CKS_PRAGMA(warning(pop))

    #undef CKS_IGNORE_WARNING_MSVC
    #define CKS_IGNORE_WARNING_MSVC(warnings) CKS_PRAGMA(warning(disable : warnings))

#elif CKS_COMPILER_GCC // GCC
    #define CKS_PRAGMA(tokens) _Pragma(#tokens)
    #undef CKS_DIAGNOSTICS_PUSH
    #define CKS_DIAGNOSTICS_PUSH CKS_PRAGMA(GCC diagnostic push)

    #undef CKS_DIAGNOSTICS_POP
    #define CKS_DIAGNOSTICS_POP CKS_PRAGMA(GCC diagnostic pop)

    #undef CKS_IGNORE_WARNING_GCC
    #define CKS_IGNORE_WARNING_GCC(warnings) CKS_PRAGMA(GCC diagnostic ignored warnings)

#elif CKS_COMPILER_CLANG || CKS_COMPILER_CLANG_CL // clang / clang-cl
    #define CKS_PRAGMA(tokens) _Pragma(#tokens)

    #undef CKS_DIAGNOSTICS_PUSH
    #define CKS_DIAGNOSTICS_PUSH CKS_PRAGMA(clang diagnostic push)

    #undef CKS_DIAGNOSTICS_POP
    #define CKS_DIAGNOSTICS_POP CKS_PRAGMA(clang diagnostic pop)

    #undef CKS_IGNORE_WARNING_CLANG
    #define CKS_IGNORE_WARNING_CLANG(warnings) CKS_PRAGMA(clang diagnostic ignored warnings)
#endif

// packed struct
#if CKS_COMPILER_MSVC
    #define CKS_BEGIN_PACKED_STRUCT(name) __pragma(pack(push, 1)) struct name
    #define CKS_END_PACKED_STRUCT __pragma(pack(pop))
#elif CKS_COMPILER_GCC || CKS_COMPILER_CLANG || CKS_COMPILER_CLANG_CL
    #define CKS_BEGIN_PACKED_STRUCT(name) struct __attribute__((packed)) name
    #define CKS_END_PACKED_STRUCT
#else
    #error "Compiler not supported"
#endif


// function intrinsics attr
#define CKS_FUNC_ATTR_INTRINSICS_SSE
#define CKS_FUNC_ATTR_INTRINSICS_SSE2
#define CKS_FUNC_ATTR_INTRINSICS_SSE3
#define CKS_FUNC_ATTR_INTRINSICS_SSSE3
#define CKS_FUNC_ATTR_INTRINSICS_SSE4_1
#define CKS_FUNC_ATTR_INTRINSICS_SSE4_2

#define CKS_FUNC_ATTR_INTRINSICS_AVX
#define CKS_FUNC_ATTR_INTRINSICS_FMA3
#define CKS_FUNC_ATTR_INTRINSICS_F16C
#define CKS_FUNC_ATTR_INTRINSICS_AVX2

#define CKS_FUNC_ATTR_INTRINSICS_AVX512_F

#define CKS_FUNC_ATTR_INTRINSICS_ARM_CRC32

#if CKS_COMPILER_GCC || CKS_COMPILER_CLANG || CKS_COMPILER_CLANG_CL

    #undef CKS_FUNC_ATTR_INTRINSICS_SSE
    #define CKS_FUNC_ATTR_INTRINSICS_SSE __attribute__((target("sse")))

    #undef CKS_FUNC_ATTR_INTRINSICS_SSE2
    #define CKS_FUNC_ATTR_INTRINSICS_SSE2 __attribute__((target("sse2")))

    #undef CKS_FUNC_ATTR_INTRINSICS_SSE3
    #define CKS_FUNC_ATTR_INTRINSICS_SSE3 __attribute__((target("sse3")))

    #undef CKS_FUNC_ATTR_INTRINSICS_SSSE3
    #define CKS_FUNC_ATTR_INTRINSICS_SSSE3 __attribute__((target("ssse3")))

    #undef CKS_FUNC_ATTR_INTRINSICS_SSE4_1
    #define CKS_FUNC_ATTR_INTRINSICS_SSE4_1 __attribute__((target("sse4.1")))

    #undef CKS_FUNC_ATTR_INTRINSICS_SSE4_2
    #define CKS_FUNC_ATTR_INTRINSICS_SSE4_2 __attribute__((target("sse4.2")))

    #undef CKS_FUNC_ATTR_INTRINSICS_AVX
    #define CKS_FUNC_ATTR_INTRINSICS_AVX __attribute__((target("avx")))

    #undef CKS_FUNC_ATTR_INTRINSICS_FMA3
    #define CKS_FUNC_ATTR_INTRINSICS_FMA3 __attribute__((target("fma")))

    #undef CKS_FUNC_ATTR_INTRINSICS_F16C
    #define CKS_FUNC_ATTR_INTRINSICS_F16C __attribute__((target("f16c")))

    #undef CKS_FUNC_ATTR_INTRINSICS_AVX2
    #define CKS_FUNC_ATTR_INTRINSICS_AVX2 __attribute__((target("avx2")))

    #undef CKS_FUNC_ATTR_INTRINSICS_AVX512_F
    #define CKS_FUNC_ATTR_INTRINSICS_AVX512_F __attribute__((target("avx512f")))

    #undef CKS_FUNC_ATTR_INTRINSICS_ARM_CRC32
    #define CKS_FUNC_ATTR_INTRINSICS_ARM_CRC32 __attribute__((target("+crc")))
#endif
