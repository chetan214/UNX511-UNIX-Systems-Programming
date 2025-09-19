#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <cstring>
#include <thread>

#define SOCKET_PATH "/tmp/netmon.sock"
#define BUFFER_SIZE 128

using namespace std;

string interface_name;
int sockfd;
bool keep_running = true;

void sigint_handler(int) {
    keep_running = false;
    write(sockfd, "Done", 4);
    close(sockfd);
    exit(0);
}

string read_file(const string& path) {
    ifstream file(path);
    string content;
    getline(file, content);
    return content;
}

void monitor_interface() {
    string base = "/sys/class/net/" + interface_name + "/";
    while (keep_running) {
        string state = read_file(base + "operstate");
        string up_count = read_file(base + "carrier_up_count");
        string down_count = read_file(base + "carrier_down_count");
        string rx_bytes = read_file(base + "statistics/rx_bytes");
        string rx_dropped = read_file(base + "statistics/rx_dropped");
        string rx_errors = read_file(base + "statistics/rx_errors");
        string rx_packets = read_file(base + "statistics/rx_packets");
        string tx_bytes = read_file(base + "statistics/tx_bytes");
        string tx_dropped = read_file(base + "statistics/tx_dropped");
        string tx_errors = read_file(base + "statistics/tx_errors");
        string tx_packets = read_file(base + "statistics/tx_packets");

        cout << "Interface: " << interface_name << " state:" << state
             << " up_count:" << up_count << " down_count:" << down_count << endl;
        cout << "rx_bytes:" << rx_bytes << " rx_dropped:" << rx_dropped
             << " rx_errors:" << rx_errors << " rx_packets:" << rx_packets << endl;
        cout << "tx_bytes:" << tx_bytes << " tx_dropped:" << tx_dropped
             << " tx_errors:" << tx_errors << " tx_packets:" << tx_packets << endl;

        if (state != "up") {
            write(sockfd, "Link Down", 9);
        }

        sleep(1);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <interface>" << endl;
        return 1;
    }

    interface_name = argv[1];

    signal(SIGINT, sigint_handler);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    while (connect(sockfd, (sockaddr*)&addr, sizeof(addr)) == -1) {
        sleep(1);
    }

    write(sockfd, "Ready", 5);

    char buffer[BUFFER_SIZE];
    while (keep_running) {
        memset(buffer, 0, BUFFER_SIZE);
        int n = read(sockfd, buffer, BUFFER_SIZE);
        if (n <= 0) break;

        if (string(buffer) == "Monitor") {
            write(sockfd, "Monitoring", 10);
            monitor_interface();
        } else if (string(buffer) == "Set Link Up") {
            string cmd = "sudo ip link set " + interface_name + " up";
            system(cmd.c_str());
        } else if (string(buffer) == "Shut Down") {
            sigint_handler(0);
        }
    }

    return 0;
}
