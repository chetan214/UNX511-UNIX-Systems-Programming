#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstring>
#include <csignal>
#include <pthread.h>
#include <queue>
#include <mutex>

using namespace std;

#define MAX_CLIENTS 3
#define BUF_LEN 4096

bool is_running = true;
int client_count = 0;
int conn_fds[MAX_CLIENTS];
pthread_t recv_threads[MAX_CLIENTS];
int server_fd;
mutex msg_mutex;
queue<string> message_queue;

void sigint_handler(int sig) {
    is_running = false;
}

void *receive_thread(void *arg) {
    int conn_fd = *(int *)arg;
    char buf[BUF_LEN];

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(conn_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);

    while (is_running) {
        memset(buf, 0, BUF_LEN);
        int len = read(conn_fd, buf, BUF_LEN);
        if (len > 0) {
            lock_guard<mutex> lock(msg_mutex);
            message_queue.push(string(buf, len));
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: ./server <port>" << endl;
        return 1;
    }

    signal(SIGINT, sigint_handler);

    int port = atoi(argv[1]);
    struct sockaddr_in serv_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(server_fd, MAX_CLIENTS);

    cout << "Server listening on port " << port << endl;

    while (is_running) {
        if (client_count < MAX_CLIENTS) {
            struct sockaddr_in cli_addr;
            socklen_t cli_len = sizeof(cli_addr);
            int conn_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &cli_len);
            if (conn_fd >= 0) {
                cout << "Client connected: FD=" << conn_fd << endl;
                conn_fds[client_count] = conn_fd;
                pthread_create(&recv_threads[client_count], NULL, receive_thread, &conn_fds[client_count]);
                client_count++;
            }
        }

        {
            lock_guard<mutex> lock(msg_mutex);
            while (!message_queue.empty()) {
                cout << "Message: " << message_queue.front();
                message_queue.pop();
            }
        }

        sleep(1);
    }

    for (int i = 0; i < client_count; i++) {
        write(conn_fds[i], "Quit", 5);
        pthread_join(recv_threads[i], NULL);
        close(conn_fds[i]);
    }

    close(server_fd);
    cout << "Server shutting down." << endl;
    return 0;
}
