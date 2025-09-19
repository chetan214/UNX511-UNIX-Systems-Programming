#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void ca_log_event(const char *message) {
    int fd = open("syslog.log", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("Log file open failed");
        return;
    }

    struct flock lock = {F_WRLCK, SEEK_SET, 0, 0, getpid()};
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("Lock failed");
        close(fd);
        return;
    }

    write(fd, message, strlen(message));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    close(fd);
}