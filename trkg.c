#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cjson/cJSON.h>


#define REPO_URL "https://repopisipkg.pythonanywhere.com/packages.json"

struct Memory { char *data; size_t size; };

static size_t cb_write(void *ptr, size_t size, size_t nmemb, void *user) {
    size_t total = size * nmemb;
    struct Memory *m = user;
    m->data = realloc(m->data, m->size + total + 1);
    memcpy(m->data + m->size, ptr, total);
    m->size += total;
    m->data[m->size] = 0;
    return total;
}

char *sha256sum(const char *file) {
    FILE *f = fopen(file,"rb"); if(!f) return NULL;
    SHA256_CTX ctx; SHA256_Init(&ctx);
    unsigned char buf[32768], h[SHA256_DIGEST_LENGTH];
    int n;
    while((n = fread(buf,1,sizeof(buf),f)) > 0)
        SHA256_Update(&ctx, buf, n);
    SHA256_Final(h, &ctx);
    static char out[65];
    for(int i=0;i<32;i++) sprintf(out+2*i,"%02x",h[i]);
    out[64] = 0;
    fclose(f);
    return out;
}

int download_url(const char *url, const char *out) {
    CURL *c = curl_easy_init(); if(!c) return 1;
    struct Memory m = {.data = malloc(1), .size = 0};
    curl_easy_setopt(c, CURLOPT_URL, url);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, cb_write);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &m);
    if(curl_easy_perform(c) != CURLE_OK) return 1;
    curl_easy_cleanup(c);
    FILE *f = fopen(out,"wb");
    fwrite(m.data,1,m.size,f);
    fclose(f); free(m.data);
    return 0;
}

void add_path_to_bashrc() {
    const char *bashrc_path = getenv("HOME");
    if (!bashrc_path) return;

    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/.bashrc", bashrc_path);

    const char *path_line = "export PATH=\"$HOME/.local/data/usr/local/bin:$PATH\"";

    FILE *f = fopen(full_path, "r");
    if (f) {
        char line[512];
        while (fgets(line, sizeof(line), f)) {
            line[strcspn(line, "\r\n")] = 0;
            if (strcmp(line, path_line) == 0) {
                fclose(f);
                printf("[*] PATH zaten .bashrc içinde.\n");
                return;
            }
        }
        fclose(f);
    }

    f = fopen(full_path, "a");
    if (!f) {
        fprintf(stderr, "[!] .bashrc dosyasına yazılamıyor.\n");
        return;
    }
    fprintf(f, "\n# Added by TRKG package manager\n%s\n", path_line);
    fclose(f);
    printf("[+] PATH ~/.bashrc dosyasına eklendi. Lütfen terminali yeniden başlatın veya 'source ~/.bashrc' çalıştırın.\n");
}

void print_banner_wout_usage() {
    printf(" /\\_/\\  TRKG Package Manager\n");
    printf("( o.o )  dreamtech.dev & FreeC-14\n");
    printf(" > ^ <  Tarafından Sevgiyle Yapıldı :)\n\n");
}

void cmd_init(const char *name) {
    if(access(name, F_OK) == 0) {
        fprintf(stderr, "[!] %s already exists\n", name);
        exit(1);
    }
    char cmd[512];
    snprintf(cmd,sizeof(cmd),"mkdir -p %s/data/usr/local/bin %s/data/etc",name,name);
    system(cmd);
    snprintf(cmd,sizeof(cmd), "%s/control.trkg", name);
    FILE *f = fopen(cmd,"w");
    fprintf(f,"Name: %s\nVersion: 0.0.0\nDescription: Açıklama\nAuthor: Siz\n", name);
    fclose(f);
    printf("[+] '%s/' created.\n", name);
}

void cmd_build() {
    printf("[+] build_package(): implement build logic here\n");
}

