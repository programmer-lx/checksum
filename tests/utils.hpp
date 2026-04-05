#pragma once

#include <cstdint>
#include <string>

template<typename T>
static std::string to_hex(const T& block)
{
    const char hex_chars[] = "0123456789abcdef";
    constexpr size_t len = sizeof(T);
    std::string result;
    result.reserve(len * 2);

    const uint8_t* data = reinterpret_cast<const uint8_t*>(&block);
    
    for (size_t i = 0; i < len; ++i)
    {
        result += hex_chars[(data[i] >> 4) & 0xF];  // high 4bit
        result += hex_chars[data[i] & 0xF];         // low  4bit
    }
    return result;
}