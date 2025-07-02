import json
import urllib.request

LOCAL_DB_URL = "https://repopisipkg.pythonanywhere.com/packages.json"

def search_package(query):
    try:
        with urllib.request.urlopen(LOCAL_DB_URL) as response:
            raw_data = response.read()
            packages = json.loads(raw_data)

        results = [p for p in packages if query.lower() in p["name"].lower()]

        if results:
            print(f"[+] '{query}' için {len(results)} sonuç bulundu:")
            for p in results:
                print(f"- {p['name']} ({p.get('version', 'bilgi yok')}): {p.get('description', '')}")
        else:
            print(f"[?] '{query}' için sonuç bulunamadı.")

    except Exception as e:
        print(f"[!] Arama sırasında hata oluştu: {e}")
