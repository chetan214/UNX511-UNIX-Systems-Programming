// MemMonitor.cpp - UNX511 Lab 1 (C++98 compatible + sorted by memory)

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <cstring>
#include <cctype>
#include <iomanip>
#include <cstdlib>
#include <algorithm>  // for sort()

using namespace std;

// Structure to hold process info
struct ProcessInfo {
    string pid;
    string name;
    int memory_kb;
};

// Check if directory name is a PID (all digits)
bool is_pid(const string& name) {
    for (size_t i = 0; i < name.length(); ++i) {
        if (!isdigit(name[i])) return false;
    }
    return true;
}

// Get VmRSS from /proc/[pid]/status
int get_memory_kb(const string& pid) {
    string path = "/proc/" + pid + "/status";
    ifstream file(path.c_str());
    if (!file.is_open()) return 0;

    string line;
    while (getline(file, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            string label, unit;
            int value;
            istringstream iss(line);
            iss >> label >> value >> unit;
            return value;
        }
    }
    return 0;
}

// Get process name from /proc/[pid]/status
string get_process_name(const string& pid) {
    string path = "/proc/" + pid + "/status";
    ifstream file(path.c_str());
    if (!file.is_open()) return "unknown";

    string line;
    while (getline(file, line)) {
        if (line.substr(0, 5) == "Name:") {
            string label, name;
            istringstream iss(line);
            iss >> label >> name;
            return name;
        }
    }
    return "unknown";
}

// Comparison function for sorting
bool compare_by_memory(const ProcessInfo& a, const ProcessInfo& b) {
    return a.memory_kb > b.memory_kb;  // descending
}

int main() {
    DIR* dir = opendir("/proc");
    if (dir == NULL) {
        perror("Cannot open /proc");
        return 1;
    }

    struct dirent* entry;
    vector<ProcessInfo> processes;

    while ((entry = readdir(dir)) != NULL) {
        string pid = entry->d_name;
        if (is_pid(pid)) {
            int mem_kb = get_memory_kb(pid);
            if (mem_kb > 10000) {
                string name = get_process_name(pid);
                ProcessInfo info;
                info.pid = pid;
                info.name = name;
                info.memory_kb = mem_kb;
                processes.push_back(info);
            }
        }
    }

    closedir(dir);

    // Sort processes by memory usage
    sort(processes.begin(), processes.end(), compare_by_memory);

    // Display results
    system("clear");
    cout << left << setw(8) << "PID" << setw(25) << "Name" << "Memory (kB)" << endl;
    cout << "---------------------------------------------------------" << endl;

    for (size_t i = 0; i < processes.size(); ++i) {
        cout << left << setw(8) << processes[i].pid
             << setw(25) << processes[i].name
             << processes[i].memory_kb << endl;
    }

    return 0;
}
