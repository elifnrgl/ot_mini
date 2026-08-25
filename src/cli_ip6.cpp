#include "cli_dataset.h"
#include "ip6.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>

namespace ot_mini {

// ---------------------------------------------------------------------------
// Kurulum
// ---------------------------------------------------------------------------

Cli::Cli()
{
    // Gerçek cihazda extended adres fabrikada (ya da otPlatRadioGetIeeeEui64
    // üzerinden) atanmış olarak gelir ve process/reboot'lar arasında sabit
    // kalır. Burada gerçek donanım olmadığından, Cli her başlatıldığında
    // rastgele üretiyoruz - ama tek bir Cli örneğinin YAŞAMI boyunca sabit
    // kalıyor (dataset.cpp'deki GenerateRandom ile aynı seed yaklaşımı).
    std::srand(static_cast<unsigned>(std::time(nullptr)) ^ static_cast<unsigned>(reinterpret_cast<uintptr_t>(this)));

    for (auto &b : mExtAddress)
    {
        b = static_cast<uint8_t>(std::rand() & 0xff);
    }
}

void Cli::EnsureMlEidIid()
{
    if (mHasMlEidIid) return;

    for (auto &b : mMlEidIid)
    {
        b = static_cast<uint8_t>(std::rand() & 0xff);
    }

    mHasMlEidIid = true;
}

// ---------------------------------------------------------------------------
// Kendi adres kontrolleri (ping/udp/coap icin - bkz. cli_dataset.h)
// ---------------------------------------------------------------------------

bool Cli::IsOwnUnicastAddress(const Ip6Address &aAddr)
{
    Ip6Address linkLocal;
    ComputeLinkLocalAddress(mExtAddress, linkLocal);
    if (Ip6AddressEqual(aAddr, linkLocal)) return true;

    if (mHasActiveDataset)
    {
        // ML-EID her zaman "var"dır (gercek cihazda mesh-local prefix
        // atandigi an atanir) - EnsureMlEidIid burada da tembel-uretimi
        // (ilk 'ipaddr mleid' ile ayni sekilde) tetikliyor, boylece hic
        // sorulmamis olsa da adres KARSILASTIRMA icin dogru sekilde var olur.
        EnsureMlEidIid();

        uint8_t prefix[8];
        mActiveDataset.GetMeshLocalPrefix(prefix);

        Ip6Address mlEid;
        ComputeMlEid(prefix, mMlEidIid, mlEid);
        if (Ip6AddressEqual(aAddr, mlEid)) return true;
    }

    return false;
}

bool Cli::IsSubscribedMulticastAddress(const Ip6Address &aAddr)
{
    // Gercek cihaz LLATN/RLATN'a ancak Thread attach olduktan sonra uye olur;
    // ot_mini'de 'ipmaddr' de ayni kosulu (yalnizca active dataset commit
    // edilmis olmasi) kullaniyor, biz de tutarlilik icin ayni kurala uyuyoruz.
    if (!mHasActiveDataset) return false;

    uint8_t prefix[8];
    mActiveDataset.GetMeshLocalPrefix(prefix);

    Ip6Address llatn, rlatn;
    ComputeAllThreadNodesAddress(prefix, McastScope::kLinkLocal, llatn);
    ComputeAllThreadNodesAddress(prefix, McastScope::kRealmLocal, rlatn);

    return Ip6AddressEqual(aAddr, llatn) || Ip6AddressEqual(aAddr, rlatn);
}

// ---------------------------------------------------------------------------
// ipaddr - tekil (unicast) adresler
// ---------------------------------------------------------------------------

void Cli::ProcessIpAddr(const std::vector<std::string> &aArgs)
{
    if (aArgs.empty())
    {
        // Gerçek koddaki bare `ipaddr` gibi: netif'e eklenmis tum unicast
        // adresleri listeler. Burada "eklenmis" olan, her zaman var olan
        // link-local ve (varsa) ML-EID.
        Ip6Address linkLocal;
        ComputeLinkLocalAddress(mExtAddress, linkLocal);
        std::printf("%s\n", linkLocal.ToString().c_str());

        if (mHasActiveDataset)
        {
            uint8_t prefix[8];
            mActiveDataset.GetMeshLocalPrefix(prefix); // committed ise her zaman set

            EnsureMlEidIid();

            Ip6Address mlEid;
            ComputeMlEid(prefix, mMlEidIid, mlEid);
            std::printf("%s\n", mlEid.ToString().c_str());
        }

        std::printf("Done\n");
        return;
    }

    const std::string &sub = aArgs[0];

    if (sub == "linklocal")
    {
        Ip6Address addr;
        ComputeLinkLocalAddress(mExtAddress, addr);
        std::printf("%s\n", addr.ToString().c_str());
        std::printf("Done\n");
    }
    else if (sub == "mleid")
    {
        if (!mHasActiveDataset)
        {
            std::printf("Error: ML-EID icin mesh-local prefix gerekir; once 'dataset commit active' calistir\n");
            return;
        }

        uint8_t prefix[8];
        mActiveDataset.GetMeshLocalPrefix(prefix);

        EnsureMlEidIid();

        Ip6Address addr;
        ComputeMlEid(prefix, mMlEidIid, addr);
        std::printf("%s\n", addr.ToString().c_str());
        std::printf("Done\n");
    }
    else if (sub == "rloc")
    {
        // RLOC16, yalnizca gercek bir Thread attach sirasinda Leader/parent
        // tarafindan atanir (mle.cpp / address_resolver). ot_mini'de ne bir
        // mesh ne de bir Leader oldugundan bu adres DURUSTCE uretilemez -
        // uydurmak yerine acikca desteklenmedigini belirtiyoruz.
        std::printf("Error: 'ipaddr rloc' desteklenmiyor - RLOC16 gercek bir Thread attach'i gerektirir "
                     "(ot_mini'de mesh/Leader yok)\n");
    }
    else
    {
        std::printf("Error: bilinmeyen 'ipaddr' secenegi '%s' (linklocal, mleid, rloc, ya da argumansiz listele)\n",
                     sub.c_str());
    }
}

// ---------------------------------------------------------------------------
// ipmaddr - cok noktaya yayin (multicast) adresleri
// ---------------------------------------------------------------------------

void Cli::ProcessIpMulticastAddr(const std::vector<std::string> &aArgs)
{
    if (!mHasActiveDataset)
    {
        std::printf(
            "Error: All-Thread-Nodes adresleri mesh-local prefix'ten turetilir; once 'dataset commit active' calistir\n");
        return;
    }

    uint8_t prefix[8];
    mActiveDataset.GetMeshLocalPrefix(prefix);

    if (aArgs.empty())
    {
        // Gercek cihaz attach oldugunda LLATN/RLATN'a otomatik uye olur; biz
        // de bare `ipmaddr` icin ikisini birden listeliyoruz.
        Ip6Address llatn, rlatn;
        ComputeAllThreadNodesAddress(prefix, McastScope::kLinkLocal, llatn);
        ComputeAllThreadNodesAddress(prefix, McastScope::kRealmLocal, rlatn);

        std::printf("%s\n", llatn.ToString().c_str());
        std::printf("%s\n", rlatn.ToString().c_str());
        std::printf("Done\n");
        return;
    }

    const std::string &sub = aArgs[0];

    if (sub == "llatn")
    {
        Ip6Address addr;
        ComputeAllThreadNodesAddress(prefix, McastScope::kLinkLocal, addr);
        std::printf("%s\n", addr.ToString().c_str());
        std::printf("Done\n");
    }
    else if (sub == "rlatn")
    {
        Ip6Address addr;
        ComputeAllThreadNodesAddress(prefix, McastScope::kRealmLocal, addr);
        std::printf("%s\n", addr.ToString().c_str());
        std::printf("Done\n");
    }
    else
    {
        std::printf("Error: bilinmeyen 'ipmaddr' secenegi '%s' (llatn, rlatn, ya da argumansiz listele)\n",
                     sub.c_str());
    }
}

} // namespace ot_mini
