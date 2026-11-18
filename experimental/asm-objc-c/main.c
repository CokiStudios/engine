#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

extern size_t exp_asm_strlen(const char* s);
extern uint64_t exp_asm_fnv1a64(const void* data, size_t len);
extern int exp_run_native_window(const char* title, const char* card_title, const char* card_text,
                                 int w, int h, int headless_smoke);

typedef struct {
    char name[64];
    double num_val;
    char str_val[256];
    int is_str;
} ExpVar;

typedef struct {
    ExpVar vars[64];
    int var_count;
    char window_title[128];
    char card_title[128];
    char card_text[256];
    int win_w;
    int win_h;
    int has_window;
    char stdout_buf[4096];
} ExpVM;

static void vm_init(ExpVM* vm) {
    memset(vm, 0, sizeof(*vm));
    snprintf(vm->window_title, sizeof(vm->window_title), "Looping Experimental (Asm+ObjC+C)");
    snprintf(vm->card_title, sizeof(vm->card_title), "Zero-Dependency Engine");
    snprintf(vm->card_text, sizeof(vm->card_text), "Running on Assembly + C11 + objc_msgSend");
    vm->win_w = 800;
    vm->win_h = 500;
}

static void vm_append_out(ExpVM* vm, const char* line) {
    size_t cur = exp_asm_strlen(vm->stdout_buf);
    size_t rem = sizeof(vm->stdout_buf) - cur - 2;
    if (rem > 0) {
        snprintf(vm->stdout_buf + cur, rem, "%s\n", line);
    }
}

static ExpVar* vm_find_or_add(ExpVM* vm, const char* name) {
    for (int i = 0; i < vm->var_count; ++i) {
        if (strcmp(vm->vars[i].name, name) == 0) return &vm->vars[i];
    }
    if (vm->var_count >= 64) return NULL;
    ExpVar* v = &vm->vars[vm->var_count++];
    memset(v, 0, sizeof(*v));
    snprintf(v->name, sizeof(v->name), "%s", name);
    return v;
}

static char* trim_ws(char* s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n') ++s;
    size_t n = exp_asm_strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\r' || s[n - 1] == '\n')) {
        s[--n] = '\0';
    }
    return s;
}

static int extract_quoted(const char* src, char* dst, size_t cap) {
    const char* q1 = strchr(src, '"');
    if (!q1) return 0;
    const char* q2 = strchr(q1 + 1, '"');
    if (!q2) return 0;
    size_t len = (size_t)(q2 - (q1 + 1));
    if (len >= cap) len = cap - 1;
    memcpy(dst, q1 + 1, len);
    dst[len] = '\0';
    return 1;
}

static void vm_exec_code(ExpVM* vm, const char* src) {
    if (!src) return;
    char* copy = strdup(src);
    if (!copy) return;

    char* saveptr = NULL;
    for (char* raw = strtok_r(copy, "\n", &saveptr); raw; raw = strtok_r(NULL, "\n", &saveptr)) {
        char* line = trim_ws(raw);
        if (!*line || *line == '#' || strncmp(line, "//", 2) == 0) continue;

        if (strncmp(line, "say(", 4) == 0 || strncmp(line, "print(", 6) == 0) {
            char msg[256];
            if (extract_quoted(line, msg, sizeof(msg))) {
                vm_append_out(vm, msg);
            } else {
                const char* lp = strchr(line, '(');
                const char* rp = strrchr(line, ')');
                if (lp && rp && rp > lp + 1) {
                    char key[64];
                    size_t klen = (size_t)(rp - lp - 1);
                    if (klen >= sizeof(key)) klen = sizeof(key) - 1;
                    memcpy(key, lp + 1, klen);
                    key[klen] = '\0';
                    char* tk = trim_ws(key);
                    ExpVar* v = NULL;
                    for (int i = 0; i < vm->var_count; ++i) {
                        if (strcmp(vm->vars[i].name, tk) == 0) { v = &vm->vars[i]; break; }
                    }
                    if (v) {
                        char numbuf[128];
                        if (v->is_str) vm_append_out(vm, v->str_val);
                        else {
                            snprintf(numbuf, sizeof(numbuf), "%.6g", v->num_val);
                            vm_append_out(vm, numbuf);
                        }
                    }
                }
            }
        } else if (strncmp(line, "val ", 4) == 0 || strncmp(line, "mut ", 4) == 0) {
            char* eq = strchr(line + 4, '=');
            if (eq) {
                *eq = '\0';
                char* vname = trim_ws(line + 4);
                char* vval = trim_ws(eq + 1);
                ExpVar* v = vm_find_or_add(vm, vname);
                if (v) {
                    char qbuf[256];
                    if (extract_quoted(vval, qbuf, sizeof(qbuf))) {
                        v->is_str = 1;
                        snprintf(v->str_val, sizeof(v->str_val), "%s", qbuf);
                    } else {
                        v->is_str = 0;
                        v->num_val = strtod(vval, NULL);
                    }
                }
            }
        } else if (strncmp(line, "window.create(", 14) == 0) {
            vm->has_window = 1;
            extract_quoted(line, vm->window_title, sizeof(vm->window_title));
        } else if (strstr(line, "card") != NULL) {
            vm->has_window = 1;
            extract_quoted(line, vm->card_title, sizeof(vm->card_title));
        }
    }
    free(copy);
}

