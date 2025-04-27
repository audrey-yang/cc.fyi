#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>

#include "containerize.h"

// Helper function to clean up parent resources before shutting down
void release_parent_resources(char *stack, clone_args *args, int pid)
{
    if (pid > 0)
    {
        kill(pid, SIGKILL);
    }

    close(args->pipe_fd[0]);
    close(args->pipe_fd[1]);
    free(stack);
}

/**
 * Set up the container
 */
int main(int argc, char *argv[])
{
    // Set up arguments for container
    const int STACK_SIZE = 65536;
    char *stack = malloc(STACK_SIZE);
    if (!stack)
    {
        perror("malloc");
        return 1;
    }

    clone_args args;
    args.argv = argv;
    if (pipe(args.pipe_fd) < 0)
    {
        perror("pipe");
        return 1;
    }

    unsigned long flags = CLONE_NEWUSER | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC;

    // Call clone to create a new process for the container
    int pid;
    if ((pid = clone((void *)run_in_container, stack + STACK_SIZE, flags | SIGCHLD, &args)) == -1)
    {
        perror("clone");
        release_parent_resources(stack, &args, -1);
        return 1;
    }

    // After child is created, write to UID/GID maps so child runs as the current user
    if (setup_uid_gid_map(pid))
    {
        release_parent_resources(stack, &args, pid);
        return 1;
    }

    // Signal child via pipe once mapping is finished
    if (dprintf(args.pipe_fd[1], "1") < 0)
    {
        perror("dprintf");
        release_parent_resources(stack, &args, pid);
        return 1;
    }

    // Wait for child to finish
    int status;
    if (wait(&status) == -1)
    {
        perror("wait");
        release_parent_resources(stack, &args, pid);
        return 1;
    }

    release_parent_resources(stack, &args, -1);

    return 0;
}