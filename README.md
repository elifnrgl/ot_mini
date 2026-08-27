# ot_mini

ot_mini, WSL (Ubuntu) ortamında geliştirilmiş olup, ağ yığınları, veri kümeleri ve komut satırı arayüzü bileşenlerinin temel mantığını anlamak ve test etmek için tasarlanmıştır.


🚀 Özellikler

Minimal OpenThread Komut Satırı Arayüzü (CLI): dataset, ifconfig, thread, state, ipaddr, ipmaddr, netdata, ping, ve coap gibi 
temel OpenThread komutlarının hafifletilmiş simülasyonu.

Modüler Modeller ve Sürücüler:

Ağ veri kümesi yönetimi (dataset.h / .cpp)

CoAP protokol desteği (coap.h, cli_coap.cpp)

IPv6 ve UDP haberleşme katmanları (ip6.h, udp.h)

Ağ veri tabloları ve ping işlevleri (netdata.h, cli_ping.cpp)

Geliştirici Dostu Yapı: VS Code ve WSL (Ubuntu) entegrasyonu ile kolay derleme ve hata ayıklama (GDB) desteği.

Derleme ve Çalıştırma Adımları

Projeyi klonla veya terminal üzerinden proje dizinine git:

Bash

cd ~/ot_mini_3/ot_mini

Kaynak kodları derle (manuel derleme için örnek komut):

Bash

g++ -std=c++11 src/*.cpp -Iinclude -o ot_mini_exec

Çalıştırılabilir dosyayı başlat:

Bash
./ot_mini_exec


💻 Kullanım

Uygulama çalıştığında karşına >  şeklinde bir komut satırı arayüzü gelecektir. Desteklenen komutlardan bazıları:

dataset — Aktif ağ veri kümesini görüntüler veya yapılandırır.

ifconfig — Ağ arayüz durumunu sorgular.

thread — Thread protokol durumunu yönetir (başlat/durdur).

ipaddr — Cihazın sahip olduğu IPv6 adreslerini listeler.

ping <ipv6_adresi> — Belirtilen adrese ICMPv6 ping isteği gönderir.

exit — CLI oturumunu sonlandırır.

📜 Lisans

Bu proje kişisel ve akademik geliştirme çalışmaları kapsamında oluşturulmuştur.
