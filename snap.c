/* snap.c - Control de versiones local ultraminimalista */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>

#define SNAP_DIR ".snap"
#define MAX_MSG 256
#define MAX_PATH 512

static void die(const char *msg) {
    fprintf(stderr, "error: %s\n", msg);
    exit(1);
}

static void ensure_snap_dir(void) {
    struct stat st;
    if (stat(SNAP_DIR, &st) != 0) {
        if (mkdir(SNAP_DIR, 0755) != 0)
            die("no se pudo crear " SNAP_DIR);
    }
}

static int get_next_id(void) {
    int max = 0;
    DIR *d = opendir(SNAP_DIR);
    if (!d) return 1;
    
    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.') continue;
        int id = atoi(e->d_name);
        if (id > max) max = id;
    }
    closedir(d);
    return max + 1;
}

static void copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) return;
    
    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return;
    }
    
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
        fwrite(buf, 1, n, out);
    }
    
    fclose(in);
    fclose(out);
}

static void copy_dir_recursive(const char *src, const char *dst) {
    mkdir(dst, 0755);
    
    DIR *d = opendir(src);
    if (!d) return;
    
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        if (strcmp(e->d_name, ".snap") == 0)
            continue;
        
        char src_path[MAX_PATH];
        char dst_path[MAX_PATH];
        snprintf(src_path, MAX_PATH, "%s/%s", src, e->d_name);
        snprintf(dst_path, MAX_PATH, "%s/%s", dst, e->d_name);
        
        struct stat st;
        if (stat(src_path, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                copy_dir_recursive(src_path, dst_path);
            } else {
                copy_file(src_path, dst_path);
            }
        }
    }
    closedir(d);
}

static void cmd_init(void) {
    ensure_snap_dir();
    printf("✓ repositorio iniciado\n");
}

static void cmd_save(const char *msg) {
    ensure_snap_dir();
    
    int id = get_next_id();
    char snap_path[MAX_PATH];
    snprintf(snap_path, MAX_PATH, "%s/%d", SNAP_DIR, id);
    
    copy_dir_recursive(".", snap_path);
    
    char info_path[MAX_PATH];
    snprintf(info_path, MAX_PATH, "%s/info", snap_path);
    
    FILE *f = fopen(info_path, "w");
    if (f) {
        time_t now = time(NULL);
        fprintf(f, "%s\n%ld\n", msg ? msg : "sin mensaje", now);
        fclose(f);
    }
    
    printf("✓ snapshot #%d guardado\n", id);
}

static void cmd_list(void) {
    DIR *d = opendir(SNAP_DIR);
    if (!d) die("no hay snapshots");
    
    printf("Snapshots:\n");
    
    struct dirent *e;
    while ((e = readdir(d))) {
        if (e->d_name[0] == '.') continue;
        
        char info_path[MAX_PATH];
        snprintf(info_path, MAX_PATH, "%s/%s/info", SNAP_DIR, e->d_name);
        
        FILE *f = fopen(info_path, "r");
        if (f) {
            char msg[MAX_MSG];
            long timestamp;
            fgets(msg, MAX_MSG, f);
            fscanf(f, "%ld", &timestamp);
            fclose(f);
            
            msg[strcspn(msg, "\n")] = 0;
            time_t t = timestamp;
            char *time_str = ctime(&t);
            time_str[strcspn(time_str, "\n")] = 0;
            
            printf("  #%s - %s (%s)\n", e->d_name, msg, time_str);
        }
    }
    closedir(d);
}

static void remove_recursive(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return;
    
    if (S_ISDIR(st.st_mode)) {
        DIR *d = opendir(path);
        if (!d) return;
        
        struct dirent *e;
        while ((e = readdir(d))) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
                continue;
            
            char full[MAX_PATH];
            snprintf(full, MAX_PATH, "%s/%s", path, e->d_name);
            remove_recursive(full);
        }
        closedir(d);
        rmdir(path);
    } else {
        unlink(path);
    }
}

static void cmd_restore(int id) {
    char snap_path[MAX_PATH];
    snprintf(snap_path, MAX_PATH, "%s/%d", SNAP_DIR, id);
    
    struct stat st;
    if (stat(snap_path, &st) != 0)
        die("snapshot no existe");
    
    DIR *d = opendir(".");
    if (!d) die("no se pudo leer directorio");
    
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
            continue;
        if (strcmp(e->d_name, ".snap") == 0)
            continue;
        
        remove_recursive(e->d_name);
    }
    closedir(d);
    
    copy_dir_recursive(snap_path, ".");
    remove_recursive("info");
    
    printf("✓ restaurado a snapshot #%d\n", id);
}

static void cmd_diff(int id) {
    char snap_path[MAX_PATH];
    snprintf(snap_path, MAX_PATH, "%s/%d", SNAP_DIR, id);
    
    struct stat st;
    if (stat(snap_path, &st) != 0)
        die("snapshot no existe");
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "diff -r %s . | grep -v '.snap' | head -20", snap_path);
    system(cmd);
}

static void usage(void) {
    printf("snap - control de versiones local\n\n");
    printf("Comandos:\n");
    printf("  snap init              - iniciar repositorio\n");
    printf("  snap save [mensaje]    - guardar snapshot\n");
    printf("  snap list              - listar snapshots\n");
    printf("  snap restore <id>      - restaurar snapshot\n");
    printf("  snap diff <id>         - ver diferencias\n");
    exit(0);
}

int main(int argc, char **argv) {
    if (argc < 2) usage();
    
    const char *cmd = argv[1];
    
    if (strcmp(cmd, "init") == 0) {
        cmd_init();
    } else if (strcmp(cmd, "save") == 0) {
        const char *msg = argc > 2 ? argv[2] : NULL;
        cmd_save(msg);
    } else if (strcmp(cmd, "list") == 0) {
        cmd_list();
    } else if (strcmp(cmd, "restore") == 0) {
        if (argc < 3) die("falta id del snapshot");
        cmd_restore(atoi(argv[2]));
    } else if (strcmp(cmd, "diff") == 0) {
        if (argc < 3) die("falta id del snapshot");
        cmd_diff(atoi(argv[2]));
    } else {
        usage();
    }
    
    return 0;
}