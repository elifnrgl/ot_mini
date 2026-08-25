#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "ip6.h"

namespace ot_mini {

// ---------------------------------------------------------------------------
// NetDataEntry
//
// Gerçek OpenThread'de bir cihazın "local" (henüz Leader'a register edilmemiş)
// Network Data Publisher girdisinin karşılığı: `netdata publish prefix` ya da
// `netdata publish route` ile eklenen tek bir on-mesh prefix ya da external
// route kaydı.
// ---------------------------------------------------------------------------
struct NetDataEntry
{
    Ip6Address mPrefix;
    uint8_t    mPrefixLength = 64;
    bool       mIsRoute      = false; // false: on-mesh prefix, true: external route
    bool       mStable       = false;
    int8_t     mPreference   = 0; // -1 low, 0 med, 1 high

    // On-mesh prefix bayrakları (yalnizca !mIsRoute icin anlamli).
    bool mPreferred = false, mSlaac = false, mDhcp = false, mConfigure = false, mDefaultRoute = false,
         mOnMesh = false, mNdDns = false;

    // External route bayraklari (yalnizca mIsRoute icin anlamli).
    bool mNat64 = false, mAdvertisingPio = false;
};

// ---------------------------------------------------------------------------
// NetData
//
// Gerçek koddaki "local Network Data" (Publisher'ın henüz Leader'a
// register etmediği kendi girdileri) kavramının karşılığı. Burada ne bir
// Leader ne de gerçek bir kayıt protokolü var; bu yüzden `netdata register`
// ve düz (Leader'dan gelen) `netdata show` desteklenmiyor - yalnızca
// `netdata show local` dürüstçe yeniden üretilebiliyor.
// ---------------------------------------------------------------------------
class NetData
{
public:
    void Clear() { mEntries.clear(); }

    // Ayni (prefix, length, isRoute) ucluye sahip bir girdi varsa uzerine yazar,
    // yoksa ekler (gercek Publisher'in "replace" davranisinin sadelesmis hali).
    void PublishEntry(const NetDataEntry &aEntry);

    // Verilen prefix/length'e sahip TUM girdileri (hem prefix hem route) kaldirir.
    // Kaldirilan girdi sayisini dondurur.
    size_t Unpublish(const Ip6Address &aPrefix, uint8_t aLength);

    const std::vector<NetDataEntry> &GetEntries() const { return mEntries; }
    bool                             IsEmpty() const { return mEntries.empty(); }

    // "Prefixes: / Routes: / Services:" bicimini yazdirir (gercek `netdata show`
    // ile ayni satir formatinda - bkz. README_NETDATA.md).
    void Print() const;

    // En iyi-caba TLV kodlamasi: Thread Network Data TLV genel bicimini
    // (Type<<1|Stable, Length, Value) ve Prefix/BorderRouter/HasRoute TLV
    // duzenini izler. UYARI: dataset TLV'lerinin aksine (ki onlarin tip
    // numaralari kamuya acik otMeshcopTlvType enum'undan birebir), buradaki
    // bayrak bit yerlesimi hafizadan yeniden kurulmus bir yaklasimdir ve
    // gercek bir OpenThread cihazinin ciktisiyla bayt-bayt ayni olacagi
    // GARANTI EDILMEZ - yalnizca kendi icinde tutarli, egitici bir modeldir.
    std::vector<uint8_t> ToTlvBytes() const;
    std::string          ToHexTlvs() const;

private:
    std::vector<NetDataEntry> mEntries;
};

// Bayrak harflerini ("paros" gibi) ayristirir. aIsRoute'e gore farkli harf
// kumeleri kabul edilir (prefix: p a d c r o s n, route: s n a).
bool ParseNetDataFlags(const std::string &aFlags, bool aIsRoute, NetDataEntry &aEntry);

// "high" | "med" | "low" -> +1 | 0 | -1. Bos string -> med (varsayilan).
bool ParsePreference(const std::string &aStr, int8_t &aPreference);
std::string PreferenceToString(int8_t aPreference);

} // namespace ot_mini
