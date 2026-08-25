#include "dataset.h"
#include "hex.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace ot_mini {

void Dataset::SetBytes(const uint8_t *aBytes, uint8_t aLength)
{
    mLength = aLength;
    std::memcpy(mBytes, aBytes, aLength);
}

// ---------------------------------------------------------------------------
// Düşük seviye TLV işlemleri
// ---------------------------------------------------------------------------

bool Dataset::FindTlv(TlvType aType, const uint8_t *&aValue, uint8_t &aLength) const
{
    bool   found  = false;
    size_t offset = 0;

    while (offset + kTlvHeaderSize <= mLength)
    {
        uint8_t type = mBytes[offset];
        uint8_t len  = mBytes[offset + 1];

        if (type == static_cast<uint8_t>(aType))
        {
            aValue  = &mBytes[offset + kTlvHeaderSize];
            aLength = len;
            found   = true;
            break;
        }

        offset += kTlvHeaderSize + len;
    }

    return found;
}

void Dataset::RemoveTlv(TlvType aType)
{
    size_t offset = 0;

    while (offset + kTlvHeaderSize <= mLength)
    {
        uint8_t type      = mBytes[offset];
        uint8_t len       = mBytes[offset + 1];
        size_t  entrySize = kTlvHeaderSize + len;

        if (type == static_cast<uint8_t>(aType))
        {
            size_t tailSize = mLength - (offset + entrySize);
            std::memmove(&mBytes[offset], &mBytes[offset + entrySize], tailSize);
            mLength -= static_cast<uint8_t>(entrySize);
            break; // dataset TLV'lerinde her tip yalnızca bir kez bulunur
        }

        offset += entrySize;
    }
}

void Dataset::SetTlv(TlvType aType, const uint8_t *aValue, uint8_t aLength)
{
    // NOT: Gerçek OpenThread var olan TLV'yi YERİNDE değiştirir (uzunluk aynıysa)
    // ya da onu silip yenisini uygun konuma ekler; sıralamayı büyük ölçüde korur.
    // Biz burada öğretici sadelik için "önce kaldır, sonra sona ekle" yaklaşımını
    // kullanıyoruz - sonuç (hangi TLV'lerin buffer'da olduğu) aynı, sadece TLV'lerin
    // buffer içindeki SIRASI gerçek koddaki gibi korunmuyor.
    RemoveTlv(aType);

    if (static_cast<size_t>(mLength) + kTlvHeaderSize + aLength <= kMaxDatasetSize)
    {
        mBytes[mLength++] = static_cast<uint8_t>(aType);
        mBytes[mLength++] = aLength;
        std::memcpy(&mBytes[mLength], aValue, aLength);
        mLength += aLength;
    }
}

// ---------------------------------------------------------------------------
// Alan bazlı get/set - her biri gerçek koddaki bir TLV tipine karşılık gelir
// ---------------------------------------------------------------------------

bool Dataset::GetChannel(uint16_t &aChannel) const
{
    const uint8_t *value;
    uint8_t        length;
    bool           found = FindTlv(TlvType::kChannel, value, length);

    if (found && length == 3) // [page:1][channel:2]
    {
        aChannel = static_cast<uint16_t>((value[1] << 8) | value[2]);
    }

    return found;
}

void Dataset::SetChannel(uint16_t aChannel)
{
    uint8_t buf[3] = {0, static_cast<uint8_t>(aChannel >> 8), static_cast<uint8_t>(aChannel & 0xff)};
    SetTlv(TlvType::kChannel, buf, sizeof(buf));
}

bool Dataset::GetPanId(uint16_t &aPanId) const
{
    const uint8_t *value;
    uint8_t        length;
    bool           found = FindTlv(TlvType::kPanId, value, length);

    if (found && length == 2)
    {
        aPanId = static_cast<uint16_t>((value[0] << 8) | value[1]);
    }

    return found;
}

void Dataset::SetPanId(uint16_t aPanId)
{
    uint8_t buf[2] = {static_cast<uint8_t>(aPanId >> 8), static_cast<uint8_t>(aPanId & 0xff)};
    SetTlv(TlvType::kPanId, buf, sizeof(buf));
}

bool Dataset::GetExtPanId(uint8_t aExtPanId[8]) const
{
    const uint8_t *value;
    uint8_t        length;
    bool           found = FindTlv(TlvType::kExtPanId, value, length);

    if (found && length == 8)
    {
        std::memcpy(aExtPanId, value, 8);
    }

    return found;
}

void Dataset::SetExtPanId(const uint8_t aExtPanId[8]) { SetTlv(TlvType::kExtPanId, aExtPanId, 8); }

bool Dataset::GetNetworkName(std::string &aName) const
{
    const uint8_t *value;
    uint8_t        length;
    bool           found = FindTlv(TlvType::kNetworkName, value, length);

    if (found)
    {
        aName.assign(reinterpret_cast<const char *>(value), length);
    }

    return found;
}

void Dataset::SetNetworkName(const std::string &aName)
{
    uint8_t length = static_cast<uint8_t>(aName.size() > 16 ? 16 : aName.size());
    SetTlv(TlvType::kNetworkName, reinterpret_cast<const uint8_t *>(aName.data()), length);
}

