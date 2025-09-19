/******************************************************************************
*  UNX511-Assignment3
*  I declare that this lab is my own work in accordance with Seneca Academic Policy.
*  No part of this assignment has been copied manually or electronically from any other source
*  (including web sites) or distributed to other students.
*
*  Name: ___________________  Student ID: _______________  Date: ____________________
******************************************************************************/
#include "Logger.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <pthread.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int g_fd = -1;
static sockaddr_in g_server{};
static volatile bool g_is_running = false;
static volatile LOG_LEVEL g_filter = DEBUG;
static pthread_t g_rx_thread = 0;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;

// Receiver thread: accepts server commands (e.g., "Set Log Level=<n>")
static void *rx_thread_func(void *arg) {
    (void)arg;
    char buf[LOG_BUF_LEN];

    while (g_is_running) {
        pthread_mutex_lock(&g_lock);
        ssize_t n = recvfrom(g_fd, buf, sizeof(buf) - 1, 0, nullptr, nullptr);
        pthread_mutex_unlock(&g_lock);

        if (n < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // nothing available: sleep 1s per spec
                usleep(1000 * 1000);
                continue;
            }
            // transient error; brief backoff
            usleep(100 * 1000);
            continue;
        }
        buf[n] = '\0';

        int newLevel = -1;
        if (sscanf(buf, "Set Log Level=%d", &newLevel) == 1 &&
            newLevel >= DEBUG && newLevel <= CRITICAL) {
            g_filter = static_cast<LOG_LEVEL>(newLevel);
        }
    }
    return nullptr;
}

static int make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) return -1;
    return 0;
}

int InitializeLog() {
    if (g_is_running) return 0;

    // Resolve server IP/port
    const char *ip = getenv("LOG_SERVER_IP");
    const char *portStr = getenv("LOG_SERVER_PORT");
    if (!ip) ip = DEFAULT_LOG_SERVER_IP;
    int port = portStr ? atoi(portStr) : DEFAULT_LOG_SERVER_PORT;
    if (port <= 0) port = DEFAULT_LOG_SERVER_PORT;

    // Socket
    g_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_fd < 0) {
        perror("Logger socket");
        return -1;
    }
    if (make_nonblocking(g_fd) < 0) {
        perror("Logger nonblocking");
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    memset(&g_server, 0, sizeof(g_server));
    g_server.sin_family = AF_INET;
    g_server.sin_port = htons(static_cast<uint16_t>(port));
    if (inet_pton(AF_INET, ip, &g_server.sin_addr) != 1) {
        fprintf(stderr, "Logger inet_pton failed for IP %s\n", ip);
        close(g_fd);
        g_fd = -1;
        return -1;
    }

    g_is_running = true;
    if (pthread_create(&g_rx_thread, nullptr, rx_thread_func, nullptr) != 0) {
        perror("Logger pthread_create");
        close(g_fd);
        g_fd = -1;
        g_is_running = false;
        return -1;
    }
    return 0;
}

void SetLogLevel(LOG_LEVEL level) {
    g_filter = level;
}

void Log(LOG_LEVEL level, const char *file, const char *func, int line, const char *message) {
    if (!g_is_running || g_fd < 0) return;
    if (level < g_filter) return;

    // Timestamp
    time_t now = time(nullptr);
    char dt[64];
    ctime_r(&now, dt);
    // ctime_r appends newline; trim
    size_t len_dt = strnlen(dt, sizeof(dt));
    if (len_dt && dt[len_dt - 1] == '\n') dt[len_dt - 1] = '\0';

    static const char *levelStr[] = {"DEBUG", "WARNING", "ERROR", "CRITICAL"};

    char buf[LOG_BUF_LEN];
    int n = snprintf(buf, sizeof(buf), "%s %s %s:%s:%d %s\n",
                     dt, levelStr[level], file, func, line, message ? message : "");
    if (n < 0) return;
    if (n >= (int)sizeof(buf)) n = sizeof(buf) - 1;
    buf[n] = '\0';

    pthread_mutex_lock(&g_lock);
    (void)sendto(g_fd, buf, n + 1, 0, (struct sockaddr *)&g_server, sizeof(g_server));
    pthread_mutex_unlock(&g_lock);
}

void ExitLog() {
    if (!g_is_running) return;
    g_is_running = false;
    if (g_rx_thread) {
        pthread_join(g_rx_thread, nullptr);
        g_rx_thread = 0;
    }
    if (g_fd >= 0) {
        close(g_fd);
        g_fd = -1;
    }
}
