#include "hex.h"
#include <cctype>
#include <cstdio>

namespace ot_mini {

static int HexCharToNibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool HexStringToBytes(const std::string &aHex, uint8_t *aBytes, size_t aMaxBytes, size_t &aByteLen)
{
    bool ok = true;

    aByteLen = 0;

    // Hex string çift sayıda karakter olmalı (her byte = 2 hex karakter)
    if (aHex.size() % 2 != 0)
    {
        ok = false;
    }
    else if (aHex.size() / 2 > aMaxBytes)
    {
        ok = false; // buffer'a sığmıyor
    }
    else
    {
        for (size_t i = 0; i < aHex.size(); i += 2)
        {
            int hi = HexCharToNibble(aHex[i]);
            int lo = HexCharToNibble(aHex[i + 1]);

            if (hi < 0 || lo < 0)
            {
                ok = false;
                break;
            }

            aBytes[aByteLen++] = static_cast<uint8_t>((hi << 4) | lo);
        }
    }

    return ok;
}

std::string BytesToHexString(const uint8_t *aBytes, size_t aLength)
{
    static const char kHexChars[] = "0123456789abcdef";
    std::string        result;

    result.reserve(aLength * 2);

    for (size_t i = 0; i < aLength; i++)
    {
        result.push_back(kHexChars[aBytes[i] >> 4]);
        result.push_back(kHexChars[aBytes[i] & 0x0f]);
    }

    return result;
}

} // namespace ot_mini
