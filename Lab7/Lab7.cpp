
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>
#include <cstdio>  // For perror

#define MAX_ARGS 4
#define MAX_LEN 32

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: ./Lab7 \"command1 args\" \"command2 args\"" << endl;
        return 1;
    }

    char arg1_input[MAX_LEN], arg2_input[MAX_LEN];
    strcpy(arg1_input, argv[1]);
    strcpy(arg2_input, argv[2]);

    char* arg1[MAX_ARGS] = {NULL};
    char* arg2[MAX_ARGS] = {NULL};

    // Tokenize first argument
    int i = 0;
    char* token = strtok(arg1_input, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        arg1[i++] = token;
        token = strtok(NULL, " ");
    }

    // Tokenize second argument
    i = 0;
    token = strtok(arg2_input, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        arg2[i++] = token;
        token = strtok(NULL, " ");
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // First child - run first command and write to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execvp(arg1[0], arg1);
        perror("execvp arg1");
        exit(EXIT_FAILURE);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Second child - read from pipe and run second command
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[1]);
        close(pipefd[0]);
        execvp(arg2[0], arg2);
        perror("execvp arg2");
        exit(EXIT_FAILURE);
    }

    // Parent
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return 0;
}