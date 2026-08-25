#include "cli_dataset.h"
#include "ip6.h"
#include <cstdio>
#include <cstdlib>
#include <chrono>

namespace ot_mini {

namespace {

bool ParseUint(const std::string &aStr, unsigned long &aValue)
{
    if (aStr.empty()) return false;

    char         *end;
    unsigned long v = std::strtoul(aStr.c_str(), &end, 10);
    if (*end != '\0') return false;

    aValue = v;
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// ping - ICMPv6 Echo Request/Reply (gerçek koddaki "Ping Sender" modülü,
// bkz. openthread.io/reference/config/group/config-ping-sender)
//
// ot_mini gerçek bir ikinci düğüm/mesh içermediğinden, bir ping ancak hedef
// adres bu düğümün KENDİ adreslerinden biriyse (unicast: link-local/ML-EID)
// ya da -m bayrağıyla abone olunan bir multicast grubuysa "yanıt" alabilir -
// diğer her durumda (gerçekçi şekilde) %100 paket kaybı raporlanır.
//
// Gerçek cihazda bir "timeout" birkaç saniye sürer (varsayılan
// OPENTHREAD_CONFIG_PING_SENDER_DEFAULT_TIMEOUT = 3000ms); ot_mini senkron
// çalıştığından (bkz. cli_thread.cpp'deki "thread start" açıklaması) bu
// bekleme süresini SİMÜLE ETMİYORUZ, sonucu hemen bildiriyoruz.
// ---------------------------------------------------------------------------
void Cli::ProcessPing(const std::vector<std::string> &aArgs)
{
    if (!mIsIfUp)
    {
        std::printf(
            "Error: 'ping' icin once 'ifconfig up' calistirilmali (IPv6 arayuzu asagidayken paket gonderilemez)\n");
        return;
    }

    unsigned long count         = 1; // OPENTHREAD_CONFIG_PING_SENDER_DEFAULT_COUNT
    unsigned long size          = 8; // OPENTHREAD_CONFIG_PING_SENDER_DEFAULT_SIZE (ICMPv6 basligi haric veri boyutu)
    bool          multicastLoop = false;
    std::string   addrStr;

    for (size_t i = 0; i < aArgs.size(); i++)
    {
        const std::string &arg = aArgs[i];

        if (arg == "-c")
        {
            unsigned long v;
            if (i + 1 >= aArgs.size() || !ParseUint(aArgs[++i], v) || v == 0)
            {
                std::printf("Error: '-c' icin 0'dan buyuk gecerli bir sayi verilmeli\n");
                return;
            }
            count = v;
        }
        else if (arg == "-s")
        {
            unsigned long v;
            if (i + 1 >= aArgs.size() || !ParseUint(aArgs[++i], v))
            {
                std::printf("Error: '-s' icin gecerli bir sayi verilmeli\n");
                return;
            }
            size = v;
        }
        else if (arg == "-m")
        {
            multicastLoop = true;
        }
        else if (!arg.empty() && arg[0] == '-')
        {
            std::printf("Error: bilinmeyen 'ping' secenegi '%s' - ot_mini yalnizca -c, -s, -m destekliyor "
                         "(async/-I/-i/-p/-t/-w gercek bir zamanlayici/paket-kaynagi secimi gerektirir)\n",
                         arg.c_str());
            return;
        }
        else
        {
            if (!addrStr.empty())
            {
                std::printf("Error: birden fazla adres verildi\n");
                return;
            }
            addrStr = arg;
        }
    }

    if (addrStr.empty())
    {
        std::printf("Error: 'ping [-c sayim] [-s boyut] [-m] <adres>' yaz\n");
        return;
    }

    Ip6Address dest;
    if (!ParseIp6Address(addrStr, dest))
    {
        std::printf("Error: gecersiz adres '%s'\n", addrStr.c_str());
        return;
    }

    bool isMulticast = dest.mFields[0] == 0xff; //hedef adresin ilk byte ı 0xff ise bu adres multıcast
    bool reachable;

    if (isMulticast)
    {

        reachable = multicastLoop && IsSubscribedMulticastAddress(dest); //bu adrese ping gönderilebilir mi
    }
    else
    {
        reachable = IsOwnUnicastAddress(dest);
    }

    unsigned long received = 0;
    unsigned long minMs = 0, maxMs = 0, sumMs = 0;

    for (unsigned long seq = 1; seq <= count; seq++)
    {
        if (!reachable) continue;

        // Gercek islemde paket IPv6/ICMPv6 katmanindan gecip geri donuyor -
        // ot_mini'de bu tamamen bellek ici oldugundan olculen sure gercekten
        // de ~0ms 
        auto          start = std::chrono::steady_clock::now();
        auto          end   = std::chrono::steady_clock::now();
        unsigned long ms    = static_cast<unsigned long>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

        std::printf("%lu bytes from %s: icmp_seq=%lu hlim=64 time=%lums\n", size + 8, dest.ToString().c_str(), seq,
                     ms);

        if (received == 0) { minMs = maxMs = ms; }
        else
        {
            if (ms < minMs) minMs = ms;
            if (ms > maxMs) maxMs = ms;
        }
        sumMs += ms;
        received++;
    }

    double lossPct = 100.0 * static_cast<double>(count - received) / static_cast<double>(count);
    double avgMs   = received > 0 ? static_cast<double>(sumMs) / static_cast<double>(received) : 0.0;

    std::printf("%lu packets transmitted, %lu packets received. Packet loss = %.1f%%.\n", count, received, lossPct);
    std::printf("Round-trip min/avg/max = %lu/%.1f/%lu ms.\n", minMs, avgMs, maxMs);
    std::printf("Done\n");
}

} // namespace ot_mini
