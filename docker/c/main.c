#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>

int child_func(char *argv[]) {
    if (execvp(argv[2], argv + 2)) {
        perror("execvp");
        return 1;
    }
    return 0;
}

int parent_clone(char *argv[]) {
    sethostname("container", 9);
    child_func(argv);
    return 0;
}

int main(int argc, char *argv[]) {
    const int STACK_SIZE = 65536;
    char* stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("malloc");
        exit(1);
    }

    unsigned long flags = CLONE_NEWUTS;

    if (clone((void*) parent_clone, stack + STACK_SIZE, flags | SIGCHLD, argv) == -1) {
        perror("clone");
        exit(1);
    }
    
    int status;
    if (wait(&status) == -1) {
        perror("wait");
        exit(1);
    }

    return 0;
}