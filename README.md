# ot_mini

ot_mini, gömülü sistemler ve IoT geliştirme süreçlerinde hafif, taşınabilir ve pratik bir simülasyon ortamı sunmak amacıyla geliştirilmiş minimal bir OpenThread CLI (Command Line Interface) ve C/C++ tabanlı yardımcı kütüphane projesidir. WSL (Ubuntu) ortamında geliştirilmiş olup, ağ yığınları, veri kümeleri ve komut satırı arayüzü bileşenlerinin temel mantığını anlamak ve test etmek için tasarlanmıştır.



ot_mini/
│
├── include/                 # Başlık dosyaları (Header files)
│   ├── cli_dataset.h        # Dataset komut satırı arayüzü tanımları
│   ├── coap.h               # CoAP protokol başlık dosyası
│   ├── dataset.h            # Ağ veri kümesi yapıları ve metotları
│   ├── hex.h                # Hex dönüşüm yardımcı araçları
│   ├── ip6.h                # IPv6 adresleme ve paket yapıları
│   ├── netdata.h            # Network Data yapıları
│   ├── tlv.h                # TLV (Type-Length-Value) çözümleyici
│   └── udp.h                # UDP başlık tanımları
│
├── src/                     # Kaynak kod dosyaları (Source files)
│   ├── cli_coap.cpp         # CoAP CLI komut işleyicileri
│   ├── cli_dataset.cpp      # Dataset CLI komut işleyicileri
│   ├── cli_ip6.cpp          # IPv6 CLI komut işleyicileri
│   ├── cli_netdata.cpp      # Network Data CLI komut işleyicileri
│   ├── cli_ping.cpp         # Ping komut işleyicisi
│   ├── cli_thread.cpp       # Thread ağ durumu komutları
│   ├── cli_udp.cpp          # UDP CLI komut işleyicileri
│   └── dataset.cpp          # Dataset mantıksal implementasyonu
│
├── .vscode/                 # VS Code yapılandırma dosyaları (tasks, launch vb.)
└── README.md                # Proje dokümantasyonu

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