/******************************************************************************
 *  UNX511-Lab6
 *  I declare that this lab is my own work in accordance with Seneca Academic Policy.
 *  No part of this assignment has been copied manually or electronically from any other source
 *  (including web sites) or distributed to other students.
 *  
 *  Name: ___________________ Student ID: _______________ Date: ____________________
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "common.h"

int main() {
    int sockfd;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        exit(1);
    }

    printf("[Client] Connected to server.\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = read(sockfd, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;

        printf("[Client] Received: %s\n", buffer);

        if (strcmp(buffer, "Pid") == 0) {
            snprintf(buffer, BUFFER_SIZE, "This client has pid %d", getpid());
            write(sockfd, buffer, strlen(buffer));
            printf("[Client] Sent PID\n");
        } else if (strcmp(buffer, "Sleep") == 0) {
            printf("[Client] Sleeping for 5 seconds...\n");
            sleep(5);
            strcpy(buffer, "Done");
            write(sockfd, buffer, strlen(buffer));
            printf("[Client] Sent Done\n");
        } else if (strcmp(buffer, "Quit") == 0) {
            printf("[Client] Quitting...\n");
            break;
        }
    }

    close(sockfd);
    return 0;
}
