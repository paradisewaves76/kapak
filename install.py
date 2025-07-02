import tarfile
import os
import urllib.request
import json
import hashlib

DEFAULT_REPO_URL = "https://repopisipkg.pythonanywhere.com/packages.json"

def install_package(pkg_path_or_name, repo_url=DEFAULT_REPO_URL):
    if os.path.isfile(pkg_path_or_name):
        _install_from_file(pkg_path_or_name, repo_url)
        return
    try:
        with urllib.request.urlopen(repo_url) as response:
            raw_data = response.read()
            packages = json.loads(raw_data)
    except Exception as e:
        print(f"[!] Repo paket listesi alınamadı: {e}")
        return

    pkg = next((p for p in packages if p.get("name") == pkg_path_or_name), None)
    if not pkg:
        print(f"[!] Paket bulunamadı: {pkg_path_or_name}")
        return

    url = pkg.get("url")
    sha256_expected = pkg.get("sha256")
    filename = url.split("/")[-1]

    print(f"[+] {pkg_path_or_name} indiriliyor: {url}")
    try:
        urllib.request.urlretrieve(url, filename)
    except Exception as e:
        print(f"[!] Paket indirilemedi: {e}")
        return
    try:
        with open(filename, "rb") as f:
            file_hash = hashlib.sha256(f.read()).hexdigest()
        if file_hash != sha256_expected:
            print(f"[!] SHA256 doğrulaması başarısız: {pkg_path_or_name}")
            os.remove(filename)
            return
    except Exception as e:
        print(f"[!] Dosya doğrulama hatası: {e}")
        os.remove(filename)
        return

    _install_from_file(filename, repo_url)

    os.remove(filename)

def _install_from_file(pkg_path, repo_url=None):
    installed_files = []

    with tarfile.open(pkg_path, "r:gz") as tar:
        for member in tar.getmembers():
            if member.name == "control.trkg":
                continue
            if member.name.startswith("data/"):
                rel_path = member.name[len("data/"):]
                if rel_path:
                    member.name = rel_path
                    tar.extract(member, "/")
                    full_path = os.path.join("/", rel_path)
                    installed_files.append(full_path)

    for f in installed_files:
        if f.startswith("/usr/local/bin/") and os.path.isfile(f):
            try:
                os.chmod(f, 0o755)
                print(f"[✓] Çalıştırma izni verildi: {f}")
            except Exception as e:
                print(f"[!] İzin verilemedi: {f} -> {e}")

    print(f"[+] Paket kuruldu: {pkg_path}")

    if repo_url:
        print(f"[i] Kaynak repo: {repo_url}")
