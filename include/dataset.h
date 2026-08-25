#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include "tlv.h"

namespace ot_mini {

// Gerçek OpenThread'de bu sınır OT_OPERATIONAL_DATASET_MAX_LENGTH = 254'tür.
constexpr size_t kMaxDatasetSize = 254;

// ---------------------------------------------------------------------------
// Dataset sınıfı
//
// Gerçek OpenThread'de bu, src/core/meshcop/dataset.cpp'deki ot::MeshCoP::Dataset
// sınıfının karşılığı. Görevi TEK: ham TLV baytlarından oluşan bir buffer'ı
// yönetmek (bulma / ekleme / güncelleme / silme). Bu sınıf hiçbir zaman
// "ağ", "radyo" ya da "CLI" bilmez - sadece byte dizisiyle uğraşır.
// ---------------------------------------------------------------------------
class Dataset
{
public:
    Dataset() : mLength(0) {}

    void Clear() { mLength = 0; }

    uint8_t        GetLength() const { return mLength; }
    const uint8_t *GetBytes() const { return mBytes; }
    void           SetBytes(const uint8_t *aBytes, uint8_t aLength);

    // --- Düşük seviye TLV işlemleri (gerçek koddaki FindTlv / WriteTlv'nin karşılığı) ---
    bool FindTlv(TlvType aType, const uint8_t *&aValue, uint8_t &aLength) const;
    void SetTlv(TlvType aType, const uint8_t *aValue, uint8_t aLength); // varsa günceller, yoksa ekler
    void RemoveTlv(TlvType aType);

    // --- Alan bazlı get/set (gerçek koddaki Get<kChannel>() / Write<ChannelTlv>() eşleniği) ---
    bool GetChannel(uint16_t &aChannel) const;
    void SetChannel(uint16_t aChannel);

    bool GetPanId(uint16_t &aPanId) const;
    void SetPanId(uint16_t aPanId);

    bool GetExtPanId(uint8_t aExtPanId[8]) const;
    void SetExtPanId(const uint8_t aExtPanId[8]);

    bool GetNetworkName(std::string &aName) const;
    void SetNetworkName(const std::string &aName);

    bool GetNetworkKey(uint8_t aKey[16]) const;
    void SetNetworkKey(const uint8_t aKey[16]);

    bool GetMeshLocalPrefix(uint8_t aPrefix[8]) const;
    void SetMeshLocalPrefix(const uint8_t aPrefix[8]);

    bool GetPskc(uint8_t aPskc[16]) const;
    void SetPskc(const uint8_t aPskc[16]);

    bool GetActiveTimestamp(uint64_t &aTimestamp) const;
    void SetActiveTimestamp(uint64_t aTimestamp);

    // --- Yardımcılar ---
    void Print() const;        // "Channel: 15" gibi okunabilir çıktı
    void PrintTlvsHex() const; // ham TLV'leri hex string olarak yazdırır
    void GenerateRandom();     // `dataset init new`'in karşılığı

private:
    uint8_t mBytes[kMaxDatasetSize];
    uint8_t mLength;
};

} // namespace ot_mini
