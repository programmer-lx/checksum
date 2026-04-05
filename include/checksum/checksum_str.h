#pragma once

#include <stddef.h>
#include <stdint.h>

/*
str 的长度是 checksum_bytes 的两倍 + 1 (末尾有 '\0')
*/
static inline void cks_to_string(char* str, const void* checksum_mem, size_t checksum_bytes)
{
    const uint8_t* data = (const uint8_t*)checksum_mem;
    const char* hex_char = "0123456789abcdef";
    for (size_t i = 0; i < checksum_bytes; ++i)
    {
        // high 4bits
        str[i] = hex_char[(data[i] >> 4) & 0xf];

        // low 4bits
        str[i] = hex_char[data[i] & 0xf];
    }
}

/* checksum的str的长度(不包括末尾'\0') */
enum cks_HexCharCount
{
    CKS_HEX_CHAR_COUNT_CRC32C       = 8,
    CKS_HEX_CHAR_COUNT_MD5          = 32,
    CKS_HEX_CHAR_COUNT_SHA256       = 64,
    CKS_HEX_CHAR_COUNT_XXHASH3_64   = 16,
    CKS_HEX_CHAR_COUNT_XXHASH3_128  = 32
};
