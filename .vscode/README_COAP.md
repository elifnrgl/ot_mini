
> coap start

Done

> coap resource test-resource

Done

> coap set 30

Done

get

> coap get fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0 test-resource  

(test resource içerigini hex biçimde gösterir)

Done

coap request from [fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0] GET

coap response sent

coap response from [fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0] with payload: 3330

put (confirmable)

> coap put fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0 test-resource con payload

Done

coap request from [fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0] PUT with payload: 7061796c6f6164

coap response sent

coap response from [fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0]

post (non-con, varsayılan)

> coap post fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0 test-resource hello

Done

coap request from [fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0] POST with payload: 68656c6c6f

coap response sent

coap response from [fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0]

delete

> coap delete fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0 test-resource con

Done

coap request from [fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0] DELETE

coap response sent

coap response from [fd59:f2e8:e2f1:a2ca:15e7:c4bd:be61:f7e0]


Yanlış adrese istek (zaman aşımı)


link-local adresi (fe80:...) veya farklı bir düğümün adresi verilirse IsOwnUnicastAddress false döner:

> coap get fe80:0:0:0:60cd:929a:1ce9:3890 test-resource
Done
coap request timed out






Komut	    Söz Dizimi	Ne Yapar
coap help   -coap help	-Desteklenen alt komutları listeler

coap start	-coap start	-CoAP servisini başlatır 

coap stop	-coap stop	CoAP servisini durdurur 

coap resource	-coap resource (argümansız)	Şu an tanımlı kaynağın URI'sini gösterir, yoksa "Resource not set" der

coap resource <uri>	 -coap resource test	Sunucu tarafında bir kaynağın URI-path'ini tanımlar/değiştirir

coap set	coap set <içerik>	Tanımlı kaynağın içeriğini (payload) doğrudan ayarlar — istemciye hiç gitmeden

coap get	coap get <adres> <uri>	Kaynağın içeriğini okur, hex olarak yanıt basar

coap put	coap put <adres> <uri> [con|non-con] <içerik>	Kaynağın içeriğini değiştirir

coap post	coap post <adres> <uri> [con|non-con] <içerik>	ot_mini'de put ile aynı davranır (basitleştirme)

coap delete	coap delete <adres> <uri> [con|non-con]	Kaynağın içeriğini boşaltır
