#pragma once

#include <stddef.h> /* size_t */
#include <stdint.h> /* uint8_t */

#if (defined(_WIN32) || defined(_WIN64)) /* Windows永远是小端序 */ \
    || defined(__LITTLE_ENDIAN__) \
    || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

    #define CKS_LITTLE_ENDIAN 1

#elif defined(__BIG_ENDIAN__) \
    || (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)

    #define CKS_LITTLE_ENDIAN 0

#endif

static inline void cks_reverse_bytes(void* data, size_t size)
{
    if (size <= 1)
    {
        return;
    }

    uint8_t* bytes = (uint8_t*)data;

    size_t i = 0;
    size_t j = size - 1;
    while (i < j)
    {
        uint8_t tmp = bytes[i];
        bytes[i] = bytes[j];
        bytes[j] = tmp;

        ++i;
        --j;
    }
}

static inline void cks_to_big_endian(void* data, size_t size)
{
#if CKS_LITTLE_ENDIAN
    cks_reverse_bytes(data, size);
#else
    (void)data;
    (void)size;
#endif
}

static inline void cks_to_little_endian(void* data, size_t size)
{
#if !CKS_LITTLE_ENDIAN
    cks_reverse_bytes(data, size);
#else
    (void)data;
    (void)size;
#endif
}
