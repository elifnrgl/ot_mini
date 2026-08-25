#pragma once
#include <cstdint>
#include "ip6.h"

namespace ot_mini {

// ---------------------------------------------------------------------------
// UdpSocket
//
// Gerçek OpenThread'deki `otUdpSocket`'in (bkz. CLI README_UDP.md'deki "the
// example socket") çok sadeleştirilmiş karşılığı. ot_mini tek-process/tek-
// düğüm olduğundan bu gerçek bir işletim sistemi soketi DEĞİLDİR - yalnızca
// bind/connect DURUMUNU tutar.
//
// SINIR: gerçek CLI örneğinde tek bir global soket vardır (bu yüzden Cli de
// tek bir UdpSocket üyesi tutuyor - src/cli/cli_udp.cpp'deki `sSocket`'in
// karşılığı). Gönderilen bir paket, ancak hedef adres+port bu SOKETİN kendi
// bind'ine denk düşüyorsa (ki bu da hedefin bu düğümün KENDİ adreslerinden
// biri olmasını gerektirir - bkz. cli_udp.cpp) "teslim edilmiş" sayılır.
// İki gerçek düğüm arası UDP için ayrı bir process + gerçek işletim sistemi
// soketi (127.0.0.1 üzerinden) gerekir; bu, ot_mini'nin kapsamı dışında.
// ---------------------------------------------------------------------------
struct UdpSocket
{
    bool mIsOpen = false;

    bool       mIsBound = false;
    Ip6Address mLocalAddr{}; // tüm alanlar 0 = "::" (unspecified, herhangi bir adrese bind)
    uint16_t   mLocalPort = 0;

    bool       mIsConnected = false;
    Ip6Address mPeerAddr{};
    uint16_t   mPeerPort = 0;
};

} // namespace ot_mini
