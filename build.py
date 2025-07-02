import tarfile
import os

def build_package():
    with tarfile.open("output.trkg", "w:gz") as tar:
        tar.add("control.trkg")
        for root, dirs, files in os.walk("data"):
            for file in files:
                full_path = os.path.join(root, file)
                arcname = os.path.relpath(full_path, ".")
                tar.add(full_path, arcname=arcname)
    print("[+] Paket oluşturuldu: output.trkg")