static char* read_file_all(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0 || sz > 4 * 1024 * 1024) { fclose(f); return NULL; }
    char* buf = (char*)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t n = fread(buf, 1, (size_t)sz, f);
    buf[n] = '\0';
    fclose(f);
    return buf;
}

static void send_http_response(int client_fd, int code, const char* status,
                               const char* ctype, const char* body) {
    char header[512];
    size_t blen = exp_asm_strlen(body);
    int hlen = snprintf(
        header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "X-Looping-Runtime: Experimental-Asm-ObjC-C\r\n\r\n",
        code, status, ctype, blen
    );
    if (hlen > 0) (void)write(client_fd, header, (size_t)hlen);
    if (blen > 0) (void)write(client_fd, body, blen);
}

static int serve_http(int port, const char* default_script, int max_requests) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        perror("socket");
        return 1;
    }
    int yes = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons((uint16_t)port);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(srv);
        return 1;
    }
    if (listen(srv, 16) < 0) {
        perror("listen");
        close(srv);
        return 1;
    }

    printf("[looping-exp] Zero-Dependency HTTP Server (Assembly + ObjC-ABI + C11) listening on http://127.0.0.1:%d\n", port);
    fflush(stdout);

    int served = 0;
    while (max_requests <= 0 || served < max_requests) {
        int cfd = accept(srv, NULL, NULL);
        if (cfd < 0) break;

        char req[8192];
        ssize_t n = recv(cfd, req, sizeof(req) - 1, 0);
        if (n > 0) {
            req[n] = '\0';
            if (strncmp(req, "GET /health", 11) == 0) {
                char payload[512];
                const char* tag = "Looping-Asm-ObjC-C-ZeroDep";
                uint64_t h = exp_asm_fnv1a64(tag, exp_asm_strlen(tag));
                snprintf(payload, sizeof(payload),
                         "{\"ok\":true,\"runtime\":\"Assembly+ObjC+C\",\"dependencies\":\"none\",\"fnv1a\":\"0x%llx\"}\n",
                         (unsigned long long)h);
                send_http_response(cfd, 200, "OK", "application/json; charset=utf-8", payload);
            } else if (strncmp(req, "POST /run", 9) == 0) {
                const char* body = strstr(req, "\r\n\r\n");
                body = body ? (body + 4) : "";
                ExpVM vm;
                vm_init(&vm);
                vm_exec_code(&vm, body);
                uint64_t h = exp_asm_fnv1a64(vm.stdout_buf, exp_asm_strlen(vm.stdout_buf));
                char resp[4608];
                snprintf(resp, sizeof(resp),
                         "{\"ok\":true,\"fnv1a\":\"0x%llx\",\"output\":%s}\n",
                         (unsigned long long)h,
                         vm.stdout_buf[0] ? "\"executed\"" : "\"empty\"");
                /* Include actual stdout cleanly in plain text if Accept asks or json */
                (void)resp;
                send_http_response(cfd, 200, "OK", "text/plain; charset=utf-8",
                                   vm.stdout_buf[0] ? vm.stdout_buf : "[ok]\n");
            } else {
                ExpVM vm;
                vm_init(&vm);
                if (default_script) {
                    char* src = read_file_all(default_script);
                    if (src) { vm_exec_code(&vm, src); free(src); }
                } else {
                    vm_exec_code(&vm, "say(\"Looping Experimental Runtime Online (Zero Python / Zero Node)\")");
                }
                char html[6144];
                snprintf(html, sizeof(html),
                         "<!doctype html><html><body style='background:#070b14;color:#f8fafc;font-family:monospace;padding:32px'>"
                         "<h1 style='color:#00f5d4'>Looping Experimental (Assembly + ObjC + C11)</h1>"
                         "<p>Zero Python • Zero Node.js • Zero C++ stdlib</p>"
                         "<pre style='background:#0f172a;padding:16px;border-radius:8px'>%s</pre>"
                         "</body></html>\n",
                         vm.stdout_buf);
                send_http_response(cfd, 200, "OK", "text/html; charset=utf-8", html);
            }
            ++served;
        }
        close(cfd);
    }

    close(srv);
    return 0;
}

