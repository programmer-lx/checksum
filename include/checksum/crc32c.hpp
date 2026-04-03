#pragma once

#include <cstdint>
#include <cstddef>

#include "checksum/detail/base.hpp"
#include "checksum/detail/arch.hpp"
#include "checksum/detail/attributes.hpp"

namespace cks
{
    using CRC32C = uint32_t;

    CKS_FORCE_INLINE CRC32C CKS_CALL_CONV crc32c_begin() noexcept
    {
        return UINT32_C(0xffffffff);
    }

    CKS_FORCE_INLINE CRC32C CKS_CALL_CONV crc32c_end(CRC32C crc) noexcept
    {
        return crc ^ UINT32_C(0xffffffff);
    }

    namespace detail
    {
        // software implementation
        CRC32C CKS_CALL_CONV crc32c_update_soft(CRC32C crc, const void* data, size_t size) noexcept;

        // x86 SSE4.2
        #if CKS_ARCH_X86
        CKS_FUNC_ATTR_INTRINSICS_SSE4_2
        CRC32C CKS_CALL_CONV crc32c_update_sse42(CRC32C crc, const void* data, size_t size) noexcept;
        #endif

        // arm CRC32 intrinsic
        #if CKS_ARCH_ARM
        CKS_FUNC_ATTR_INTRINSICS_ARM_CRC32
        CRC32C CKS_CALL_CONV crc32c_update_arm(CRC32C crc, const void* data, size_t size) noexcept;
        #endif
    }

    CRC32C CKS_CALL_CONV crc32c_update(CRC32C crc, const void* data, size_t size) noexcept;
}