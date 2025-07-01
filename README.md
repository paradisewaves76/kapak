# trkg Paket Yöneticisi Rehberi

## 1. Kurulum

`trkg`'yi kurmak için GitHub üzerinden yayınlanan `.deb` paketini indirip kurabilirsiniz:

```bash
wget https://github.com/dreamtechdev230/trkg/releases/latest/download/trkg.deb
sudo dpkg -i trkg.deb
```

Kurulumdan sonra terminalde sadece:

```bash
trkg
```

komutunu yazarak çalıştığını doğrulayabilirsiniz.

---

## 2. Temel Komutlar

```bash
trkg install <paket>   # Paket yükler
trkg remove <paket>    # Paket kaldırır
trkg update            # Tüm paketleri günceller
trkg search <terim>    # Paket araması yapar
trkg build             # Paket oluşturur
trkg init [isim]       # Yeni bir paket dizini başlatır
```

---

## 3. Paket Kaldırma ve Güncelleme

### Kaldırma:
```bash
trkg remove paket_adi
```

### Güncelleme:
```bash
trkg update
```

İsteğe bağlı olarak yeniden kurulum yapılabilir:
```bash
trkg install paket_adi --reinstall
```

---

## 4. Paket Oluşturma Rehberi

### Dizin Yapısı:

```text
benim-paketim/
├── control.trkg
└── data/
```

Bu yapıda:
- `control.trkg`: Paketle ilgili meta bilgileri içerir.
- `data/`: Sisteme kopyalanacak dosyaları içerir.

---

### `control.trkg` Dosyası Örneği:

```ini
Name: benim-paketim
Version: 1.0.0
Description: Basit bir örnek paket
Author: Senin Adın
```

---

### Paket Oluşturmak için Komut:

Proje klasörünüzdeyken aşağıdaki komutu çalıştırın:

```bash
trkg build
```

Bu komut, `output.trkg` adında bir paket dosyası oluşturur. Bu dosya `control.trkg` ve `data/` klasörünün içeriğini sıkıştırarak paketler.

---

### Notlar:

- `data/` klasöründeki dosyalar hedef sisteme aynen kopyalanır.
- `control.trkg` dosyası mutlaka kök dizinde bulunmalıdır.
- Oluşturulan `.trkg` dosyası daha sonra `trkg install` ile kurulabilir.

---

## 5. `init` Komutu

Yeni bir paket başlatmak için şu komutu kullanın:

```bash
trkg init benim-paketim
```

Bu komut aşağıdaki yapıyı oluşturur:

```text
benim-paketim/
├── control.trkg
└── data/
```

Otomatik oluşturulan `control.trkg` içeriği:

```ini
Name: paket-adi
Version: 0.1.0
Description: Açıklama buraya
Author: Siz
```

---

## 6. Shell veya Python Programlarını trkg ile Paketleme Rehberi

`trkg`, herhangi bir dosyayı sistemin uygun dizinlerine kopyalayacak şekilde paketleyebilir. Bu sayede `.sh` veya `.py` scriptlerinizi kolayca yüklenebilir hale getirebilirsiniz.

---

### 1. Dizini Hazırla

Yeni bir paket dizini oluştur:

```bash
trkg init benim-programim
cd benim-programim
```

---

### 2. Script Dosyasını Yerleştir

Scriptinizi `data/` klasörüne, sistemde olması gereken yola göre kopyalayın:

Örneğin bir komut satırı aracı istiyorsanız:

```bash
cp ../my_script.sh data/usr/local/bin/my-script
chmod +x data/usr/local/bin/my-script
```

Python scripti için:

```bash
cp ../my_tool.py data/usr/local/bin/my-tool
chmod +x data/usr/local/bin/my-tool
```

> **Not:** `usr/local/bin` altına konan dosyalar, PATH'e eklidir ve doğrudan terminalden çağrılabilir.

---

### 3. `control.trkg` Dosyasını Düzenle

```ini
Name: my-script
Version: 1.0.0
Description: Basit bir shell/python aracı
Author: Senin İsmin
```

---

### 4. Paketi Oluştur

```bash
trkg build
```

Bu işlem sonunda bulunduğun dizinde `output.trkg` adlı bir paket dosyası oluşur.

---

### 5. Kurulumu Test Et

```bash
sudo trkg install output.trkg
```

Kurulduktan sonra script doğrudan çalıştırılabilir:

```bash
my-script
```
veya
```bash
my-tool
```

---

### 6. Kaldırma

```bash
sudo trkg remove my-script
```

---

### Ekstra: Python Betiğini Taşınabilir Hale Getirme

Python betiğinizin başına şu satırı eklemeyi unutmayın:

```python
#!/usr/bin/env python3
```

Ve çalıştırılabilir hale getirin:

```bash
chmod +x my_tool.py
```
