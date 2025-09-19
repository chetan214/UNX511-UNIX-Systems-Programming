/******************************************************************************
*  UNX511-Assignment3
*  I declare that this lab is my own work in accordance with Seneca Academic Policy.
*  No part of this assignment has been copied manually or electronically from any other source
*  (including web sites) or distributed to other students.
*
*  Name: ___________________  Student ID: _______________  Date: ____________________
******************************************************************************/
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef LOG_BUF_LEN
#define LOG_BUF_LEN 2048
#endif

static volatile bool g_is_running = true;
static int g_fd = -1;
static pthread_t g_rx_thread = 0;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static sockaddr_in g_last_sender{};
static socklen_t g_last_sender_len = sizeof(g_last_sender);
static const char *g_log_path = "server.log";

static void shutdownHandler(int) {
    g_is_running = false;
}

static int make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    return 0;
}

static void *rx_thread_func(void *arg) {
    (void)arg;
    // Open log file (rw-rw-rw-)
    int lfd = open(g_log_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (lfd < 0) {
        perror("server log open");
        return nullptr;
    }

    char buf[LOG_BUF_LEN];
    sockaddr_in rem{};
    socklen_t rlen = sizeof(rem);

    while (g_is_running) {
        pthread_mutex_lock(&g_lock);
        ssize_t n = recvfrom(g_fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&rem, &rlen);
        pthread_mutex_unlock(&g_lock);

        if (n < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                usleep(1000 * 1000); // 1s idle
                continue;
            }
            // transient error\n
            usleep(100 * 1000);
            continue;
        }
        buf[n] = '\0';

        // Track last sender so menu can reply
        pthread_mutex_lock(&g_lock);
        g_last_sender = rem;
        g_last_sender_len = rlen;
        pthread_mutex_unlock(&g_lock);

        // Append to log file
        if (write(lfd, buf, n) < 0) {
            perror("server write");
        }
    }

    close(lfd);
    return nullptr;
}

static void dump_log() {
    int fd = open(g_log_path, O_RDONLY);
    if (fd < 0) {
        perror("open log for read");
        return;
    }
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (write(STDOUT_FILENO, buf, n) < 0) break;
    }
    close(fd);
    printf("\nPress Enter to continue: ");
    fflush(stdout);
    (void)getchar();
}

int main() {
    signal(SIGINT, shutdownHandler);

    const char *bind_ip = getenv("SERVER_BIND_IP"); // optional; default 0.0.0.0
    const char *portStr = getenv("SERVER_BIND_PORT");
    int port = portStr ? atoi(portStr) : 55555;
    if (port <= 0) port = 55555;

    g_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_fd < 0) {
        perror("server socket");
        return 1;
    }
    if (make_nonblocking(g_fd) < 0) {
        perror("server nonblocking");
        return 1;
    }
    int enable = 1;
    setsockopt(g_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind_ip && *bind_ip) {
        inet_pton(AF_INET, bind_ip, &addr.sin_addr);
    }
    if (bind(g_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("server bind");
        return 1;
    }

    if (pthread_create(&g_rx_thread, nullptr, rx_thread_func, nullptr) != 0) {
        perror("server pthread_create");
        return 1;
    }

    // Menu loop
    while (g_is_running) {
        printf("\n=== Log Server Menu ===\n");
        printf("1) Set the log level on last sender\n");
        printf("2) Dump the server log file here\n");
        printf("0) Shut down\n");
        printf("Selection: ");
        fflush(stdout);

        int ch = getchar();
        if (ch == EOF) break;
        // consume trailing newline
        if (ch == '\n') continue;

        int sel = ch - '0';
        int c;
        while ((c = getchar()) != '\n' && c != EOF) { /* flush line */ }

        if (sel == 1) {
            printf("Enter new filter level (0=DEBUG,1=WARNING,2=ERROR,3=CRITICAL): ");
            fflush(stdout);
            int lvl = 0;
            if (scanf("%d", &lvl) != 1) {
                while ((c = getchar()) != '\n' && c != EOF) {}
                continue;
            }
            while ((c = getchar()) != '\n' && c != EOF) {}

            char buf[64];
            int n = snprintf(buf, sizeof(buf), "Set Log Level=%d", lvl);
            pthread_mutex_lock(&g_lock);
            if (g_last_sender_len > 0) {
                sendto(g_fd, buf, n + 1, 0, (struct sockaddr *)&g_last_sender, g_last_sender_len);
                printf("Command sent to last sender.\n");
            } else {
                printf("No sender yet. Wait for a client to log first.\n");
            }
            pthread_mutex_unlock(&g_lock);
        } else if (sel == 2) {
            dump_log();
        } else if (sel == 0) {
            g_is_running = false;
        }
    }

    if (g_rx_thread) pthread_join(g_rx_thread, nullptr);
    if (g_fd >= 0) close(g_fd);
    printf("Server stopped.\n");
    return 0;
}
