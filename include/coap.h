#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace ot_mini {

// ---------------------------------------------------------------------------
// CoapResource
//
// Gerçek OpenThread CLI örneğindeki (src/cli/README_COAP.md) TEK "test
// resource" kavramının karşılığı: `coap resource <uri>` ile URI-path atanır,
// `coap set <icerik>` ile içeriği değiştirilir. Gerçek CLI örneği de yalnızca
// BİR kaynağı destekler (birden fazla kaynak için otCoapAddResource ile
// uygulama kodu yazmak gerekir) - ot_mini de bu sınırı bilerek koruyor.
// ---------------------------------------------------------------------------
struct CoapResource
{
    bool                 mIsSet = false; // `coap resource` hiç çağrılmadıysa sunucu her isteğe 4.04 döner
    std::string          mUriPath;
    std::vector<uint8_t> mPayload; // `coap set`/`coap put`/`coap post` ile değiştirilir
};

// ---------------------------------------------------------------------------
// CoapServer
//
// Gerçek koddaki CoAP agent'ının (`coap start`/`coap stop`) çok sadeleştirilmiş
// karşılığı. ot_mini'de istemci VE sunucu tarafı aynı Cli örneğinde, aynı
// "test resource" üzerinden simüle edilir (bkz. cli_coap.cpp) - gerçek bir
// ikinci düğüm olmadığından yalnızca kendi adreslerine yapılan istekler
// yanıtlanabilir.
// ---------------------------------------------------------------------------
struct CoapServer
{
    bool         mIsStarted = false;
    CoapResource mResource;
};

} // namespace ot_mini
