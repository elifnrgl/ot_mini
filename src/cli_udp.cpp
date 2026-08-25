#include "cli_dataset.h"
#include "ip6.h"
#include <cstdio>
#include <cstdlib>

namespace ot_mini {

namespace {

bool ParsePort(const std::string &aStr, uint16_t &aPort)
{
    if (aStr.empty()) return false;

    char         *end;
    unsigned long v = std::strtoul(aStr.c_str(), &end, 10);
    if (*end != '\0' || v > 0xffff) return false;

    aPort = static_cast<uint16_t>(v);
    return true;
}

bool IsUnspecified(const Ip6Address &aAddr)
{
    for (uint8_t b : aAddr.mFields)
    {
        if (b != 0) return false;
    }
    return true;
}

uint16_t RandomEphemeralPort()
{
    // Gercek isletim sistemlerindeki gecici (ephemeral) port araligi: 49152-65535
    // (IANA "Dynamic and/or Private" araligi) - README_UDP.md ornek ciktilarindaki
    // "49153" gibi degerler de bu araliktan.
    return static_cast<uint16_t>(49152 + (std::rand() % (65535 - 49152 + 1)));
}

} // namespace

// ---------------------------------------------------------------------------
// udp - UDP soket ömrü (gerçek koddaki CLI örneğinin TEK global soketi,
// bkz. src/cli/README_UDP.md ve src/cli/cli_udp.cpp'deki `sSocket`)
//
// ot_mini tek-process/tek-düğüm olduğundan bu gerçek bir işletim sistemi
// soketi DEĞİLDİR (bkz. udp.h). Bir "send", ancak hedef adres+port bu SOKETİN
// kendi bind'ine denk düşüyorsa (ki bu da hedefin bu düğümün KENDİ adresi
// olmasını gerektirir) anında "teslim edilmiş" sayılır ve alım satırı
// yazdırılır. Aksi halde - TIPKI GERÇEK UDP'DE OLDUĞU GİBİ - gönderen paketin
// ulaşıp ulaşmadığını asla öğrenemez, bu yüzden sessizce "Done" ile bırakılır
// (UDP bağlantısız bir protokoldür, teslim garantisi/onayı yoktur).
// ---------------------------------------------------------------------------
void Cli::ProcessUdp(const std::vector<std::string> &aArgs)
{
    if (aArgs.empty())
    {
        std::printf("Error: 'udp help' yaz (udp tek basina bir komut degil)\n");
        return;
    }

    const std::string &cmd = aArgs[0];

    if (cmd == "help")
    {
        std::printf("udp komutlari:\n");
        std::printf("  help open close bind connect send\n");
        std::printf("Done\n");
    }
    else if (cmd == "open")
    {
        // Gercek koddaki otUdpOpen: zaten acik bir soketi tekrar acmak hata
        // degildir (idempotent) - bkz. cli_thread.cpp'deki 'ifconfig up' ile ayni yaklasim.
        mUdpSocket.mIsOpen = true;
        std::printf("Done\n");
    }
    else if (cmd == "close")
    {
        if (!mUdpSocket.mIsOpen)
        {
            std::printf("Error: soket zaten kapali\n");
            return;
        }

        mUdpSocket = UdpSocket{};
        std::printf("Done\n");
    }
    else if (cmd == "bind")
    {
        if (!mUdpSocket.mIsOpen)
        {
            std::printf("Error: 'udp bind' icin once 'udp open' calistirilmali\n");
            return;
        }

        size_t argIdx = 1;

        // Gercek CLI'daki -u/-b/-h netif secenekleri: ot_mini'de tek bir
        // arayuz (kendi IPv6 katmani) oldugundan bunlar ayirt edilemez.
        // -u zaten varsayilan davranisla ayni oldugundan sessizce yok
        // sayiliyor; -b/-h ise durustce reddediliyor (backbone/host arayuzu
        // ot_mini'de yok).
        if (argIdx < aArgs.size() && aArgs[argIdx] == "-u")
        {
            argIdx++;
        }
        else if (argIdx < aArgs.size() && (aArgs[argIdx] == "-b" || aArgs[argIdx] == "-h"))
        {
            std::printf("Error: 'udp bind %s' desteklenmiyor - backbone/host arayuzu ot_mini'de yok\n",
                         aArgs[argIdx].c_str());
            return;
        }

        if (aArgs.size() != argIdx + 2)
        {
            std::printf(
                "Error: 'udp bind [-u] <ip> <port>' yaz (herhangi bir adres icin ip yerine '::' kullan)\n");
            return;
        }

        Ip6Address addr;
        if (!ParseIp6Address(aArgs[argIdx], addr))
        {
            std::printf("Error: gecersiz adres '%s'\n", aArgs[argIdx].c_str());
            return;
        }

        uint16_t port;
        if (!ParsePort(aArgs[argIdx + 1], port))
        {
            std::printf("Error: gecersiz port '%s'\n", aArgs[argIdx + 1].c_str());
            return;
        }

        // Gercek OT herhangi bir yerel adrese bind edilebilir (cihazin
        // netif'ine eklenmis olmasi kaydiyla). ot_mini'de bunu durustce
        // dogrulamanin tek yolu: ya "::" (herhangi biri) ya da KENDI
        // adreslerimizden biri olmasi - baska bir adrese "bind edildi"
        // demek uydurma olurdu.
        if (!IsUnspecified(addr) && !IsOwnUnicastAddress(addr))
        {
            std::printf("Error: '%s' bu dugumun kendi adreslerinden biri degil - bind edilemez\n",
                         addr.ToString().c_str());
            return;
        }

        mUdpSocket.mIsBound   = true;
        mUdpSocket.mLocalAddr = addr;
        mUdpSocket.mLocalPort = port;
        std::printf("Done\n");
    }
    else if (cmd == "connect")
    {
        if (!mUdpSocket.mIsOpen)
        {
            std::printf("Error: 'udp connect' icin once 'udp open' calistirilmali\n");
            return;
        }

        if (aArgs.size() != 3)
        {
            std::printf("Error: 'udp connect <ip> <port>' yaz\n");
            return;
        }

        Ip6Address addr;
        if (!ParseIp6Address(aArgs[1], addr))
        {
            std::printf("Error: gecersiz adres '%s'\n", aArgs[1].c_str());
            return;
        }

        uint16_t port;
        if (!ParsePort(aArgs[2], port))
        {
            std::printf("Error: gecersiz port '%s'\n", aArgs[2].c_str());
            return;
        }

        // Gercek davranis: soket henuz bind edilmemisse, connect onu da
        // otomatik olarak gecici bir porta bind eder.
        if (!mUdpSocket.mIsBound)
        {
            mUdpSocket.mIsBound   = true;
            mUdpSocket.mLocalPort = RandomEphemeralPort();
        }

        mUdpSocket.mIsConnected = true;
        mUdpSocket.mPeerAddr    = addr;
        mUdpSocket.mPeerPort    = port;
        std::printf("Done\n");
    }
    else if (cmd == "send")
    {
        if (!mUdpSocket.mIsOpen)
        {
            std::printf("Error: 'udp send' icin once 'udp open' calistirilmali\n");
            return;
        }

        Ip6Address dest{};
        uint16_t   destPort = 0;
        size_t     msgStart;

        // "udp send <ip> <port> <mesaj...>" mi yoksa (connected soket icin)
        // "udp send <mesaj...>" mi oldugunu ayirt et: ilk iki argumanin
        // birlikte gecerli bir adres+port olusturup olusturmadigina bak.
        // NOT: mesajin ilk iki kelimesi tesaduf eseri gecerli bir adres+port
        // gibi gorunuyorsa (ornegin "udp send 1 2" mesaji), bu sezgisel yontem
        // yanlis anlar - gercek CLI'da adres/mesaj ayrimi konumsal oldugundan
        // (adres HER ZAMAN ilk iki arguman) bu belirsizlik yalnizca ot_mini'nin
        // "connected soket icin adres atlanabilir" kolayligindan kaynaklaniyor.
        Ip6Address maybeAddr;
        uint16_t   maybePort;
        if (aArgs.size() >= 4 && ParseIp6Address(aArgs[1], maybeAddr) && ParsePort(aArgs[2], maybePort))
        {
            dest     = maybeAddr;
            destPort = maybePort;
            msgStart = 3;
        }
        else if (mUdpSocket.mIsConnected)
        {
            dest     = mUdpSocket.mPeerAddr;
            destPort = mUdpSocket.mPeerPort;
            msgStart = 1;
        }
        else
        {
            std::printf("Error: 'udp send <ip> <port> <mesaj>' yaz ya da once 'udp connect' calistir\n");
            return;
        }

        if (msgStart >= aArgs.size())
        {
            std::printf("Error: gonderilecek bir mesaj belirtmelisin\n");
            return;
        }

        std::string message;
        for (size_t i = msgStart; i < aArgs.size(); i++)
        {
            if (!message.empty()) message += ' ';
            message += aArgs[i];
        }

        // Soket henuz bind edilmemisse, send de onu gecici bir porta bind eder
        // ("Issuing the udp send command binds the socket to an ephemeral port
        // if the socket has not already been bound" - README_UDP.md).
        if (!mUdpSocket.mIsBound)
        {
            mUdpSocket.mIsBound   = true;
            mUdpSocket.mLocalPort = RandomEphemeralPort();
        }

        std::printf("Done\n");

        // Teslimat kontrolu: paket ancak (a) hedef bizim kendi adresimiz ya
        // da abone oldugumuz bir multicast grubuysa, (b) soketimiz o adrese
        // (ya da "::" ile herhangi birine) bind edilmisse VE (c) hedef port
        // bizim dinledigimiz porta esitse "alinir".
        bool destIsUs      = IsOwnUnicastAddress(dest) || IsSubscribedMulticastAddress(dest);
        bool localAddrOk   = IsUnspecified(mUdpSocket.mLocalAddr) || Ip6AddressEqual(mUdpSocket.mLocalAddr, dest);
        bool portMatches   = destPort == mUdpSocket.mLocalPort;

        if (destIsUs && localAddrOk && portMatches)
        {
            // Gercek ciktidaki "<N> bytes from <addr> <port>" satirinin
            // karsiligi (bkz. README_UDP.md ornegi). Kaynak adresi olarak
            // KENDI adresimizi gosteriyoruz - loopback oldugu icin gonderen
            // ve alan ayni dugum.
            std::printf("%zu bytes from %s %u\n", message.size(), dest.ToString().c_str(),
                         static_cast<unsigned>(mUdpSocket.mLocalPort));
            std::printf("%s\n", message.c_str());
        }
    }
    else if (cmd == "linksecurity")
    {
        std::printf("Error: 'udp linksecurity' desteklenmiyor - ot_mini'de link-katmani guvenligi "
                     "(MAC/802.15.4 sifreleme) hic simule edilmiyor\n");
    }
    else
    {
        std::printf("Error: bilinmeyen 'udp' komutu '%s' (yardim: udp help)\n", cmd.c_str());
    }
}

} // namespace ot_mini
