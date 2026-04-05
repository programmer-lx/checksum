#pragma once

#include "common.h"
#include "arch.h"
#include "os_detect.h"
#include "compiler.h"
#include "attributes.h"

#define CKS_CALL_CONV

/* public API */
#if defined(CKS_BUILD_STATIC)
    #define CKS_API /* static library */
#elif defined(CKS_BUILD_DLL)
    #define CKS_API CKS_DLL_EXPORT /* shared library export */
#else
    #define CKS_API CKS_DLL_IMPORT /* shared library import */
#endif
