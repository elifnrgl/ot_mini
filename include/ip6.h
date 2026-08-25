#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

namespace ot_mini {

// ---------------------------------------------------------------------------
// Ip6Address
//
// Gerçek OpenThread'deki otIp6Address / ot::Ip6::Address'in sadeleştirilmiş
// karşılığı: sadece 16 ham byte.
// ---------------------------------------------------------------------------
struct Ip6Address
{
    uint8_t mFields[16];

    // Gerçek `otIp6AddressToString` ile BİREBİR aynı davranış: RFC 5952 sıfır
    // sıkıştırması ("::") YAPMAZ. 8 grubun tamamını, her grubun başındaki
    // sıfırları atarak, ':' ile ayırıp yazar.
    // Ör: fdde:ad00:beef:0:0:ff:fe00:0
    //
    // (Prefix'lerin `::/64` ile yazdırılması -- dataset.cpp'de olduğu gibi --
    // ayrı bir formattır ve otIp6PrefixToString'e karşılık gelir; burada
    // KARIŞTIRILMAMASI için bilerek uygulanmadı.)
    std::string ToString() const;
};

// ---------------------------------------------------------------------------
// fe80::/64 + IID hesaplama
//
// Gerçek koddaki Mle::UpdateLinkLocalAddress()'in karşılığı. IID, genişletilmiş
// (extended/EUI-64) adresten, "modified EUI-64" kuralıyla (RFC 4291 Ek A)
// türetilir: adres aynen kopyalanır, sadece ilk byte'taki U/L (universal/local)
// bitinin (0x02) tersi alınır. Thread bunu OUI ekleme adımı olmadan, doğrudan
// 64-bit extended adres üzerinde uygular.
// ---------------------------------------------------------------------------
void ComputeLinkLocalAddress(const uint8_t aExtAddress[8], Ip6Address &aAddr);

// ---------------------------------------------------------------------------
// Mesh-Local EID (ML-EID) hesaplama: mesh-local prefix + rastgele/kalıcı IID.
// Gerçek koddaki ML-EID'in aksine burada U/L biti çevrilmez -- IID zaten
// rastgele üretilip cihazda saklanan bir değerdir (link-local'deki gibi bir
// donanım adresinden türetilmez).
// ---------------------------------------------------------------------------
void ComputeMlEid(const uint8_t aMeshLocalPrefix[8], const uint8_t aIid[8], Ip6Address &aAddr);

// ---------------------------------------------------------------------------
// "All Thread Nodes" multicast adresleri (mle.cpp'deki karşılığı).
// Biçim: ff3<scope>:0040:<mesh-local prefix (8 byte)>:0000:0001
// ---------------------------------------------------------------------------
enum class McastScope : uint8_t
{
    kLinkLocal  = 0x32, // ff32:40:.. -> Link-Local All Thread Nodes (LLATN)
    kRealmLocal = 0x33, // ff33:40:.. -> Realm-Local All Thread Nodes (RLATN)
};

void ComputeAllThreadNodesAddress(const uint8_t aMeshLocalPrefix[8], McastScope aScope, Ip6Address &aAddr);

// ---------------------------------------------------------------------------
// Metinden adres/prefix ayrıştırma ve prefix yazdırma
//
// Gerçek koddaki otIp6AddressFromString / otIp6PrefixFromString / otIp6PrefixToString'in
// karşılığı. NOT: Adres ("ipaddr", TLV value içi vb.) ve prefix yazdırma FARKLI davranır:
//  - Ip6Address::ToString() (yukarıda) hiç sıkıştırma yapmaz (otIp6AddressToString).
//  - Ip6PrefixToString() RFC 5952 sıkıştırması ("::") yapar (otIp6PrefixToString) --
//    bu ikisi bilerek ayrı tutuldu, gerçek kodda da öyle.
// ---------------------------------------------------------------------------

// "fd00:1234:5678::" gibi bir IPv6 adres string'ini ayrıştırır ("::" en fazla bir kez
// kullanılabilir). Başarılı olursa true döner.
bool ParseIp6Address(const std::string &aStr, Ip6Address &aAddr);

// İki adresin 16 baytının birebir aynı olup olmadığını söyler. Gerçek koddaki
// `Ip6::Address::operator==`'ün karşılığı - ping/udp/coap'ın "bu hedef adres
// benim kendi adreslerimden biri mi?" kontrolü için gerekli.
bool Ip6AddressEqual(const Ip6Address &aA, const Ip6Address &aB);

// "fd00:1234:5678::/64" gibi bir prefix string'ini adres + bit uzunluğuna ayrıştırır.
// Prefix uzunluğunu aşan baytlar sıfırlanır (gerçek otIp6PrefixFromString'deki gibi).
bool ParseIp6Prefix(const std::string &aStr, Ip6Address &aPrefix, uint8_t &aLength);

// RFC 5952 sıkıştırmalı ("::") prefix string'i üretir, ör: "fd00:1234:5678::/64".
std::string Ip6PrefixToString(const Ip6Address &aPrefix, uint8_t aLength);

} // namespace ot_mini