void cmd_list() {
    const char *home = getenv("HOME");
    if(!home) home = "/tmp";

    char path[512];
    snprintf(path, sizeof(path), "%s/.local/data/usr/local/bin", home);

    DIR *d = opendir(path);
    if(!d) {
        fprintf(stderr, "[!] Paket dizini bulunamadı: %s\n", path);
        return;
    }

    struct dirent *entry;
    printf("[*] Yüklü paketler:\n");
    while((entry = readdir(d)) != NULL) {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        printf(" - %s\n", entry->d_name);
    }
    closedir(d);
}


void cmd_info(const char *pkg) {
    const char *home = getenv("HOME");
    if(!home) home = "/tmp";

    char path[512];
    snprintf(path, sizeof(path), "%s/.local/data/%s/control.trkg", home, pkg);

    FILE *f = fopen(path, "r");
    if(!f) {
        fprintf(stderr, "[!] Paket bulunamadı veya kontrol dosyası eksik: %s\n", pkg);
        return;
    }

    printf("[*] %s paket bilgileri:\n\n", pkg);

    char line[512];
    while(fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }

    fclose(f);
}

void cmd_clean() {
    const char *home = getenv("HOME");
    if(!home) home = "/tmp";

    printf("[*] Temizlik başlıyor...\n");

    if(remove("tmp_repo.json") == 0)
        printf("[+] tmp_repo.json silindi.\n");

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -f %s/.local/data/*.trkg", home);
    int ret = system(cmd);
    if(ret == 0)
        printf("[+] Geçici paket dosyaları silindi.\n");
    else
        printf("[!] Geçici paket dosyaları silinemedi veya bulunamadı.\n");
}


void cmd_install(const char *pkg) {
    printf("[+] Installing %s...\n", pkg);
    char cmd[512];
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    snprintf(cmd, sizeof(cmd), "mkdir -p %s/.local/data && tar -xzf %s -C %s/.local/data --strip-components=1 && chmod +x %s/.local/data/usr/local/bin/*", home, pkg, home, home);
    if(system(cmd)==0) printf("[+] Installed %s\n", pkg);
    else fprintf(stderr,"[!] Install failed: %s\n", pkg);
}


void cmd_remove(const char *name) {
    printf("[+] Removing %s...\n", name);
    const char *home = getenv("HOME");
    char cmd[512];
    if(home) {
        snprintf(cmd, sizeof(cmd), "rm -rf %s/.local/data/usr/local/bin/%s", home, name);
    } else {
        snprintf(cmd, sizeof(cmd), "rm -rf /usr/local/%s", name);
    }
    if(system(cmd) == 0) printf("[+] Removed %s\n", name);
    else fprintf(stderr, "[!] Remove failed: %s\n", name);
}



void print_help() {
    printf("TRKG Paket Yöneticisi Komutları:\n");
    printf("  init <name>       : Yeni paket dizini oluşturur\n");
    printf("  build             : Paketi derler\n");
    printf("  install <file>    : Paketi kurar\n");
    printf("  remove <name>     : Paketi kaldırır\n");
    printf("  search <query>    : Paketlerde arama yapar\n");
    printf("  upgrade           : Reposundaki paketleri günceller\n");
    printf("  help              : Bu yardım mesajını gösterir\n");
}


void cmd_search(const char *query) {
    curl_global_init(CURL_GLOBAL_ALL);
    char tmp[] = "tmp_repo.json";
    if(download_url(REPO_URL, tmp)) {
        fprintf(stderr,"[!] Repo indirilemedi.\n"); return;
    }
    FILE *f = fopen(tmp,"rb");
    fseek(f,0,SEEK_END); long sz = ftell(f); rewind(f);
    char *d = malloc(sz+1); fread(d,1,sz,f); d[sz] = 0; fclose(f);
    cJSON *j = cJSON_Parse(d); free(d); remove(tmp);
    if(!j) { fprintf(stderr,"[!] JSON parse error\n"); return; }
    int n = cJSON_GetArraySize(j);
    for(int i=0;i<n;i++){
        cJSON *pkg = cJSON_GetArrayItem(j,i);
        const char *name = cJSON_GetObjectItem(pkg,"name")->valuestring;
        const char *desc = cJSON_GetObjectItem(pkg,"description")->valuestring;
        if(strstr(name, query) || (desc && strstr(desc, query)))
            printf(" * %s: %s\n", name, desc);
    }
    cJSON_Delete(j);
    curl_global_cleanup();
}

void cmd_upgrade() {
    curl_global_init(CURL_GLOBAL_ALL);
    char tmp[] = "tmp_repo.json";
    if(download_url(REPO_URL, tmp)) { fprintf(stderr,"[!] Repo hata\n"); return; }
    FILE *f = fopen(tmp,"rb");
    fseek(f,0,SEEK_END); long sz = ftell(f); rewind(f);
    char *d = malloc(sz+1); fread(d,1,sz,f); d[sz]=0; fclose(f);
    cJSON *j = cJSON_Parse(d); free(d); remove(tmp);
    if(!j) { fprintf(stderr,"[!] JSON parse error\n"); return; }
    int n = cJSON_GetArraySize(j);
    for(int i=0;i<n;i++){
        cJSON *pkg = cJSON_GetArrayItem(j,i);
        const char *name = cJSON_GetObjectItem(pkg,"name")->valuestring;
        const char *ver = cJSON_GetObjectItem(pkg,"version")->valuestring;
        const char *url = cJSON_GetObjectItem(pkg,"url")->valuestring;
        const char *sha = cJSON_GetObjectItem(pkg,"sha256")->valuestring;
        const char *fn = strrchr(url,'/')+1;
        printf("[+] %s (%s) downloading...\n", name, ver);
        if(download_url(url, fn)) { fprintf(stderr,"[!] Download failed: %s\n", name); continue; }
        char *act = sha256sum(fn);
        if(!act || strcmp(act, sha)) { fprintf(stderr,"[!] SHA mismatch: %s\n", name); remove(fn); continue; }
        printf("[✓] %s verified\n", name);
        cmd_install(fn);
        remove(fn);
    }
    cJSON_Delete(j);
    curl_global_cleanup();
}

void print_banner() {
    printf(" /\\_/\\  TRKG Package Manager\n");
    printf("( o.o )  dreamtech.dev & FreeC-14\n");
    printf(" > ^ <  Kullanım: trkg <cmd> [arg]\n\n");
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        print_banner_wout_usage();
        fprintf(stderr, "Unknown or missing command\n");
        return 1;
    }

    const char *c = argv[1];

    if(strcmp(c,"help") == 0) {
        print_banner();
        print_help();
        return 0;
    }

    print_banner_wout_usage();

    if(strcmp(c,"init") == 0 && argc == 3) {
        cmd_init(argv[2]);
    }
    else if(strcmp(c,"build") == 0) {
        cmd_build();
    }
    else if(strcmp(c,"install") == 0 && argc == 3) {
        cmd_install(argv[2]);
    }
    else if(strcmp(c,"remove") == 0 && argc == 3) {
        cmd_remove(argv[2]);
    }
    else if(strcmp(c,"search") == 0 && argc == 3) {
        cmd_search(argv[2]);
    }
    else if(strcmp(c,"upgrade") == 0) {
        cmd_upgrade();
    }
    else if(strcmp(c,"path") == 0) {
        add_path_to_bashrc();
        return 0;
    }
    else if(strcmp(c,"list") == 0) {
        cmd_list();
    }
    else if(strcmp(c,"info") == 0 && argc == 3) {
        cmd_info(argv[2]);
    }
    else if(strcmp(c,"clean") == 0) {
        cmd_clean();
    }
    else {
        fprintf(stderr, "Unknown or missing command\n");
        return 1;
    }
    

    return 0;
}
