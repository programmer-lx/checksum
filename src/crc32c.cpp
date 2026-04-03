#include "checksum/crc32c.hpp"

#include <array>
#include <cstring> // std::memcpy

#include "checksum/detail/arch.hpp"

#if CKS_ARCH_X86
#include <nmmintrin.h>
#endif

#if CKS_ARCH_ARM
#include <arm_acle.h>
#endif

#include "checksum/detail/cpu.hpp"

namespace cks
{
    namespace detail
    {
        consteval std::array<uint32_t, 256> make_crc32c_table()
        {
            constexpr uint32_t POLY = 0x82F63B78; // 反转后的多项式
            std::array<uint32_t, 256> table{};
            for (uint32_t i = 0; i < 256; i++)
            {
                uint32_t c = i;
                for (int j = 0; j < 8; j++)
                {
                    if (c & 1)
                        c = (c >> 1) ^ POLY;
                    else
                        c >>= 1;
                }
                table[i] = c;
            }
            return table;
        }

        auto crc32c_table = make_crc32c_table();

        CRC32C CKS_CALL_CONV crc32c_update_soft(CRC32C crc, const void* data, size_t size) noexcept
        {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);

            for (size_t i = 0; i < size; i++)
            {
                uint8_t index = (crc ^ bytes[i]) & 0xff;
                crc = (crc >> 8) ^ crc32c_table[index];
            }

            return crc;
        }

        #if CKS_ARCH_X86
        CKS_FUNC_ATTR_INTRINSICS_SSE4_2
        CRC32C CKS_CALL_CONV crc32c_update_sse42(CRC32C crc, const void* data, size_t size) noexcept
        {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);

            size_t i = 0;

            #if CKS_ARCH_X86_64
            for (; i + 8 <= size; i += 8)
            {
                uint64_t v;
                std::memcpy(&v, bytes + i, 8); // 避免未对齐 UB
                crc = static_cast<uint32_t>(_mm_crc32_u64(crc, v));
            }
            #endif

            for (; i + 4 <= size; i += 4)
            {
                uint32_t v;
                std::memcpy(&v, bytes + i, 4);
                crc = _mm_crc32_u32(crc, v);
            }

            for (; i < size; ++i)
            {
                crc = _mm_crc32_u8(crc, bytes[i]);
            }

            return crc;
        }
        #endif

        #if CKS_ARCH_ARM
        CRC32C CKS_CALL_CONV crc32c_update_arm(CRC32C crc, const void* data, size_t size) noexcept
        {
            const uint8_t* bytes = reinterpret_cast<const uint8_t*>(data);

            size_t i = 0;

            #if CKS_ARCH_ARM_64
            for (; i + 8 <= size; i += 8)
            {
                uint64_t v;
                std::memcpy(&v, bytes + i, 8); // 避免未对齐 UB
                crc = __crc32cd(crc, v);
            }
            #endif

            for (; i + 4 <= size; i += 4)
            {
                uint32_t v;
                std::memcpy(&v, bytes + i, 4);
                crc = __crc32cw(crc, v);
            }

            for (; i < size; ++i)
            {
                crc = __crc32cb(crc, bytes[i]);
            }

            return crc;
        }
        #endif
    }

    namespace
    {
        auto crc32c_fn() noexcept
        {
            static auto fn = []()
            {
                const cpu::Info& info = cpu::get_singleton_info();

                #if CKS_ARCH_X86
                if (info.sse4_2)
                {
                    return detail::crc32c_update_sse42;
                }
                #endif

                #if CKS_ARCH_ARM
                if (info.crc32)
                {
                    return detail::crc32c_update_arm;
                }
                #endif

                // fallback to software implementation
                return detail::crc32c_update_soft;
            }();
            return fn;
        }
    }

    CRC32C CKS_CALL_CONV crc32c_update(CRC32C crc, const void* data, size_t size) noexcept
    {
        return crc32c_fn()(crc, data, size);
    }
}