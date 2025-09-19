#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>

#define SOCKET_PATH "/tmp/netmon.sock"
#define BUFFER_SIZE 128

using namespace std;

vector<int> client_sockets;
vector<pid_t> child_pids;
int server_sock = -1;

void cleanup_and_exit(int) {
    cout << "\nShutting down network monitor..." << endl;
    for (int sock : client_sockets) {
        write(sock, "Shut Down", 9);
        close(sock);
    }
    for (pid_t pid : child_pids) {
        kill(pid, SIGINT);
    }
    if (server_sock != -1) close(server_sock);
    unlink(SOCKET_PATH);
    exit(0);
}

int main() {
    signal(SIGINT, cleanup_and_exit);

    int num_interfaces;
    cout << "Enter number of interfaces: ";
    cin >> num_interfaces;

    vector<string> interfaces(num_interfaces);
    for (int i = 0; i < num_interfaces; ++i) {
        cout << "Enter interface " << i + 1 << ": ";
        cin >> interfaces[i];
    }

    unlink(SOCKET_PATH);

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (bind(server_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }

    listen(server_sock, num_interfaces);

    for (const string& iface : interfaces) {
        pid_t pid = fork();
        if (pid == 0) {
            execl("./intfMonitor", "intfMonitor", iface.c_str(), NULL);
            perror("execl");
            exit(1);
        } else {
            child_pids.push_back(pid);
        }
    }

    for (int i = 0; i < num_interfaces; ++i) {
        int client_fd = accept(server_sock, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }
        client_sockets.push_back(client_fd);

        char buffer[BUFFER_SIZE];
        read(client_fd, buffer, BUFFER_SIZE);
        if (string(buffer) == "Ready") {
            write(client_fd, "Monitor", 7);
            read(client_fd, buffer, BUFFER_SIZE);
            if (string(buffer) == "Monitoring") {
                cout << "Interface Monitor " << i + 1 << " started." << endl;
            }
        }
    }

    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);
        int max_fd = -1;
        for (int sock : client_sockets) {
            FD_SET(sock, &readfds);
            if (sock > max_fd) max_fd = sock;
        }

        timeval timeout = {1, 0};
        int activity = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
        if (activity > 0) {
            for (int i = 0; i < client_sockets.size(); ++i) {
                if (FD_ISSET(client_sockets[i], &readfds)) {
                    char buffer[BUFFER_SIZE];
                    int n = read(client_sockets[i], buffer, BUFFER_SIZE);
                    if (n > 0 && string(buffer) == "Link Down") {
                        cout << "Received 'Link Down' from interface " << i + 1 << endl;
                        write(client_sockets[i], "Set Link Up", 11);
                    }
                }
            }
        }
    }

    return 0;
}
