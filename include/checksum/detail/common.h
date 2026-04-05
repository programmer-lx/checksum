#pragma once

#define CKS_STR_IMPL(x) #x
#define CKS_STR(x) CKS_STR_IMPL(x)

#define CKS_CONCAT_IMPL(a, b) a##b
#define CKS_CONCAT(a, b) CKS_CONCAT_IMPL(a, b)
