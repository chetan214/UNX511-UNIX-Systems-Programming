
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "common.h"

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    unlink(SOCKET_PATH);

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("[Server] Waiting for connection...\n");
    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("accept");
        exit(1);
    }

    // Step 1: Send "Pid"
    strcpy(buffer, "Pid");
    write(client_fd, buffer, strlen(buffer));
    printf("[Server] Sent: %s\n", buffer);

    // Step 2: Read PID response
    memset(buffer, 0, BUFFER_SIZE);
    read(client_fd, buffer, BUFFER_SIZE);
    printf("[Server] Received: %s\n", buffer);

    // Step 3: Send "Sleep"
    strcpy(buffer, "Sleep");
    write(client_fd, buffer, strlen(buffer));
    printf("[Server] Sent: %s\n", buffer);

    // Step 4: Wait for "Done"
    memset(buffer, 0, BUFFER_SIZE);
    read(client_fd, buffer, BUFFER_SIZE);
    printf("[Server] Received: %s\n", buffer);

    // Step 5: Send "Quit"
    strcpy(buffer, "Quit");
    write(client_fd, buffer, strlen(buffer));
    printf("[Server] Sent: %s\n", buffer);

    // Cleanup
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    printf("[Server] Connection closed. Exiting.\n");

    return 0;
}
