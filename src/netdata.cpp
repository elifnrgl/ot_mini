#include "netdata.h"
#include "hex.h"
#include <cstdio>
#include <cstring>

namespace ot_mini {

namespace {

// Gercek koddaki Mle::kInvalidRloc16 (0xfffe) - "gecerli/atanmis bir RLOC16
// degil" anlamina gelen sentinel deger. ot_mini hicbir zaman gercek bir Thread
// mesh'e attach olmadigindan, tum netdata girdileri bu placeholder'i kullanir.
constexpr uint16_t kPlaceholderRloc16 = 0xfffe;

void AppendU8(std::vector<uint8_t> &aOut, uint8_t aByte) { aOut.push_back(aByte); }

void AppendU16BE(std::vector<uint8_t> &aOut, uint16_t aValue)
{
    aOut.push_back(static_cast<uint8_t>(aValue >> 8));
    aOut.push_back(static_cast<uint8_t>(aValue & 0xff));
}

// Thread Network Data TLV tip numaralari (spec Table "Network Data TLV Types").
enum NetDataTlvType : uint8_t
{
    kTlvHasRoute     = 0,
    kTlvPrefix       = 1,
    kTlvBorderRouter = 2,
};

} // namespace

void NetData::PublishEntry(const NetDataEntry &aEntry)
{
    for (auto &e : mEntries)
    {
        if (e.mIsRoute == aEntry.mIsRoute && e.mPrefixLength == aEntry.mPrefixLength &&
            std::memcmp(e.mPrefix.mFields, aEntry.mPrefix.mFields, sizeof(e.mPrefix.mFields)) == 0)
        {
            e = aEntry; // ayni kayit -> uzerine yaz (gercek Publisher "replace" davranisi)
            return;
        }
    }

    mEntries.push_back(aEntry);
}

size_t NetData::Unpublish(const Ip6Address &aPrefix, uint8_t aLength)
{
    size_t removed = 0;

    for (size_t i = 0; i < mEntries.size();)
    {
        if (mEntries[i].mPrefixLength == aLength &&
            std::memcmp(mEntries[i].mPrefix.mFields, aPrefix.mFields, sizeof(aPrefix.mFields)) == 0)
        {
            mEntries.erase(mEntries.begin() + static_cast<long>(i));
            removed++;
        }
        else
        {
            i++;
        }
    }

    return removed;
}

static std::string BuildPrefixFlags(const NetDataEntry &e)
{
    std::string s;
    if (e.mPreferred) s += 'p';
    if (e.mSlaac) s += 'a';
    if (e.mDhcp) s += 'd';
    if (e.mConfigure) s += 'c';
    if (e.mDefaultRoute) s += 'r';
    if (e.mOnMesh) s += 'o';
    if (e.mStable) s += 's';
    if (e.mNdDns) s += 'n';
    return s;
}

static std::string BuildRouteFlags(const NetDataEntry &e)
{
    std::string s;
    if (e.mStable) s += 's';
    if (e.mNat64) s += 'n';
    if (e.mAdvertisingPio) s += 'a';
    return s;
}

void NetData::Print() const
{
    std::printf("Prefixes:\n");
    for (const auto &e : mEntries)
    {
        if (e.mIsRoute) continue;
        std::printf("%s %s %s %04x\n", Ip6PrefixToString(e.mPrefix, e.mPrefixLength).c_str(),
                     BuildPrefixFlags(e).c_str(), PreferenceToString(e.mPreference).c_str(), kPlaceholderRloc16);
    }

    std::printf("Routes:\n");
    for (const auto &e : mEntries)
    {
        if (!e.mIsRoute) continue;
        std::printf("%s %s %s %04x\n", Ip6PrefixToString(e.mPrefix, e.mPrefixLength).c_str(),
                     BuildRouteFlags(e).c_str(), PreferenceToString(e.mPreference).c_str(), kPlaceholderRloc16);
    }

    // ot_mini bir Servis Yayinlayicisi (netdata publish service) desteklemiyor;
    // bu bolum gercek ciktiyla ayni sekilde her zaman bos yazdirilir.
    std::printf("Services:\n");
}

std::vector<uint8_t> NetData::ToTlvBytes() const
{
    // Ayni (prefix, length) ciftine sahip girdileri tek bir Prefix TLV altinda
    // grupla (gercek kodda da bir prefix icin en fazla bir Prefix TLV olur,
    // BorderRouter/HasRoute alt-TLV'leri onun icine yerlesir).
    struct Group
    {
        Ip6Address prefix;
        uint8_t    length;
        const NetDataEntry *onMesh = nullptr;
        const NetDataEntry *route  = nullptr;
    };

    std::vector<Group> groups;

    for (const auto &e : mEntries)
    {
        Group *g = nullptr;
        for (auto &existing : groups)
        {
            if (existing.length == e.mPrefixLength &&
                std::memcmp(existing.prefix.mFields, e.mPrefix.mFields, sizeof(e.mPrefix.mFields)) == 0)
            {
                g = &existing;
                break;
            }
        }
        if (!g)
        {
            groups.push_back(Group{e.mPrefix, e.mPrefixLength, nullptr, nullptr});
            g = &groups.back();
        }

        if (e.mIsRoute)
            g->route = &e;
        else
            g->onMesh = &e;
    }

    std::vector<uint8_t> out;

    for (const auto &g : groups)
    {
        bool stable = (g.onMesh && g.onMesh->mStable) || (g.route && g.route->mStable);

        std::vector<uint8_t> value;
        AppendU8(value, 0); // Domain ID (ot_mini her zaman varsayilan domain=0 kullanir)
        AppendU8(value, g.length);

        uint8_t prefixBytes = static_cast<uint8_t>((g.length + 7) / 8);
        for (uint8_t i = 0; i < prefixBytes; i++) value.push_back(g.prefix.mFields[i]);

        if (g.onMesh)
        {
            std::vector<uint8_t> sub;
            AppendU16BE(sub, kPlaceholderRloc16);

            uint16_t flags = 0;
            if (g.onMesh->mPreferred) flags |= (1u << 15);
            if (g.onMesh->mSlaac) flags |= (1u << 14);
            if (g.onMesh->mDhcp) flags |= (1u << 13);
            if (g.onMesh->mConfigure) flags |= (1u << 12);
            if (g.onMesh->mDefaultRoute) flags |= (1u << 11);
            if (g.onMesh->mOnMesh) flags |= (1u << 10);
            if (g.onMesh->mNdDns) flags |= (1u << 9);
            flags |= static_cast<uint16_t>((g.onMesh->mPreference & 0x3) << 4);
            AppendU16BE(sub, flags);

            uint8_t subType = static_cast<uint8_t>((kTlvBorderRouter << 1) | (g.onMesh->mStable ? 1 : 0));
            AppendU8(value, subType);
            AppendU8(value, static_cast<uint8_t>(sub.size()));
            value.insert(value.end(), sub.begin(), sub.end());
        }

        if (g.route)
        {
            std::vector<uint8_t> sub;
            AppendU16BE(sub, kPlaceholderRloc16);

            uint8_t pref = static_cast<uint8_t>((g.route->mPreference & 0x3) << 6);
            if (g.route->mNat64) pref |= (1u << 5);
            if (g.route->mAdvertisingPio) pref |= (1u << 4);
            AppendU8(sub, pref);

            uint8_t subType = static_cast<uint8_t>((kTlvHasRoute << 1) | (g.route->mStable ? 1 : 0));
            AppendU8(value, subType);
            AppendU8(value, static_cast<uint8_t>(sub.size()));
            value.insert(value.end(), sub.begin(), sub.end());
        }

        uint8_t topType = static_cast<uint8_t>((kTlvPrefix << 1) | (stable ? 1 : 0));
        AppendU8(out, topType);
        AppendU8(out, static_cast<uint8_t>(value.size()));
        out.insert(out.end(), value.begin(), value.end());
    }

    return out;
}

std::string NetData::ToHexTlvs() const
{
    std::vector<uint8_t> bytes = ToTlvBytes();
    return BytesToHexString(bytes.data(), bytes.size());
}

bool ParseNetDataFlags(const std::string &aFlags, bool aIsRoute, NetDataEntry &aEntry)
{
    for (char c : aFlags)
    {
        if (aIsRoute)
        {
            switch (c)
            {
            case 's': aEntry.mStable = true; break;
            case 'n': aEntry.mNat64 = true; break;
            case 'a': aEntry.mAdvertisingPio = true; break;
            default: return false;
            }
        }
        else
        {
            switch (c)
            {
            case 'p': aEntry.mPreferred = true; break;
            case 'a': aEntry.mSlaac = true; break;
            case 'd': aEntry.mDhcp = true; break;
            case 'c': aEntry.mConfigure = true; break;
            case 'r': aEntry.mDefaultRoute = true; break;
            case 'o': aEntry.mOnMesh = true; break;
            case 's': aEntry.mStable = true; break;
            case 'n': aEntry.mNdDns = true; break;
            default: return false;
            }
        }
    }

    return true;
}

bool ParsePreference(const std::string &aStr, int8_t &aPreference)
{
    if (aStr.empty() || aStr == "med") { aPreference = 0; return true; }
    if (aStr == "high") { aPreference = 1; return true; }
    if (aStr == "low") { aPreference = -1; return true; }
    return false;
}

std::string PreferenceToString(int8_t aPreference)
{
    if (aPreference > 0) return "high";
    if (aPreference < 0) return "low";
    return "med";
}

} // namespace ot_mini
