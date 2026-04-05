#pragma once

#include "checksum/detail/base.h"

#ifdef _MSC_VER
#include <threads.h>

typedef once_flag cks_once_flag_t;
#define CKS_ONCE_FLAG_INIT ONCE_FLAG_INIT

#define cks_call_once(once_flag, func) \
    call_once(once_flag, func)


#elif CKS_COMPILER_GCC || CKS_COMPILER_CLANG
#include <pthread.h>

typedef pthread_once_t cks_once_flag_t;
#define CKS_ONCE_FLAG_INIT PTHREAD_ONCE_INIT

#define cks_call_once(once_flag, func) \
    pthread_once(once_flag, func)
#endif
