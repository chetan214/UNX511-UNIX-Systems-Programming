#ifndef LOGGER_H
#define LOGGER_H

void ca_log_event(const char *message);
void ca_signal_handler(int sig, siginfo_t *info, void *context);
void ca_setup_signals();

#endif