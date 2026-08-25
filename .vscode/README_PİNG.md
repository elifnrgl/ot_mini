
>ping fe80:0:0:0:bc73:a4d2:c9bd:8e4

 Belirtilen Link-Local IPv6 adresine varsayılan ayarlarla (1 adet paket ve 8 byte veri boyutu) tek bir ICMPv6 Echo Request gönderir.

>ping fd7b:8134:537c:259a:44c1:b31e:37ca:b0c3
 Belirtilen Mesh-Local (ULA) IPv6 adresine varsayılan ayarlarla tek bir ping gönderir. Cihazın ağdaki global veya mesh-içi yönlendirilebilir adresine ulaşılabilirliği test eder.

>ping -c 4 fd7b:8134:537c:259a:44c1:b31e:37ca:b0c3
 
 -c (count) parametresini kullanarak hedefe 4 adet ping paketi gönderir. Kodda belirtildiği üzere, 0'dan büyük geçerli bir sayı verilmelidir.


>ping -s 32 fe80:0:0:0:bc73:a4d2:c9bd:8e4


-s (size) parametresini kullanarak paketin veri boyutunu (ICMPv6 başlığı olan 8 byte hariç) 32 byte olarak ayarlar. Daha büyük veri paketlerinin ağdan geçip geçmediğini test etmek için kullanılır.

>ping -c 5 -s 64 fd7b:8134:537c:259a:44c1:b31e:37ca:b0c3

Parametreleri birleştirir. Hedefe, her biri 64 byte veri içeren toplam 5 adet ping paketi gönderir.