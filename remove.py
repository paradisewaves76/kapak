import os

def remove_package(name):
    path = f"/usr/local/bin/{name}"
    if os.path.exists(path):
        os.remove(path)
        print(f"[+] {name} kaldırıldı.")
    else:
        print(f"[!] {path} bulunamadı.")