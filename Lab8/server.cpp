
#include <iostream>
#include <queue>
#include <cstring>
#include <csignal>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <cstdio>
#include "client.h"

using namespace std;

bool is_running = true;
queue<Message> messageQueue;
pthread_mutex_t lock_x;
int msgid;

void* recv_func(void* arg);

void shutdownHandler(int sig) {
    if (sig == SIGINT) {
        is_running = false;
    }
}

int main() {
    pthread_t recvThread;
    struct sigaction action;
    action.sa_handler = shutdownHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    key_t key = ftok("serverclient", 65);
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1) {
        perror("msgget");
        return 1;
    }

    pthread_mutex_init(&lock_x, NULL);
    pthread_create(&recvThread, NULL, recv_func, NULL);

    while (is_running) {
        pthread_mutex_lock(&lock_x);
        if (!messageQueue.empty()) {
            Message msg = messageQueue.front();
            messageQueue.pop();
            pthread_mutex_unlock(&lock_x);

            msg.mtype = msg.msgBuf.dest;
            msgsnd(msgid, &msg, sizeof(msg), 0);
        } else {
            pthread_mutex_unlock(&lock_x);
            usleep(50000); // Delay to reduce CPU usage
        }
    }

    // Send "Quit" to all clients before shutdown
    Message quitMsg;
    quitMsg.msgBuf.source = 0;
    strcpy(quitMsg.msgBuf.buf, "Quit");
    for (int i = 1; i <= 3; ++i) {
        quitMsg.mtype = i;
        quitMsg.msgBuf.dest = i;
        msgsnd(msgid, &quitMsg, sizeof(quitMsg), 0);
    }

    pthread_join(recvThread, NULL);
    msgctl(msgid, IPC_RMID, NULL);
    cout << "Server shutting down gracefully." << endl;
    return 0;
}

void* recv_func(void* arg) {
    while (is_running) {
        Message msg;
        ssize_t status = msgrcv(msgid, &msg, sizeof(msg), 4, 0); // blocking
        if (status != -1) {
            pthread_mutex_lock(&lock_x);
            messageQueue.push(msg);
            pthread_mutex_unlock(&lock_x);
        }
    }
    pthread_exit(NULL);
}
