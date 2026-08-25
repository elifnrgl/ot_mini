#include "cli_dataset.h"
#include "ip6.h"
#include "hex.h"
#include <cstdio>
#include <vector>

namespace ot_mini {

namespace {

bool ParseCoapType(const std::string &aStr, bool &aIsConfirmable)
{
    if (aStr == "con")
    {
        aIsConfirmable = true;
        return true;
    }
    if (aStr == "non-con")
    {
        aIsConfirmable = false;
        return true;
    }
    return false;
}

std::vector<uint8_t> StringToBytes(const std::string &aStr)
{
    return std::vector<uint8_t>(aStr.begin(), aStr.end());
}

} // namespace

// ---------------------------------------------------------------------------
// coap - CoAP istemci + sunucu (gerçek koddaki CLI örneğinin TEK global
// "test resource"ı, bkz. src/cli/README_COAP.md)
//
// Gerçek CLI'de aynı `coap` agent'ı hem istemci (get/put/post/delete
// gönderir) hem sunucu (kendi kaynağına gelen istekleri yanıtlar) rolünü
// oynar. ot_mini'de gerçek bir ikinci düğüm olmadığından, bir istek ancak
// hedef adres bu düğümün KENDİ adresiyse "sunucu tarafına ulaşmış" sayılır -
// böylece istemci ve sunucu davranışı, tek process içinde art arda simüle
// edilir (bkz. IsOwnUnicastAddress). Aksi halde istek gerçekçi şekilde
// zaman aşımına uğrar.
//
// Çıktı formatı README_COAP.md ile birebir uyumlu tutulur:
//   sunucu tarafı : "coap request from [<addr>] <METHOD>[ with payload: <hex>]"
//                    "coap response sent"
//   istemci tarafı: "coap response from [<addr>][ with payload: <hex>]"
// NOT: Gerçek otCoapMessageInitResponse davranışında yanıt tipi (ACK ya da
// NON) istekteki CON/NON-CON tipine göre belirlenir, ancak METOD ne olursa
// olsun (CON ya da NON-CON) her zaman bir yanıt üretilir - bu yüzden
// "coap response sent" / "coap response from [...]" satırları istek tipinden
// bağımsız olarak basılır (README_COAP.md içindeki varsayılan non-con GET
// örneğinde de yanıtın geldiği görülüyor).
//
// NOT: coaps (DTLS ile güvenli CoAP), coap observe/cancel,
// coap parameters (yeniden-gönderim zamanlamaları) ve block-wise transfer
// bilinçli olarak desteklenmiyor - bunlar gerçek bir ikinci düğüm/gerçek
// zamanlayıcı gerektiriyor ya da ot_mini'nin kapsamının dışında.
// ---------------------------------------------------------------------------
void Cli::ProcessCoap(const std::vector<std::string> &aArgs)
{
    if (aArgs.empty())
    {
        std::printf("Error: 'coap help' yaz (coap tek basina bir komut degil)\n");
        return;
    }

    const std::string &cmd = aArgs[0];

    if (cmd == "help")
    {
        // README_COAP.md ile ayni bicimde, komut basina bir satir.
        // (coaps/observe/cancel/parameters/block-wise ot_mini'de
        // desteklenmedigi icin listelenmiyor.)
        std::printf("help\n");
        std::printf("delete\n");
        std::printf("get\n");
        std::printf("post\n");
        std::printf("put\n");
        std::printf("resource\n");
        std::printf("set\n");
        std::printf("start\n");
        std::printf("stop\n");
        std::printf("Done\n");
    }
    else if (cmd == "start")
    {
        // Gercek koddaki gibi idempotent.
        mCoapServer.mIsStarted = true;
        std::printf("Done\n");
    }
    else if (cmd == "stop")
    {
        if (!mCoapServer.mIsStarted)
        {
            std::printf("Error: CoAP servisi zaten calismiyor\n");
            return;
        }

        mCoapServer.mIsStarted = false;
        std::printf("Done\n");
    }
    else if (cmd == "resource")
    {
        if (!mCoapServer.mIsStarted)
        {
            std::printf("Error: 'coap resource' icin once 'coap start' calistirilmali\n");
            return;
        }

        if (aArgs.size() == 1)
        {
            // README_COAP.md: 'coap resource' argumansiz cagrildiginda
            // sadece uri-path (ya da bos satir) basilir, ekstra metin yok.
            if (mCoapServer.mResource.mIsSet)
                std::printf("%s\n", mCoapServer.mResource.mUriPath.c_str());
            else
                std::printf("\n");

            std::printf("Done\n");
            return;
        }

        mCoapServer.mResource.mIsSet   = true;
        mCoapServer.mResource.mUriPath = aArgs[1];
        std::printf("Done\n");
    }
    else if (cmd == "set")
    {
        if (!mCoapServer.mResource.mIsSet)
        {
            std::printf("Error: 'coap set' icin once 'coap resource <uri>' ile bir kaynak tanimlanmali\n");
            return;
        }

        std::string content;
        for (size_t i = 1; i < aArgs.size(); i++)
        {
            if (!content.empty()) content += ' ';
            content += aArgs[i];
        }

        mCoapServer.mResource.mPayload = StringToBytes(content);
        std::printf("Done\n");
    }
    else if (cmd == "get" || cmd == "put" || cmd == "post" || cmd == "delete")
    {
        if (!mCoapServer.mIsStarted)
        {
            std::printf("Error: '%s' icin once 'coap start' calistirilmali\n", cmd.c_str());
            return;
        }

        if (aArgs.size() < 3)
        {
            if (cmd == "get")
                std::printf("Error: 'coap get <adres> <uri> [con|non-con]' yaz\n");
            else
                std::printf("Error: 'coap %s <adres> <uri> [con|non-con] [icerik]' yaz\n", cmd.c_str());
            return;
        }

        Ip6Address addr;
        if (!ParseIp6Address(aArgs[1], addr))
        {
            std::printf("Error: gecersiz adres '%s'\n", aArgs[1].c_str());
            return;
        }

        const std::string &uri = aArgs[2];

        bool   isConfirmable = false; // varsayilan: non-con (README_COAP.md ile ayni)
        size_t nextIdx       = 3;
        if (nextIdx < aArgs.size() && (aArgs[nextIdx] == "con" || aArgs[nextIdx] == "non-con"))
        {
            ParseCoapType(aArgs[nextIdx], isConfirmable);
            nextIdx++;
        }

        // README_COAP.md: 'get <adres> <uri-path> [type]' - GET icin payload
        // parametresi yok, sadece put/post/delete icin var.
        std::string payload;
        if (cmd != "get")
        {
            for (size_t i = nextIdx; i < aArgs.size(); i++)
            {
                if (!payload.empty()) payload += ' ';
                payload += aArgs[i];
            }
        }
        else if (nextIdx < aArgs.size())
        {
            std::printf("Error: 'coap get' fazladan arguman kabul etmez\n");
            return;
        }

        // Istek "gonderiliyor" - gercek CLI'daki "Sending coap message: Done" satirinin karsiligi.
        std::printf("Done\n");

        // ot_mini'de gercek bir ikinci dugum yok: istek ancak hedef adres
        // KENDI adresimizse yanitlanabilir. Aksi halde - gercek CoAP'ta oldugu
        // gibi - istek zaman asimina ugrar (gercek varsayilan ACK_TIMEOUT ~1s,
        // MAX_RETRANSMIT tekrarlariyla birkac saniyeye cikar; ot_mini senkron
        // calistigindan bu bekleme suresini simule ETMIYORUZ, sonucu hemen
        // bildiriyoruz).
        if (!IsOwnUnicastAddress(addr))
        {
            std::printf("coap request timed out\n");
            return;
        }

        const char *method = (cmd == "get")    ? "GET"
                              : (cmd == "put")  ? "PUT"
                              : (cmd == "post") ? "POST"
                                                 : "DELETE";

        // Sunucu tarafi: "coap request from [<addr>] <METHOD>[ with payload: <hex>]"
        std::printf("coap request from [%s] %s", addr.ToString().c_str(), method);
        if (!payload.empty())
        {
            std::printf(" with payload: %s",
                        BytesToHexString(reinterpret_cast<const uint8_t *>(payload.data()), payload.size())
                            .c_str());
        }
        std::printf("\n");

        if (!mCoapServer.mResource.mIsSet || mCoapServer.mResource.mUriPath != uri)
        {
            std::printf("coap response sent\n");
            std::printf("coap response from [%s] 4.04 Not Found\n", addr.ToString().c_str());
            return;
        }

        if (cmd == "get")
        {
            std::printf("coap response sent\n");
            // Istemci tarafi: yanit her zaman gelir (CON/NON-CON fark etmez).
            std::printf("coap response from [%s] with payload: %s\n", addr.ToString().c_str(),
                         BytesToHexString(mCoapServer.mResource.mPayload.data(),
                                          mCoapServer.mResource.mPayload.size())
                             .c_str());
            return;
        }

        // PUT/POST: kaynagin icerigini verilen payload'a gunceller.
        // DELETE: icerigi bosaltir. (ot_mini tek bir sabit kaynak destekledigi
        // icin POST'u da PUT gibi "guncelle" olarak ele aliyoruz - gercek
        // kodda POST yeni bir alt-kaynak da olusturabilir, bu ot_mini'nin
        // kapsami disinda.)
        if (cmd == "delete")
            mCoapServer.mResource.mPayload.clear();
        else
            mCoapServer.mResource.mPayload = StringToBytes(payload);

        std::printf("coap response sent\n");

        // Istemci tarafi: PUT/POST/DELETE yanitinin govdesi olmaz (2.04
        // Changed / 2.02 Deleted), bu yuzden sadece adres basilir - tipten
        // (CON/NON-CON) bagimsiz olarak yanit her zaman uretilir, tipki
        // gercek otCoapMessageInitResponse davranisinda oldugu gibi.
        (void)isConfirmable;
        std::printf("coap response from [%s]\n", addr.ToString().c_str());
    }
    else
    {
        std::printf(
            "Error: bilinmeyen 'coap' komutu '%s' (yardim: coap help; coaps/observe/cancel/parameters/"
            "block-wise transfer ot_mini'de desteklenmiyor)\n",
            cmd.c_str());
    }
}

} // namespace ot_mini