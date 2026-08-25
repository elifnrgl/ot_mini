#pragma once
#include <string>
#include <vector>
#include "dataset.h"
#include "netdata.h"
#include "udp.h"
#include "coap.h"

namespace ot_mini {

// ---------------------------------------------------------------------------
// Role
//
// Gerçek koddaki otDeviceRole'ün (src/core/thread/mle_types.hpp) karşılığı.
// Gerçekte 5 değer vardır: disabled, detached, child, router, leader
// (bkz. `state` komutu -- CLI README: "offline, disabled, detached, child,
// router or leader"). ot_mini'de ne gerçek bir radyo ne de ikinci bir düğüm
// olduğundan child/router ASLA üretilmez: bu roller başka bir Parent/Router'a
// gerçekten MLE ile bağlanmayı gerektirir. Leader ise farklı -- bir düğüm
// kendi partition'ını KENDİSİ kurar (bkz. cli_thread.cpp'deki ProcessThread
// içindeki açıklama) ve bu, ot_mini'nin "tek düğüm" modeliyle dürüstçe temsil
// edilebilir.
// ---------------------------------------------------------------------------
enum class Role : uint8_t
{
    kDisabled, // Thread protokolü çalışmıyor ("thread stop" sonrası ya da hiç başlamamış)
    kDetached, // ot_mini'de senkron olduğumuz için gözlemlenebilir bir ara durum değil;
               // yalnızca belgeleme amacıyla tutuluyor (bkz. cli_thread.cpp)
    kLeader,   // "thread start" sonrası tek düğümün kendi partition'ını kurmuş hali
};

const char *RoleToString(Role aRole);

// Gerçek OpenThread'de bu, src/cli/cli_dataset.cpp'deki ot::Cli::Dataset sınıfının
// karşılığı: kullanıcı girdisini parse edip Dataset sınıfının API'sini çağırır.
// (`ipaddr`/`ipmaddr`/`netdata` de gerçek OpenThread'de olduğu gibi -- src/cli/cli.cpp'de
// ayrı bir sınıf yerine doğrudan Cli üzerinde -- burada da aynı sınıfa eklendi.
// `ifconfig`/`thread`/`state` de aynı nedenle burada.)
class Cli
{
public:
    Cli();

    void ProcessLine(const std::string &aLine);

private:
    void ProcessDataset(const std::vector<std::string> &aArgs);
    void ProcessIpAddr(const std::vector<std::string> &aArgs);
    void ProcessIpMulticastAddr(const std::vector<std::string> &aArgs);
    void ProcessNetData(const std::vector<std::string> &aArgs);
    void ProcessIfconfig(const std::vector<std::string> &aArgs);
    void ProcessThread(const std::vector<std::string> &aArgs);
    void ProcessState(const std::vector<std::string> &aArgs);
    void ProcessPing(const std::vector<std::string> &aArgs);
    void ProcessUdp(const std::vector<std::string> &aArgs);
    void ProcessCoap(const std::vector<std::string> &aArgs);

    // ML-EID'in IID kısmı ilk ihtiyaç duyulduğunda rastgele üretilip
    // process ömrü boyunca saklanır (gerçek koddaki, cihazın flash'ında bir
    // kez üretilip kalıcı tutulan mesh-local IID'nin karşılığı).
    void EnsureMlEidIid();

    // Su anki local netdata TLV uzunluguna bakip gerekirse mNetDataMaxLength'i gunceller.
    void UpdateNetDataMaxLength();

    // Verilen adresin bu düğümün KENDİ tekil (unicast) adreslerinden biri
    // (link-local ya da - active dataset commit edilmişse - ML-EID) olup
    // olmadığını söyler. ping/udp/coap'ın "gerçekten teslim edilebilir mi?"
    // kontrolünün temeli (bkz. cli_ping.cpp / cli_udp.cpp / cli_coap.cpp):
    // ot_mini'de gerçek bir ikinci düğüm/mesh olmadığından, yalnızca kendi
    // adreslerine giden paketler dürüstçe "teslim edilmiş" sayılabilir.
    bool IsOwnUnicastAddress(const Ip6Address &aAddr);

    // Verilen adresin, bu düğümün abone olduğu multicast gruplarından biri
    // (LLATN ya da RLATN - ipmaddr'daki gibi active dataset commit gerektirir)
    // olup olmadığını söyler.
    bool IsSubscribedMulticastAddress(const Ip6Address &aAddr);

    // Gerçek koddaki `static uint8_t sDatasetTlvs[]` karşılığı: henüz commit
    // edilmemiş TASLAK buffer. Komutların çoğu bunun üzerinde çalışır.
    Dataset mStagingDataset;

    // Gerçek koddaki "cihazın flash'ında saklanan active dataset" karşılığı.
    // Burada gerçek kalıcı depo yok, sadece process hayatta olduğu sürece
    // bellekte tutuyoruz - ama KAVRAM olarak (staging vs committed) birebir aynı.
    Dataset mActiveDataset;
    bool    mHasActiveDataset = false;

    // Gerçek koddaki "factory-assigned IEEE 802.15.4 extended address"ın
    // karşılığı. Burada gerçek donanım/flash yok, bu yüzden Cli oluşturulduğunda
    // rastgele üretilip process ömrü boyunca sabit tutulur. Link-local adres
    // bundan türetilir.
    uint8_t mExtAddress[8];

    // ML-EID'in rastgele IID kısmı. mHasActiveDataset gerekmez -- ama adresi
    // hesaplamak için mesh-local prefix gerektiğinden, `ipaddr mleid` yine de
    // commit edilmiş bir active dataset ister (bkz. cli_ip6.cpp).
    uint8_t mMlEidIid[8];
    bool    mHasMlEidIid = false;

    // Gerçek koddaki "local Network Data" (Publisher'ın Leader'a henüz register
    // etmediği kendi girdileri) karşılığı. `netdata register` ve gerçek Leader'dan
    // gelen düz `netdata show` desteklenmiyor -- bkz. cli_netdata.cpp.
    NetData mLocalNetData;
    size_t  mNetDataMaxLength = 0;

    // Gerçek koddaki IPv6 arayüz durumu (`otIp6IsEnabled` / "ifconfig up|down").
    // Thread, arayüz "up" değilken ASLA başlatılamaz (bkz. cli_thread.cpp).
    bool mIsIfUp = false;

    // Gerçek koddaki `otThreadGetDeviceRole()`'ün karşılığı. "thread start"/"thread stop"
    // ile değişir; ayrıntı için Role enum'undaki ve cli_thread.cpp'deki açıklamalara bak.
    Role mRole = Role::kDisabled;

    // Gerçek CLI örneğindeki TEK global UDP soketinin (bkz. udp.h) karşılığı.
    UdpSocket mUdpSocket;

    // Gerçek CLI örneğindeki CoAP agent + tek "test resource"ın (bkz. coap.h) karşılığı.
    CoapServer mCoapServer;
};

} // namespace ot_mini
