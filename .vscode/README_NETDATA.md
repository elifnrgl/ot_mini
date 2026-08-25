>netdata publish prefix <prefix> [bayraklar] [tercih]
Ağa yeni bir IPv6 öneki (prefix) duyurur.
Bayraklar (İsteğe bağlı): p, a, d, c, r, o, s, n harflerinden oluşabilir.

>netdata publish route <prefix> [bayraklar] [tercih]
Ağa harici bir yönlendirme (route) bilgisi duyurur.
Bayraklar (İsteğe bağlı): Yalnızca s, n, a harflerinden oluşabilir.
Tercih (İsteğe bağlı): high, med, low.

>netdata unpublish <prefix>
Belirtilen IPv6 önekini (prefix) veya rotayı yerel ağ verisinden siler/yayından kaldırır.

>netdata show local
Cihazın kendi yayınladığı yerel ağ verilerini (prefix ve route'ları) okunabilir bir formatta ekrana basar.
>netdata show local -x
Aynı veriyi Hex (Onaltılık) formatta (ham TLV veri yapısı olarak) gösterir.

>netdata length
Yerel ağ verisinin şu anki güncel boyutunu bayt (byte) cinsinden verir
>netdata maxlength
Cihaz çalıştığından beri ağ verisinin ulaştığı en yüksek (maksimum) boyutu bayt cinsinden gösterir.

>netdata maxlength reset
Kaydedilen bu maksimum boyut rekorunu sıfırlar ve o anki boyuta eşitler.

Bayraklar:

Preferred (Tercih Edilen): SLAAC (otomatik adres atama) ile oluşturulan IPv6 adresinin, ağdaki iletişim için birinci derecede tercih edilen (öncelikli) adres olduğunu belirtir.

Autonomous / SLAAC (Otonom)	Cihazların bir DHCP sunucusuna sormadan, doğrudan bu öneki kullanarak kendi kendilerine otomatik bir IPv6 adresi üretebileceklerini (SLAAC) söyler.

DHCPv6 Agent (DHCP Temsilcisi)	Sınır yönlendiricisinin bu önek için bir DHCPv6 temsilcisi (Agent) gibi çalıştığını belirtir. Cihazlar DHCP isteklerini bu yönlendiriciye gönderebilir.

Configure (Yapılandır)	Ağdaki cihazların sadece adres almakla kalmayıp, bu öneke ait ek ağ ayarlarını (DNS sunucusu vb.) DHCPv6 protokolü üzerinden çekmesi gerektiğini bildirir.

Default Route (Varsayılan Rota)	Sınır yönlendiricisinin dış dünyaya (örneğin internete veya yerel Wi-Fi ağına) çıkış için ana kapı (gateway) görevi gördüğünü belirtir.

On-Mesh (Mesh İçi)	Bu öneke sahip IP adreslerinin yerel Thread (Mesh) ağına ait olduğunu belirtir. Cihazlar bu önekle dışarı çıkmadan doğrudan birbirleriyle haberleşebilir.

Stable (Kalıcı)	Bu bilginin ağda kalıcı olduğunu belirtir. Ağdaki "Lider" cihaz çökse, değişse veya cihaz yeniden başlasa bile bu yapılandırma silinmez (kalıcı bellekte tutulur).

ND-DNS (veya NAT64)	
Prefix için: Yönlendiricinin Komşu Keşfi (Neighbor Discovery) üzerinden DNS hizmeti sağladığını belirtir.


Route için: Dışarı çıkışta IPv6'dan IPv4'e çeviri yapan bir NAT64 geçidi olduğunu belirtir.