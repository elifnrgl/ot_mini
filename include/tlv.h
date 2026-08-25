#pragma once
#include <cstdint>
#include <cstddef>

namespace ot_mini {

// Thread MeshCoP Dataset TLV tip numaraları.
// Bunlar gerçek OpenThread'deki (include/openthread/dataset.h -> otMeshcopTlvType)
// numaralarla BİREBİR aynı - yani bu programın ürettiği hex çıktıyı gerçek
// `ot-cli-ftd`'nin ürettiğiyle doğrudan karşılaştırabilirsin.
enum class TlvType : uint8_t
{
    kChannel         = 0,
    kPanId           = 1,
    kExtPanId        = 2,
    kNetworkName     = 3,
    kPskc            = 4,
    kNetworkKey      = 5,
    kMeshLocalPrefix = 7,
    kActiveTimestamp = 14,
};

// Dataset buffer'ının içindeki her bir TLV şu formatta diziliyor:
//   [ Type: 1 byte ][ Length: 1 byte ][ Value: Length byte ]
// Bu, gerçek OpenThread'deki ot::MeshCoP::Tlv sınıfının sadeleştirilmiş hali.
struct TlvHeader
{
    uint8_t mType;
    uint8_t mLength;
};

constexpr size_t kTlvHeaderSize = sizeof(TlvHeader); // 2 byte

} // namespace ot_mini