int main(int argc, char** argv) {
    if (argc >= 2 && strcmp(argv[1], "--test") == 0) {
        const char* probe = "CokiStudios";
        size_t len = exp_asm_strlen(probe);
        uint64_t h = exp_asm_fnv1a64(probe, len);
        ExpVM vm;
        vm_init(&vm);
        vm_exec_code(&vm, "val x = 42\nsay(\"Hello Zero-Dep\")\nsay(x)\nwindow.create(\"ASM Win\", 800, 500)\n");
        int win_rc = exp_run_native_window(vm.window_title, vm.card_title, vm.card_text, vm.win_w, vm.win_h, 1);
        int ok = (len == 11 && h != 0 && win_rc == 0 && strstr(vm.stdout_buf, "Hello Zero-Dep") != NULL &&
                  strstr(vm.stdout_buf, "42") != NULL);
        printf("[looping-exp --test] strlen=%zu fnv1a=0x%llx win_rc=%d -> %s\n",
               len, (unsigned long long)h, win_rc, ok ? "PASS" : "FAIL");
        return ok ? 0 : 1;
    }

    if (argc >= 2 && strcmp(argv[1], "--serve") == 0) {
        int port = (argc >= 3) ? atoi(argv[2]) : 8787;
        const char* script = NULL;
        int max_req = 0;
        for (int i = 3; i < argc; ++i) {
            if (strcmp(argv[i], "--script") == 0 && i + 1 < argc) script = argv[++i];
            else if (strcmp(argv[i], "--once") == 0) max_req = 1;
        }
        return serve_http(port, script, max_req);
    }

    if (argc >= 3 && strcmp(argv[1], "--window") == 0) {
        char* src = read_file_all(argv[2]);
        ExpVM vm;
        vm_init(&vm);
        if (src) { vm_exec_code(&vm, src); free(src); }
        printf("%s", vm.stdout_buf);
        return exp_run_native_window(vm.window_title, vm.card_title, vm.card_text, vm.win_w, vm.win_h, 0);
    }

    if (argc >= 2) {
        char* src = read_file_all(argv[1]);
        if (!src) {
            fprintf(stderr, "Cannot open script: %s\n", argv[1]);
            return 1;
        }
        ExpVM vm;
        vm_init(&vm);
        vm_exec_code(&vm, src);
        free(src);
        printf("%s", vm.stdout_buf);
        if (vm.has_window) {
            return exp_run_native_window(vm.window_title, vm.card_title, vm.card_text, vm.win_w, vm.win_h, 0);
        }
        return 0;
    }

    printf("Looping Experimental Micro-Runtime (Assembly + Objective-C ABI + C11)\n");
    printf("Usage:\n");
    printf("  %s --test\n", argv[0]);
    printf("  %s --serve <port> [--script file.loop] [--once]\n", argv[0]);
    printf("  %s --window <file.loop>\n", argv[0]);
    printf("  %s <file.loop>\n", argv[0]);
    return 0;
}
