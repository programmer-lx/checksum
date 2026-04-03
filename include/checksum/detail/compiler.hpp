#pragma once

// clang (must be before MSVC / GCC)
#define CKS_COMPILER_CLANG 0
#if defined(__clang__) && !defined(_MSC_VER)
    #undef CKS_COMPILER_CLANG
    #define CKS_COMPILER_CLANG 1
#endif

// clang-cl
#define CKS_COMPILER_CLANG_CL 0
#if defined(__clang__) && defined(_MSC_VER)
    #undef CKS_COMPILER_CLANG_CL
    #define CKS_COMPILER_CLANG_CL 1
#endif

// msvc
#define CKS_COMPILER_MSVC 0
#if defined(_MSC_VER) && !defined(__clang__)
    #undef CKS_COMPILER_MSVC
    #define CKS_COMPILER_MSVC 1
#endif

// gcc mingw
#define CKS_COMPILER_GCC 0
#if defined(__GNUC__) && !defined(__clang__)
    #undef CKS_COMPILER_GCC
    #define CKS_COMPILER_GCC 1
#endif

// mingw
#define CKS_COMPILER_MINGW 0
#if defined(__MINGW32__) || defined(__MINGW64__)
    #undef CKS_COMPILER_MINGW
    #define CKS_COMPILER_MINGW 1
#endif


// check
static_assert((CKS_COMPILER_CLANG + CKS_COMPILER_CLANG_CL + CKS_COMPILER_MSVC + (CKS_COMPILER_GCC || CKS_COMPILER_MINGW)) == 1,
    "Only one compiler macro can be defined.");
