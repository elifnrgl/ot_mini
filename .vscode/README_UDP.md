The User Datagram Protocol (UDP) :

Ağda bağlantı kurmaya veya paketlerin güvenli bir şekilde ulaşıp ulaşmadığını kontrol etmeye gerek kalmadan veri paketlerini ağ üzerinden gönderen hızlı ve hafif bir iletişim protokolü.


Proje tek bir süreçten  oluştuğu ve arkada gerçek bir ağ soketi (socket API) kullanılmadığı için, bu dosya "Bir UDP soketinin CLI tarafındaki durumlarını " ve "Loopback (kendi kendine) mesaj gönderimini" taklit eder.

1. Yardımcı Sınıf ve Fonksiyonlar (Anonymous Namespace)

Dosyanın en üstündeki namespace { ... } bloğu sadece bu dosyaya özel 3 tane araç fonksiyonu barındırır:

ParsePort(...): Kullanıcının yazdığı metni ("8080" gibi) uint16_t (0 - 65535) port numarasına çevirir. Port numarasının harf içerip içermediğini ve port sınırlarını aşmadığını kontrol eder.

IsUnspecified(...): Bir IPv6 adresinin :: (tanımsız / her adresi dinleyen) olup olmadığını anlar. Adresin tüm 16 baytını kontrol eder, hepsi 0 ise true döner.

RandomEphemeralPort(): Eğer kullanıcı bind yapmadan doğrudan connect veya send yaparsa, işletim sistemlerinin yaptığı gibi arka planda rastgele bir geçici port (ephemeral port) atar. IANA standartlarına uygun olarak 49152 ile 65535 arasında bir sayı seçer.

2. Komut İşleme Mantığı (Cli::ProcessUdp)

udp ana komutundan sonra gelen ilk parametreye (aArgs[0]) bakılır ve uygun blok çalıştırılır:

A. udp help

Sadece kullanıcının çalıştırabileceği alt komutları listeler ve OpenThread standardı olan Done çıktısını basar.

B. udp open

Soketi kullanıma açar (mUdpSocket.mIsOpen = true). Yorum satırlarında belirtildiği gibi idempotent'tir; yani soket zaten açıksa hata vermez, sessizce tekrar true yapıp Done basar.

C. udp close

Soket kapalıysa hata verir (Error: soket zaten kapali).

Açıksa, soketin durumunu temsil eden struct'ı sıfırlar (mUdpSocket = UdpSocket{}).

D. udp bind [-u] <ip> <port>
S
oketi belirli bir IP adresi ve porta bağlar (dinlemeye alır).

Parametre Ayıklama:

-u parametresi gelirse bunu yoksayar (varsayılan davranıştır).

-b (backbone) veya -h (host) gelirse reddeder, çünkü bu simülatörde tek bir ağ arayüzü vardır.

Doğrulama: IP adresi :: (herhangi bir adres) veya bu düğümün kendi IPv6 adreslerinden biri (IsOwnUnicastAddress) olmalıdır. Başka bir düğümün IP'sine bind olunamaz.

Durum Güncelleme: mIsBound = true yapılır, adres ve port hafızaya yazılır.

E. udp connect <ip> <port>

Soketi belirli bir hedef IP ve porta kilitler (bağlar).

Otomatik Bind Mantığı: Eğer kullanıcı soketi daha önce bind etmediyse, kod burada devreye girer: Soketi otomatik olarak 

RandomEphemeralPort() ile rastgele bir porta bind eder.

mIsConnected = true yapılır ve hedef IP/port kaydedilir.

F. udp send ... (En Kritik Kısım)

Mesaj gönderme komutudur. İki farklı şekilde kullanılabilir:

udp send <ip> <port> <mesaj> (Açık adres belirterek)

udp send <mesaj> (Eğer daha önce udp connect yapılmışsa hedef otomatik seçilir)

Aşama 1: Parametre Çözümleme (Heuristic)

Kod, gelen argümanların ilk ikisinin IP ve port olup olmadığını dener (ParseIp6Address ve ParsePort).

Eğer IP+Port bulunursa mesaj 3. argümandan başlar.

Bulunamazsa ve soket connected durumundaysa, mesaj 1. argümandan başlar ve kaydedilmiş hedef IP/port kullanılır.

Aşama 2: Otomatik Bind

Soket henüz bind edilmediyse, yine otomatik olarak rastgele bir geçici porta bind edilir.

Aşama 3: Paket Gönderimi ve Loopback Simülasyonu

Ekrana Done yazdırılır. Çünkü UDP bağlantısız (connectionless) bir protokoldür; paket yola çıkar çıkmaz "işlem bitti" sayılır, karşı tarafın alıp almadığı beklenmez.

Paket Teslim Kontrolü (Simülasyon):

destIsUs: Hedef IP adresi bizim kendi IP'miz mi veya abone olduğumuz bir Multicast adresi mi?

localAddrOk: Soketimiz :: (her adresi dinle) durumunda mı yoksa tam bu adrese mi bind edilmiş?

portMatches: Hedef port ile bizim dinlediğimiz port eşleşiyor mu?

Eğer bu 3 şart da sağlanıyorsa, paket bu simülatörün kendisine gelmiş demektir. Kod anında ekrana paket alım satırını basar:

Plaintext

<bayt_sayisi> bytes from <ip> <port>
<mesaj_icerigi>
G. udp linksecurity

OpenThread'in orijinalinde MAC katmanı/802.15.4 şifreleme ayarı için kullanılan bir komuttur. Bu simülatörde fiziksel katman/şifreleme olmadığı için bilinçli olarak "desteklenmiyor" hatası döner.


[udp open]  --> Soketi aktif et
    │
[udp bind]  --> (İsteğe Bağlı) Yerel IP/Port'u sabitle
    │
[udp send]  --> 1. Bind edilmediyse otomatik geçici port ata
                2. "Done" yaz (UDP doğası gereği)
                3. Hedef IP+Port KENDİ IP+Port'umuzla eşleşiyor mu?
                   ├── EVET --> Ekran çıktı bas (Loopback Teslimatı)
                   └── HAYIR --> Hiçbir şey yapma (Ağda kaybolmuş kabul edilir)


udp open

udp bind :: 1234

udp send fe80:0:0:0:34f7:79b7:20d0:179a 1234 merhaba dunya