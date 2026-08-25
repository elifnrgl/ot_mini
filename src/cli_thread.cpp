#include "cli_dataset.h"
#include <cstdio>

namespace ot_mini {

const char *RoleToString(Role aRole)
{
    switch (aRole)
    {
    case Role::kDisabled:
        return "disabled";
    case Role::kDetached:
        return "detached";
    case Role::kLeader:
        return "leader";
    }

    return "disabled";
}

// ---------------------------------------------------------------------------
// ifconfig - IPv6 arayüzü (gerçek koddaki `otIp6SetEnabled` / `otIp6IsEnabled`)
//
// Bu, Thread protokolünden AYRI bir katmandır: cihaz "ifconfig up" ile IPv6
// arayüzünü ayağa kaldırır (link-local adres bu noktada netif'e eklenir),
// ama bu tek başına Thread ağına katılmaya çalışmaz -- onun için ayrıca
// "thread start" gerekir (bkz. quick-start rehberleri: her ikisi de sırayla
// çalıştırılır, örn. openthread/examples/apps/cli ve OpenThread Codelab'leri).
// ---------------------------------------------------------------------------
void Cli::ProcessIfconfig(const std::vector<std::string> &aArgs)
{
    if (aArgs.empty())
    {
        std::printf("%s\n", mIsIfUp ? "up" : "down");
        std::printf("Done\n");
        return;
    }

    const std::string &sub = aArgs[0];

    if (sub == "up")
    {
        // Gerçek koddaki gibi idempotent: zaten "up" ise de sorunsuzca Done doner.
        mIsIfUp = true;
        std::printf("Done\n");
    }
    else if (sub == "down")
    {
        // Gerçek OpenThread'de Thread protokolü, arayüz "up" olmadan çalışamaz
        // (Mle::Start(), `Get<ThreadNetIf>().IsUp()` kontrol eder). Bu yüzden
        // önce "thread stop" çalıştırılmadan arayüzü indirmiyoruz -- tıpkı
        // resmi rehberlerde de her zaman önce "thread stop", sonra "ifconfig
        // down" sırasının izlenmesi gibi (bkz. OpenThread Hardware Codelab).
        if (mRole != Role::kDisabled)
        {
            std::printf("Error: arayuzu asagi indirmeden once 'thread stop' calistirilmali "
                         "(Thread protokolu ayakta bir arayuz gerektirir)\n");
            return;
        }

        mIsIfUp = false;
        std::printf("Done\n");
    }
    else
    {
        std::printf("Error: bilinmeyen 'ifconfig' secenegi '%s' (up, down, ya da argumansiz durumu goster)\n",
                     sub.c_str());
    }
}

// ---------------------------------------------------------------------------
// thread start / thread stop (gerçek koddaki `otThreadSetEnabled`)
// ---------------------------------------------------------------------------
void Cli::ProcessThread(const std::vector<std::string> &aArgs)
{
    if (aArgs.empty())
    {
        std::printf("Error: 'thread start' ya da 'thread stop' yaz\n");
        return;
    }

    const std::string &sub = aArgs[0];

    if (sub == "start")
    {
        if (mRole != Role::kDisabled)
        {
            // Gerçek koddaki Mle::Start(): zaten disabled degilse basitçe
            // basarili sayilir (no-op).
            std::printf("Done\n");
            return;
        }

        if (!mIsIfUp)
        {
            // Gerçek koddaki OT_ERROR_INVALID_STATE karsiligi (Mle::Start() ->
            // VerifyOrExit(Get<ThreadNetIf>().IsUp(), error = kErrorInvalidState)).
            std::printf("Error: 'thread start' icin once 'ifconfig up' calistirilmali \n");
            return;
        }

        uint8_t networkKey[16];
        if (!mHasActiveDataset || !mActiveDataset.GetNetworkKey(networkKey))
        {
            // Gerçek koddaki `otDatasetIsCommissioned()` karsiligi: en azindan
            // NetworkKey iceren commit edilmis bir active dataset gerekir
            // (bkz. ESP32-H2 ornegindeki "minimal provisioning": sadece
            // networkkey ile de baglanilabiliyor).
            std::printf("Error: 'thread start' icin commit edilmis bir active dataset gerekir "
                         "(en azindan networkkey ayarli olmali) - once 'dataset commit active' calistir\n");
            return;
        }

        mRole = Role::kLeader;
        std::printf("Done\n");
    }
    else if (sub == "stop")
    {
        // Gerçek koddaki otThreadSetEnabled(false): zaten disabled ise no-op,
        // degilse dogrudan disabled'a duser (ara "detached" ani burada da
        // senkron CLI'da gozlemlenmez).
        mRole = Role::kDisabled;
        std::printf("Done\n");
    }
    else
    {
        std::printf("Error: bilinmeyen 'thread' secenegi '%s' (start ya da stop)\n", sub.c_str());
    }
}

// ---------------------------------------------------------------------------
// state (gerçek koddaki `otThreadGetDeviceRole`)
// ---------------------------------------------------------------------------
void Cli::ProcessState(const std::vector<std::string> &aArgs)
{
    if (aArgs.empty())
    {
        std::printf("%s\n", RoleToString(mRole));
        std::printf("Done\n");
        return;
    }

    std::printf("Error: 'state <rol>' ile rolu zorla degistirmek gercek OpenThread'de yalnizca "
                 "reference-device build'lerinde var; ot_mini'de desteklenmiyor "
                 "(rol, 'thread start'/'thread stop'un SONUCUDUR - yardim: argumansiz 'state' ile oku)\n");
}

} // namespace ot_mini
