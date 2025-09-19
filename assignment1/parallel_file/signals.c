#include <signal.h>
#include <stdio.h>
#include <string.h>
#include "logger.h"

void ca_signal_handler(int sig, siginfo_t *info, void *context) {
    char message[256];
    if (sig == SIGUSR1) {
        snprintf(message, sizeof(message), "⚠️ Worker %d exceeded memory limit\n", info->si_pid);
    } else if (sig == SIGUSR2) {
        snprintf(message, sizeof(message), "✅ Worker %d completed its task\n", info->si_pid);
    }
    printf("%s", message);
    ca_log_event(message);
}

void ca_setup_signals() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = ca_signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
}