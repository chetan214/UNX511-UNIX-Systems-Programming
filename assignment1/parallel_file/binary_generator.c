#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void ca_generate_binary_file(const char *filename, int size_mb) {
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("File creation failed");
        exit(1);
    }

    char buffer[1024 * 1024];
    srand(time(NULL));
    for (int i = 0; i < sizeof(buffer); ++i)
        buffer[i] = rand() % 256;

    for (int i = 0; i < size_mb; ++i) {
        write(fd, buffer, sizeof(buffer));
    }

    close(fd);
}