#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

void ca_worker_process(const char *filename, pid_t parent_pid) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("Worker: Failed to open file");
        exit(1);
    }

    char buffer[4096];
    ssize_t bytes_read;
    char *accum = malloc(60 * 1024 * 1024);
    size_t pos = 0;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        if (pos + bytes_read < 60 * 1024 * 1024) {
            memcpy(accum + pos, buffer, bytes_read);
            pos += bytes_read;
        }

        FILE *status = fopen("/proc/self/status", "r");
        char line[256];
        int mem_kb = 0;

        while (fgets(line, sizeof(line), status)) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                sscanf(line + 6, "%d", &mem_kb);
                break;
            }
        }
        fclose(status);

        if (mem_kb > 50000)
            kill(parent_pid, SIGUSR1);
    }

    free(accum);
    close(fd);
    kill(parent_pid, SIGUSR2);
    exit(0);
}