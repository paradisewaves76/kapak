#!/usr/bin/env python3
import urllib.request
import json
import subprocess
import os
import typer
import hashlib
from pathlib import Path
from build import build_package
from install import install_package
from remove import remove_package
from search import search_package

app = typer.Typer()

@app.command()
def build():
    build_package()

@app.command()
def install(pkg: str):
    install_package(pkg)

@app.command()
def remove(name: str):
    remove_package(name)

@app.command()
def search(query: str):
    search_package(query)

@app.command()
def init(name: str = "my-package"):
    if os.path.exists(name):
        print(f"[!] {name} dizini zaten mevcut.")
        raise typer.Exit(code=1)

    base = Path(name)
    (base / "data/usr/local/bin").mkdir(parents=True, exist_ok=True)
    (base / "data/etc").mkdir(parents=True, exist_ok=True)

    control_path = base / "control.trkg"
    with open(control_path, "w") as f:
        f.write(
            "Name: paket-adi\n"
            "Version: 0.1.0\n"
            "Description: Açıklama buraya\n"
            "Author: Siz\n"
        )

    print(f"[+] '{name}/' paketi oluşturuldu.")
    print("  - control.trkg")
    print("  - data/usr/local/bin/")
    print("  - data/etc/")

DEFAULT_REPO_URL = "https://repopisipkg.pythonanywhere.com/packages.json"

@app.command()
def upgrade():
    print(f"[+] Repo taranıyor: {DEFAULT_REPO_URL}")

    try:
        with urllib.request.urlopen(DEFAULT_REPO_URL) as response:
            raw_data = response.read()
            package_list = json.loads(raw_data)
    except Exception as e:
        print(f"[!] Paket listesi alınamadı: {e}")
        raise typer.Exit(code=1)

    for pkg in package_list:
        name = pkg.get("name")
        version = pkg.get("version")
        url = pkg.get("url")
        sha256_expected = pkg.get("sha256")

        if not all([name, version, url, sha256_expected]):
            print(f"[!] Paket tanımı eksik: {name}")
            continue

        filename = url.split("/")[-1]
        print(f"[+] {name} ({version}) indiriliyor...")

        try:
            urllib.request.urlretrieve(url, filename)

            with open(filename, "rb") as f:
                file_hash = hashlib.sha256(f.read()).hexdigest()

            if file_hash != sha256_expected:
                print(f"[!] SHA256 doğrulaması başarısız: {name}")
                os.remove(filename)
                continue

            print(f"[✓] {name} doğrulandı.")

            install_package(filename, repo_url=DEFAULT_REPO_URL)

            os.remove(filename)

        except Exception as e:
            print(f"[!] {name} kurulamadı: {e}")
            if os.path.exists(filename):
                os.remove(filename)
            continue

if __name__ == "__main__":
    app()
