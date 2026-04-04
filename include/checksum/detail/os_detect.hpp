#pragma once

#define CKS_OS_WINDOWS 0
#if defined(_WIN32) || defined(_WIN64)
    #undef CKS_OS_WINDOWS
    #define CKS_OS_WINDOWS 1
#endif

#define CKS_OS_MACOS 0
#if defined(__APPLE__) && defined(__MACH__)
    #undef CKS_OS_MACOS
    #define CKS_OS_MACOS 1
#endif

#define CKS_OS_LINUX 0
#if defined(__linux__)
    #undef CKS_OS_LINUX
    #define CKS_OS_LINUX 1
#endif


// check
static_assert((CKS_OS_WINDOWS + CKS_OS_MACOS + CKS_OS_LINUX) == 1,
    "Only one OS macro can be defined.");
