#pragma once

/* ---------------------------------- x86 ---------------------------------- */
/* 64-bit */
#define CKS_ARCH_X86_64 0
#if defined(_M_X64) || defined(__x86_64__)
    #undef CKS_ARCH_X86_64
    #define CKS_ARCH_X86_64 1
#endif

/* 32-bit */
#define CKS_ARCH_X86_32 0
#if defined(_M_IX86) || defined(__i386__)
    #undef CKS_ARCH_X86_32
    #define CKS_ARCH_X86_32 1
#endif

/* x86 any */
#define CKS_ARCH_X86 0
#if (CKS_ARCH_X86_32 || CKS_ARCH_X86_64)
    #undef CKS_ARCH_X86
    #define CKS_ARCH_X86 1
#endif


/* ---------------------------------- arm ---------------------------------- */
/* 64-bit ARM (ARM64 / AArch64) */
#define CKS_ARCH_ARM64 0
#if defined(_M_ARM64) || defined(__aarch64__)
    #undef CKS_ARCH_ARM64
    #define CKS_ARCH_ARM64 1
#endif

/* 32-bit ARM */
#define CKS_ARCH_ARM32 0
#if defined(_M_ARM) || defined(__arm__)
    #undef CKS_ARCH_ARM32
    #define CKS_ARCH_ARM32 1
#endif

/* ARM any */
#define CKS_ARCH_ARM 0
#if (CKS_ARCH_ARM32 || CKS_ARCH_ARM64)
    #undef CKS_ARCH_ARM
    #define CKS_ARCH_ARM 1
#endif


/* ---------------------------------- 32bits ---------------------------------- */
#define CKS_ARCH_32BIT 0
#if (CKS_ARCH_X86_32 || CKS_ARCH_ARM32)
    #undef CKS_ARCH_32BIT
    #define CKS_ARCH_32BIT 1
#endif

/* ---------------------------------- 64bits ---------------------------------- */
#define CKS_ARCH_64BIT 0
#if (CKS_ARCH_X86_64 || CKS_ARCH_ARM64)
    #undef CKS_ARCH_64BIT
    #define CKS_ARCH_64BIT 1
#endif


/* check */
#if !((CKS_ARCH_X86_64 + CKS_ARCH_X86_32 + CKS_ARCH_ARM64 + CKS_ARCH_ARM32) == 1)
    #error Exactly one CPU architecture can be defined
#endif

#if !((CKS_ARCH_32BIT + CKS_ARCH_64BIT) == 1)
    #error Exactly one bitness (32/64) macro can be defined
#endif