bool Dataset::GetNetworkKey(uint8_t aKey[16]) const
{
    const uint8_t *value;
    uint8_t        length;
    bool           found = FindTlv(TlvType::kNetworkKey, value, length);

    if (found && length == 16)
    {
        std::memcpy(aKey, value, 16);
    }

    return found;
}

void Dataset::SetNetworkKey(const uint8_t aKey[16]) { SetTlv(TlvType::kNetworkKey, aKey, 16); }

bool Dataset::GetMeshLocalPrefix(uint8_t aPrefix[8]) const
{
    const uint8_t *value;
    uint8_t        length;
    bool           found = FindTlv(TlvType::kMeshLocalPrefix, value, length);

    if (found && length == 8)
    {
        std::memcpy(aPrefix, value, 8);
    }

    return found;
}

void Dataset::SetMeshLocalPrefix(const uint8_t aPrefix[8]) { SetTlv(TlvType::kMeshLocalPrefix, aPrefix, 8); }

bool Dataset::GetPskc(uint8_t aPskc[16]) const
{
    const uint8_t *value;
    uint8_t        length;
    bool           found = FindTlv(TlvType::kPskc, value, length);

    if (found && length == 16)
    {
        std::memcpy(aPskc, value, 16);
    }

    return found;
}

void Dataset::SetPskc(const uint8_t aPskc[16]) { SetTlv(TlvType::kPskc, aPskc, 16); }

bool Dataset::GetActiveTimestamp(uint64_t &aTimestamp) const
{
    const uint8_t *value;
    uint8_t        length;
    bool           found = FindTlv(TlvType::kActiveTimestamp, value, length);

    if (found && length == 8)
    {
        aTimestamp = 0;
        for (int i = 0; i < 8; i++)
        {
            aTimestamp = (aTimestamp << 8) | value[i];
        }
    }

    return found;
}

void Dataset::SetActiveTimestamp(uint64_t aTimestamp)
{
    uint8_t buf[8];
    for (int i = 0; i < 8; i++)
    {
        buf[i] = static_cast<uint8_t>(aTimestamp >> (56 - i * 8));
    }
    SetTlv(TlvType::kActiveTimestamp, buf, sizeof(buf));
}

// ---------------------------------------------------------------------------
// Yazdırma / rastgele üretim
// ---------------------------------------------------------------------------

void Dataset::Print() const
{
    uint64_t    ts;
    uint16_t    channel, panId;
    uint8_t     extPanId[8], key[16], prefix[8], pskc[16];
    std::string name;

    if (GetActiveTimestamp(ts))
        std::printf("Active Timestamp: %llu\n", static_cast<unsigned long long>(ts));

    if (GetChannel(channel))
        std::printf("Channel: %u\n", channel);

    if (GetPanId(panId))
        std::printf("PAN ID: 0x%04x\n", panId);

    if (GetExtPanId(extPanId))
        std::printf("Ext PAN ID: %s\n", BytesToHexString(extPanId, 8).c_str());

    if (GetNetworkName(name))
        std::printf("Network Name: %s\n", name.c_str());

    if (GetNetworkKey(key))
        std::printf("Network Key: %s\n", BytesToHexString(key, 16).c_str());

    if (GetMeshLocalPrefix(prefix))
    {
        std::printf("Mesh Local Prefix: %02x%02x:%02x%02x:%02x%02x:%02x%02x::/64\n", prefix[0], prefix[1],
                     prefix[2], prefix[3], prefix[4], prefix[5], prefix[6], prefix[7]);
    }

    if (GetPskc(pskc))
        std::printf("PSKc: %s\n", BytesToHexString(pskc, 16).c_str());
}

void Dataset::PrintTlvsHex() const { std::printf("%s\n", BytesToHexString(mBytes, mLength).c_str()); }

void Dataset::GenerateRandom()
{
    Clear();

    std::srand(static_cast<unsigned>(std::time(nullptr)) ^ static_cast<unsigned>(reinterpret_cast<uintptr_t>(this)));

    auto randByte = []() -> uint8_t { return static_cast<uint8_t>(std::rand() & 0xff); };

    // Thread'in 2.4 GHz bandında kullanılabilir kanallar: 11-26
    SetChannel(static_cast<uint16_t>(11 + (std::rand() % 16)));

    uint16_t panId;
    do
    {
        panId = static_cast<uint16_t>(std::rand() & 0xffff);
    } while (panId == 0xffff); // 0xffff broadcast/ayrılmış değer, kullanılmaz

    SetPanId(panId);

    uint8_t extPanId[8];
    for (auto &b : extPanId) b = randByte();
    SetExtPanId(extPanId);

    uint8_t prefix[8];
    for (auto &b : prefix) b = randByte();
    prefix[0] = 0xfd; // ULA (Unique Local Address) öneki - gerçek kod da bunu zorunlu kılar
    SetMeshLocalPrefix(prefix);

    uint8_t key[16];
    for (auto &b : key) b = randByte();
    SetNetworkKey(key);

    uint8_t pskcBuf[16];
    for (auto &b : pskcBuf) b = randByte();
    SetPskc(pskcBuf);

    char nameBuf[32];
    std::snprintf(nameBuf, sizeof(nameBuf), "OpenThread-%04x", std::rand() & 0xffff);
    SetNetworkName(nameBuf);

    SetActiveTimestamp(1); // gerçek koddaki gibi varsayılan başlangıç değeri
}

} // namespace ot_mini
