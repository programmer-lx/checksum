#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define CKS_CALL_CONV __stdcall
#else
    #define CKS_CALL_CONV
#endif