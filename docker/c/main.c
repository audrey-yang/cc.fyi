#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>


int parent_clone(char *argv[]) {
    sethostname("container", 9);

    if (mount("/proc", "/home/vagrant/c/alpine-minirootfs-3.21.3-x86/proc", "proc", 0, NULL) == -1) {
        perror("mount");
        return 1;
    }

    chdir("/home/vagrant/c/alpine-minirootfs-3.21.3-x86");
    if (chroot("/home/vagrant/c/alpine-minirootfs-3.21.3-x86") == -1) {
        perror("chroot");
        umount("/proc");
        return 1;
    }
    
    int pid = fork();
    if (pid == 0) {
        if (execvp(argv[2], argv + 2)) {
            perror("execvp");
            umount("/proc");
            return 1;
        }
    } else {
        int status;
        if (wait(&status) == -1) {
            perror("wait");
            umount("/proc");
            exit(1);
        }
    }
    umount("/proc");
    return 0;
}

int main(int argc, char *argv[]) {
    const int STACK_SIZE = 65536;
    char* stack = malloc(STACK_SIZE);
    if (!stack) {
        perror("malloc");
        exit(1);
    }

    unsigned long flags = CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWIPC;

    if (clone((void*) parent_clone, stack + STACK_SIZE, flags | SIGCHLD, argv) == -1) {
        perror("clone");
        free(stack);
        exit(1);
    }
    
    int status;
    if (wait(&status) == -1) {
        perror("wait");
        free(stack);
        exit(1);
    }

    return 0;
}