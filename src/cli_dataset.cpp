#include "cli_dataset.h"
#include "hex.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace ot_mini {


static bool OutputChannel(const Dataset &d)
{
    uint16_t v;
    if (!d.GetChannel(v)) return false;
    std::printf("%u\n", v);
    return true;
}
static bool ParseNumeric(const std::string &s, unsigned long long &aOut)
{
    if (s.empty()) return false;
    char *end;
    aOut = std::strtoull(s.c_str(), &end, 0);
    return *end == '\0';
}

static bool ParseChannel(Dataset &d, const std::string &s)
{
    unsigned long long v;
    if (!ParseNumeric(s, v)) { std::printf("Error: gecersiz sayi '%s'\n", s.c_str()); return false; }
    d.SetChannel(static_cast<uint16_t>(v));
    return true;
}

static bool OutputPanId(const Dataset &d)
{
    uint16_t v;
    if (!d.GetPanId(v)) return false;
    std::printf("0x%04x\n", v);
    return true;
}
static bool ParsePanId(Dataset &d, const std::string &s)
{
    unsigned long long v;
    if (!ParseNumeric(s, v)) { std::printf("Error: gecersiz sayi '%s'\n", s.c_str()); return false; }
    d.SetPanId(static_cast<uint16_t>(v));
    return true;
}

static bool OutputExtPanId(const Dataset &d)
{
    uint8_t v[8];
    if (!d.GetExtPanId(v)) return false;
    std::printf("%s\n", BytesToHexString(v, 8).c_str());
    return true;
}
static bool ParseExtPanId(Dataset &d, const std::string &s)
{
    uint8_t v[8];
    size_t  len;
    if (!HexStringToBytes(s, v, 8, len) || len != 8) { std::printf("Error: 8 byte (16 hex karakter) bekleniyor\n"); return false; }
    d.SetExtPanId(v);
    return true;
}

static bool OutputNetworkName(const Dataset &d)
{
    std::string n;
    if (!d.GetNetworkName(n)) return false;
    std::printf("%s\n", n.c_str());
    return true;
}
static bool ParseNetworkName(Dataset &d, const std::string &s)
{
    if (s.size() > 16) std::printf("Uyari: isim 16 karaktere kesildi\n");
    d.SetNetworkName(s);
    return true;
}

static bool OutputNetworkKey(const Dataset &d)
{
    uint8_t v[16];
    if (!d.GetNetworkKey(v)) return false;
    std::printf("%s\n", BytesToHexString(v, 16).c_str());
    return true;
}
static bool ParseNetworkKey(Dataset &d, const std::string &s)
{
    uint8_t v[16];
    size_t  len;
    if (!HexStringToBytes(s, v, 16, len) || len != 16) { std::printf("Error: 16 byte (32 hex karakter) bekleniyor\n"); return false; }
    d.SetNetworkKey(v);
    return true;
}

