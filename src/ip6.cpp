#include "ip6.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

namespace ot_mini {

std::string Ip6Address::ToString() const
{
    char buf[8 * 5]; // en fazla 8 grup x ("ffff:" = 5 karakter)
    int  pos = 0;

    for (int i = 0; i < 8; i++)
    {
        uint16_t group = static_cast<uint16_t>((mFields[i * 2] << 8) | mFields[i * 2 + 1]);

        pos += std::snprintf(&buf[pos], sizeof(buf) - pos, i == 0 ? "%x" : ":%x", group);
    }

    return std::string(buf, pos);
}

void ComputeLinkLocalAddress(const uint8_t aExtAddress[8], Ip6Address &aAddr)
{
    std::memset(aAddr.mFields, 0, sizeof(aAddr.mFields));

    aAddr.mFields[0] = 0xfe;
    aAddr.mFields[1] = 0x80;
    // mFields[2..7] = 0 (fe80::/64)

    std::memcpy(&aAddr.mFields[8], aExtAddress, 8);
    aAddr.mFields[8] ^= 0x02; // U/L bitini çevir (modified EUI-64, RFC 4291 Ek A)
}

void ComputeMlEid(const uint8_t aMeshLocalPrefix[8], const uint8_t aIid[8], Ip6Address &aAddr)
{
    std::memcpy(&aAddr.mFields[0], aMeshLocalPrefix, 8);
    std::memcpy(&aAddr.mFields[8], aIid, 8);
}

void ComputeAllThreadNodesAddress(const uint8_t aMeshLocalPrefix[8], McastScope aScope, Ip6Address &aAddr)
{
    std::memset(aAddr.mFields, 0, sizeof(aAddr.mFields));

    aAddr.mFields[0] = 0xff;
    aAddr.mFields[1] = static_cast<uint8_t>(aScope);
    aAddr.mFields[2] = 0x00;
    aAddr.mFields[3] = 0x40;

    std::memcpy(&aAddr.mFields[4], aMeshLocalPrefix, 8);

    // mFields[12..14] = 0
    aAddr.mFields[15] = 0x01;
}

// ---------------------------------------------------------------------------
// Metinden adres ayrıştırma
// ---------------------------------------------------------------------------

namespace {

// ':' ile ayrilmis grup listesine ayirir (bos string -> bos liste).
std::vector<std::string> SplitGroups(const std::string &aStr)
{
    std::vector<std::string> groups;

    if (aStr.empty()) return groups;

    size_t start = 0;

    while (true)
    {
        size_t pos = aStr.find(':', start);
        groups.push_back(aStr.substr(start, pos == std::string::npos ? std::string::npos : pos - start));
        if (pos == std::string::npos) break;
        start = pos + 1;
    }

    return groups;
}

bool ParseGroup(const std::string &aGroup, uint16_t &aValue)
{
    if (aGroup.empty() || aGroup.size() > 4) return false;

    char         *end;
    unsigned long v = std::strtoul(aGroup.c_str(), &end, 16);

    if (*end != '\0' || v > 0xffff) return false;

    aValue = static_cast<uint16_t>(v);
    return true;
}

} // namespace

bool Ip6AddressEqual(const Ip6Address &aA, const Ip6Address &aB)
{
    return std::memcmp(aA.mFields, aB.mFields, sizeof(aA.mFields)) == 0;
}

bool ParseIp6Address(const std::string &aStr, Ip6Address &aAddr)
{
    // "::" konumunu bul (en fazla bir kez gecerli).
    size_t doubleColon      = aStr.find("::");
    bool   hasDoubleColon   = doubleColon != std::string::npos;
    bool   ok               = true;

    std::vector<std::string> leftGroups, rightGroups;

    if (hasDoubleColon)
    {
        if (aStr.find("::", doubleColon + 1) != std::string::npos) return false; // birden fazla "::"

        std::string left  = aStr.substr(0, doubleColon);
        std::string right = aStr.substr(doubleColon + 2);

        leftGroups  = SplitGroups(left);
        rightGroups = SplitGroups(right);

        if (leftGroups.size() + rightGroups.size() >= 8) return false; // "::" en az bir grubu temsil etmeli
    }
    else
    {
        leftGroups = SplitGroups(aStr);
        if (leftGroups.size() != 8) return false;
    }

    std::memset(aAddr.mFields, 0, sizeof(aAddr.mFields));

    size_t idx = 0;

    for (const auto &g : leftGroups)
    {
        uint16_t v;
        if (!ParseGroup(g, v)) { ok = false; break; }
        aAddr.mFields[idx * 2]     = static_cast<uint8_t>(v >> 8);
        aAddr.mFields[idx * 2 + 1] = static_cast<uint8_t>(v & 0xff);
        idx++;
    }

    if (ok && hasDoubleColon)
    {
        idx = 8 - rightGroups.size();

        for (const auto &g : rightGroups)
        {
            uint16_t v;
            if (!ParseGroup(g, v)) { ok = false; break; }
            aAddr.mFields[idx * 2]     = static_cast<uint8_t>(v >> 8);
            aAddr.mFields[idx * 2 + 1] = static_cast<uint8_t>(v & 0xff);
            idx++;
        }
    }

    return ok;
}

bool ParseIp6Prefix(const std::string &aStr, Ip6Address &aPrefix, uint8_t &aLength)
{
    size_t slash = aStr.find('/');
    if (slash == std::string::npos) return false;

    std::string addrPart = aStr.substr(0, slash);
    std::string lenPart  = aStr.substr(slash + 1);

    if (!ParseIp6Address(addrPart, aPrefix)) return false;

    char         *end;
    unsigned long len = std::strtoul(lenPart.c_str(), &end, 10);
    if (*end != '\0' || len > 128) return false;

    aLength = static_cast<uint8_t>(len);

    // Prefix uzunluğunu aşan baytları sıfırla (gerçek otIp6PrefixFromString'deki gibi).
    for (size_t bit = aLength; bit < 128; bit++)
    {
        aPrefix.mFields[bit / 8] &= static_cast<uint8_t>(~(0x80 >> (bit % 8)));
    }

    return true;
}

std::string Ip6PrefixToString(const Ip6Address &aPrefix, uint8_t aLength)
{
    // 8 grubu hesapla.
    uint16_t groups[8];
    for (int i = 0; i < 8; i++)
    {
        groups[i] = static_cast<uint16_t>((aPrefix.mFields[i * 2] << 8) | aPrefix.mFields[i * 2 + 1]);
    }

    // RFC 5952: en uzun (>=2) ardışık sıfır grup dizisini bul.
    int bestStart = -1, bestLen = 0;
    int curStart  = -1, curLen  = 0;

    for (int i = 0; i < 8; i++)
    {
        if (groups[i] == 0)
        {
            if (curStart < 0) curStart = i;
            curLen++;
        }
        else
        {
            if (curLen > bestLen) { bestStart = curStart; bestLen = curLen; }
            curStart = -1;
            curLen   = 0;
        }
    }
    if (curLen > bestLen) { bestStart = curStart; bestLen = curLen; }
    if (bestLen < 2) { bestStart = -1; bestLen = 0; }

    std::string result;
    char        buf[8];

    for (int i = 0; i < 8;)
    {
        if (i == bestStart)
        {
            result += "::";
            i += bestLen;
            continue;
        }

        if (!result.empty() && result.back() != ':') result += ':';

        std::snprintf(buf, sizeof(buf), "%x", groups[i]);
        result += buf;
        i++;
    }

    if (result.empty()) result = "::";

    result += "/" + std::to_string(aLength);
    return result;
}

} // namespace ot_mini
