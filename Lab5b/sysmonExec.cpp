//sysmonExec.cpp - A system monitor using fork and exec
//
// 13-Jul-20  M. Watler         Created.

#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <iostream>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

using namespace std;
const int NUM = 2;

int systemMonitor();
bool isRunning = true;
bool isParent = true; // Distinguishes between the parent process and the child process(es)
pid_t childPid[NUM];

char *intf[] = { "lo", "ens33" };

int main()
{
    cout << endl << "parent:main: pid:" << getpid() << endl;
    for (int i = 0; i < NUM && isParent; ++i) {
        childPid[i] = fork();
        if (childPid[i] == 0) { // the child
            cout << "child:main: pid:" << getpid() << endl;
            isParent = false;
            execlp("./intfMonitor", "./intfMonitor", intf[i], NULL);
            cout << "child:main: pid:" << getpid() << " I should not get here!" << endl;
            cout << strerror(errno) << endl;
        }
    }

    if (isParent) {
        sleep(10);  // Allow children to initialize
        systemMonitor();
    }

    cout << "parent:main(" << getpid() << "): Finished!" << endl;

    return 0;
}

int systemMonitor() // run by the parent process
{
    int status = -1;
    pid_t pid = 0;

    // Send start signals
    cout << "parent:systemMonitor: sending SIGUSR1 to children..." << endl;
    for (int i = 0; i < NUM; ++i) {
        kill(childPid[i], SIGUSR1);
    }

    // Sleep for 30 seconds
    sleep(30);

    // Send stop signals
    cout << "parent:systemMonitor: sending SIGUSR2 to children..." << endl;
    for (int i = 0; i < NUM; ++i) {
        kill(childPid[i], SIGUSR2);
    }

    // Wait for children to terminate
    while ((pid = wait(&status)) > 0) {
        cout << "parent:systemMonitor: status:" << status
             << ". The child pid:" << pid << " has finished" << endl;
    }

    return 0;
}
