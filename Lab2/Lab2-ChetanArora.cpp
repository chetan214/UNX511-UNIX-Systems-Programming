#include "pidUtil.h"

int main() {
    vector<int> pidList;
    ErrStatus status;

    // 1. Get all PIDs and print their names
    status = GetAllPids(pidList);
    if (status != Err_OK) {
        cout << "Error: " << GetErrorMsg(status) << endl;
    } else {
        for (int pid : pidList) {
            string name;
            ErrStatus nameStatus = GetNameByPid(pid, name);
            if (nameStatus != Err_OK) {
                cout << "PID: " << pid << " - Error: " << GetErrorMsg(nameStatus) << endl;
            } else {
                cout << "PID: " << pid << ", Name: " << name << endl;
            }
        }
    }

    // 2. Get name by PID 1
    string pid1Name;
    status = GetNameByPid(1, pid1Name);
    if (status != Err_OK) {
        cout << "Error: " << GetErrorMsg(status) << endl;
    } else {
        cout << "PID 1 Name: " << pid1Name << endl;
    }

    // 3. Get PID by name "Lab2"
    int lab2Pid;
    status = GetPidByName("Lab2", lab2Pid);
    if (status != Err_OK) {
        cout << "Error: " << GetErrorMsg(status) << endl;
    } else {
        cout << "Process 'Lab2' has PID: " << lab2Pid << endl;
    }

    // 4. Get PID by name "Lab22" (non-existent)
    int lab22Pid;
    status = GetPidByName("Lab22", lab22Pid);
    if (status != Err_OK) {
        cout << "Expected error (Lab22): " << GetErrorMsg(status) << endl;
    } else {
        cout << "Unexpected! 'Lab22' has PID: " << lab22Pid << endl;
    }

    return 0;
}
