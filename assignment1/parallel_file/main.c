#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "logger.h"

// Function declarations
void ca_worker_process(const char *filename, pid_t parent_pid);
void ca_generate_binary_file(const char *filename, int size_mb);
void ca_setup_signals();

int main() {
    ca_setup_signals();

    int size1, size2, size3;
    printf("Enter file size for Worker 1 (MB): ");
    scanf("%d", &size1);
    printf("Enter file size for Worker 2 (MB): ");
    scanf("%d", &size2);
    printf("Enter file size for Worker 3 (MB): ");
    scanf("%d", &size3);

    ca_generate_binary_file("worker1.bin", size1);
    ca_generate_binary_file("worker2.bin", size2);
    ca_generate_binary_file("worker3.bin", size3);
    printf("Binary files created.\n");

    pid_t pids[3];
    const char *files[] = {"worker1.bin", "worker2.bin", "worker3.bin"};

    for (int i = 0; i < 3; ++i) {
        if ((pids[i] = fork()) == 0) {
            ca_worker_process(files[i], getppid());
        }
    }

    for (int i = 0; i < 3; ++i) wait(NULL);
    return 0;
}