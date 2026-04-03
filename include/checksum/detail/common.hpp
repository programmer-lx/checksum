#pragma once

#define CKS_STR_IMPL(x) #x
#define CKS_STR(x) CKS_STR_IMPL(x)

#define CKS_CONCAT_IMPL(a, b) a##b
#define CKS_CONCAT(a, b) CKS_CONCAT_IMPL(a, b)

// Header-only 全局常量或 constexpr 函数 (防止误用 static constexpr 导致每个TU一份)
#define CKS_HEADER_GLOBAL_CONSTEXPR inline constexpr

// Header-only 全局变量或内联函数 (防止误用 static)
#define CKS_HEADER_GLOBAL inline