static bool OutputMeshLocalPrefix(const Dataset &d)
{
    uint8_t v[8];
    if (!d.GetMeshLocalPrefix(v)) return false;
    std::printf("%02x%02x:%02x%02x:%02x%02x:%02x%02x::/64\n", v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
    return true;
}
static bool ParseMeshLocalPrefix(Dataset &d, const std::string &s)
{
    uint8_t v[8];
    size_t  len;
    if (!HexStringToBytes(s, v, 8, len) || len != 8) { std::printf("Error: 8 byte (16 hex karakter) bekleniyor\n"); return false; }
    d.SetMeshLocalPrefix(v);
    return true;
}

static bool OutputPskc(const Dataset &d)
{
    uint8_t v[16];
    if (!d.GetPskc(v)) return false;
    std::printf("%s\n", BytesToHexString(v, 16).c_str());
    return true;
}
static bool ParsePskc(Dataset &d, const std::string &s)
{
    uint8_t v[16];
    size_t  len;
    if (!HexStringToBytes(s, v, 16, len) || len != 16) { std::printf("Error: 16 byte (32 hex karakter) bekleniyor\n"); return false; }
    d.SetPskc(v);
    return true;
}

static bool OutputActiveTimestamp(const Dataset &d)
{
    uint64_t v;
    if (!d.GetActiveTimestamp(v)) return false;
    std::printf("%llu\n", static_cast<unsigned long long>(v));
    return true;
}
static bool ParseActiveTimestamp(Dataset &d, const std::string &s)
{
    unsigned long long v;
    if (!ParseNumeric(s, v)) { std::printf("Error: gecersiz sayi '%s'\n", s.c_str()); return false; }
    d.SetActiveTimestamp(v);
    return true;
}

using OutputFn = bool (*)(const Dataset &);
using ParseFn  = bool (*)(Dataset &, const std::string &);

struct FieldHandler
{
    const char *mName;
    OutputFn    mOutput;
    ParseFn     mParse;
};

// Gerçek koddaki kMappers[] dizisinin karşılığı.
static const FieldHandler kFieldHandlers[] = {
    {"activetimestamp", OutputActiveTimestamp, ParseActiveTimestamp},
    {"channel", OutputChannel, ParseChannel},
    {"panid", OutputPanId, ParsePanId},
    {"extpanid", OutputExtPanId, ParseExtPanId},
    {"networkname", OutputNetworkName, ParseNetworkName},
    {"networkkey", OutputNetworkKey, ParseNetworkKey},
    {"meshlocalprefix", OutputMeshLocalPrefix, ParseMeshLocalPrefix},
    {"pskc", OutputPskc, ParsePskc},
};

// ---------------------------------------------------------------------------
// Komut dağıtımı
// ---------------------------------------------------------------------------

static std::vector<std::string> Tokenize(const std::string &aLine)
{
    std::vector<std::string> tokens;
    std::istringstream       iss(aLine);
    std::string              tok;

    while (iss >> tok) tokens.push_back(tok);

    return tokens;
}

void Cli::ProcessLine(const std::string &aLine)
{
    std::vector<std::string> tokens = Tokenize(aLine);

    if (tokens.empty()) return;

    const std::string cmd = tokens[0];
    tokens.erase(tokens.begin());

    if (cmd == "dataset")
    {
        ProcessDataset(tokens);
    }
    else if (cmd == "ipaddr")
    {
        ProcessIpAddr(tokens);
    }
    else if (cmd == "ipmaddr")
    {
        ProcessIpMulticastAddr(tokens);
    }
    else if (cmd == "netdata")
    {
        ProcessNetData(tokens);
    }
    else if (cmd == "ifconfig")
    {
        ProcessIfconfig(tokens);
    }
    else if (cmd == "thread")
    {
        ProcessThread(tokens);
    }
    else if (cmd == "state")
    {
        ProcessState(tokens);
    }
    else if (cmd == "ping")
    {
        ProcessPing(tokens);
    }
    else if (cmd == "udp")
    {
        ProcessUdp(tokens);
    }
    else if (cmd == "coap")
    {
        ProcessCoap(tokens);
    }
    else
    {
        std::printf("Error: bu mini CLI simdilik sadece 'dataset', 'ipaddr', 'ipmaddr', 'netdata', "
                     "'ifconfig', 'thread', 'state', 'ping', 'udp', 'coap' komutlarini destekliyor\n");
    }
}

void Cli::ProcessDataset(const std::vector<std::string> &aArgs)
{
    // "dataset" tek başına -> taslağı yazdır (gerçek koddaki Print())
    if (aArgs.empty())
    {
        mStagingDataset.Print();
        std::printf("Done\n");
        return;
    }

    const std::string &cmd = aArgs[0];

    // 1) ADIM: önce bu kelime bir "alan adı" mı? (mapper tablosunda ara)
    for (const auto &handler : kFieldHandlers)
    {
        if (cmd == handler.mName)
        {
            if (aArgs.size() == 1)
            {
                // Argüman yok -> GET
                if (!handler.mOutput(mStagingDataset))
                    std::printf("Error: bu alan taslakta ayarli degil\n");
                else
                    std::printf("Done\n");
            }
            else
            {
                // Argüman var -> SET (hata mesajını Parse fonksiyonu kendi basar)
                if (handler.mParse(mStagingDataset, aArgs[1]))
                    std::printf("Done\n");
            }
            return;
        }
    }

    // 2) ADIM: alan değilse, özel alt-komutlara bak (gerçek koddaki kCommands)
    if (cmd == "help")
    {
        std::printf("dataset komutlari:\n");
        std::printf("  dataset                     taslagi yazdir\n");
        std::printf("  dataset help                bu listeyi goster\n");
        std::printf("  dataset clear               taslagi sifirla\n");
        std::printf("  dataset init new            rastgele yeni bir ag uret -> taslaga yaz\n");
        std::printf("  dataset init active         commit edilmis active'i taslaga kopyala\n");
        std::printf("  dataset commit active       taslagi 'active' olarak kaydet (kalici)\n");
        std::printf("  dataset active              kaydedilmis active dataset'i goster\n");
        std::printf("  dataset tlvs                taslagin ham TLV (hex) halini goster\n");
        std::printf("  dataset <alan> [deger]      alani oku ya da yaz:\n");
        std::printf("                              channel, panid, extpanid, networkname,\n");
        std::printf("                              networkkey, meshlocalprefix, pskc, activetimestamp\n");
        std::printf("\nipaddr komutlari:\n");
        std::printf("  ipaddr                      tekil (unicast) adresleri listele\n");
        std::printf("  ipaddr linklocal             link-local adresi goster\n");
        std::printf("  ipaddr mleid                 ML-EID adresini goster (active dataset gerektirir)\n");
        std::printf("  ipaddr rloc                  desteklenmiyor (gercek Thread attach gerektirir)\n");
        std::printf("\nipmaddr komutlari:\n");
        std::printf("  ipmaddr                      All-Thread-Nodes adreslerini listele\n");
        std::printf("  ipmaddr llatn                 Link-Local All Thread Nodes (ff32:40:..) goster\n");
        std::printf("  ipmaddr rlatn                 Realm-Local All Thread Nodes (ff33:40:..) goster\n");
        std::printf("\nnetdata komutlari:\n");
        std::printf("  netdata help                 bu listeyi goster\n");
        std::printf("  netdata publish prefix <prefix> [padcrosn] [high|med|low]\n");
        std::printf("                                on-mesh prefix yayinla (yerel)\n");
        std::printf("  netdata publish route <prefix> [sna] [high|med|low]\n");
        std::printf("                                external route yayinla (yerel)\n");
        std::printf("  netdata unpublish <prefix>   yayindaki prefix/route'u kaldir\n");
        std::printf("  netdata show local [-x]      yerel netdata'yi goster (okunabilir ya da hex TLV)\n");
        std::printf("  netdata length                yerel netdata TLV uzunlugu (byte)\n");
        std::printf("  netdata maxlength [reset]     gozlemlenen en buyuk uzunluk / sifirla\n");
        std::printf("  netdata show / register / publish dnssrp / steeringdata\n");
        std::printf("                                desteklenmiyor (gercek Leader/mesh gerektirir)\n");
        std::printf("\nifconfig komutlari:\n");
        std::printf("  ifconfig                     IPv6 arayuzu 'up' mi 'down' mi goster\n");
        std::printf("  ifconfig up                  IPv6 arayuzunu ayaga kaldir\n");
        std::printf("  ifconfig down                IPv6 arayuzunu indir (once 'thread stop' gerekir)\n");
        std::printf("\nthread komutlari:\n");
        std::printf("  thread start                 Thread protokolunu baslat (ifconfig up ve commit\n");
        std::printf("                                edilmis bir active dataset -- en az networkkey -- gerekir)\n");
        std::printf("  thread stop                  Thread protokolunu durdur\n");
        std::printf("\nstate komutu:\n");
        std::printf("  state                         guncel rolu goster: disabled, detached ya da leader\n");
        std::printf("                                (child/router burada uretilemez -- gercek bir ikinci\n");
        std::printf("                                dugum/Parent'a MLE ile baglanmayi gerektirir)\n");
    }
    else if (cmd == "clear")
    {
        mStagingDataset.Clear();
        std::printf("Done\n");
    }
    else if (cmd == "init")
    {
        if (aArgs.size() < 2)
        {
            std::printf("Error: 'dataset init new' ya da 'dataset init active' yaz\n");
        }
        else if (aArgs[1] == "new")
        {
            mStagingDataset.GenerateRandom();
            std::printf("Done\n");
        }
        else if (aArgs[1] == "active")
        {
            if (!mHasActiveDataset)
            {
                std::printf("Error: henuz commit edilmis bir active dataset yok\n");
            }
            else
            {
                mStagingDataset = mActiveDataset;
                std::printf("Done\n");
            }
        }
        else
        {
            std::printf("Error: bilinmeyen init secenegi\n");
        }
    }
    else if (cmd == "commit")
    {
        if (aArgs.size() < 2 || aArgs[1] != "active")
        {
            std::printf("Error: su an sadece 'dataset commit active' destekleniyor\n");
        }
        else
        {
            mActiveDataset    = mStagingDataset;
            mHasActiveDataset = true;
            std::printf("Done\n");
        }
    }
    else if (cmd == "active")
    {
        if (!mHasActiveDataset)
        {
            std::printf("Error: henuz commit edilmis bir active dataset yok\n");
        }
        else
        {
            mActiveDataset.Print();
            std::printf("Done\n");
        }
    }
    else if (cmd == "tlvs")
    {
        mStagingDataset.PrintTlvsHex();
        std::printf("Done\n");
    }
    else
    {
        std::printf("Error: bilinmeyen komut '%s' (yardim: dataset help)\n", cmd.c_str());
    }
}

} // namespace ot_mini
