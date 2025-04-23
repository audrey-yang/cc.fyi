#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>

typedef struct clone_args_def
{
    int pipe_fd[2];
    char **argv;
} clone_args;

void setup_uid_gid_map(int pid)
{
    char uid_map_path[32];
    char gid_map_path[32];
    sprintf(uid_map_path, "/proc/%d/uid_map", pid);
    sprintf(gid_map_path, "/proc/%d/gid_map", pid);

    // Write UID map
    FILE *uid_map_file = fopen(uid_map_path, "w");
    if (!uid_map_file)
    {
        perror("fopen uid_map");
        exit(1);
    }
    if (fprintf(uid_map_file, "0 %d 1\n", getuid()) < 0)
    {
        perror("fprintf uid_map");
        fclose(uid_map_file);
        exit(1);
    }
    fclose(uid_map_file);

    char setgroups_path[32];
    sprintf(setgroups_path, "/proc/%d/setgroups", pid);
    FILE *setgroups_file = fopen(setgroups_path, "w");
    fprintf(setgroups_file, "deny");
    fclose(setgroups_file);

    // Write GID map
    FILE *gid_map_file = fopen(gid_map_path, "w");
    if (!gid_map_file)
    {
        perror("fopen gid_map");
        exit(1);
    }
    if (fprintf(gid_map_file, "0 %d 1\n", getgid()) < 0)
    {
        perror("fprintf gid_map");
        fclose(gid_map_file);
        exit(1);
    }
    fclose(gid_map_file);
}

int parent_clone(clone_args *args)
{
    // Ensure uid_map and gid_map are written
    close(args->pipe_fd[1]);
    char ch;
    if (read(args->pipe_fd[0], &ch, 1) < 0)
    {
        perror("read");
        return 1;
    }
    close(args->pipe_fd[0]);

    printf("Inside the container: %d of group %d\n", getuid(), getgid());

    sethostname("container", 9);

    if (mount("/proc", "/home/vagrant/c/alpine-minirootfs-3.21.3-x86/proc", "proc", 0, NULL) == -1)
    {
        perror("mount");
        return 1;
    }

    chdir("/home/vagrant/c/alpine-minirootfs-3.21.3-x86");
    if (chroot("/home/vagrant/c/alpine-minirootfs-3.21.3-x86") == -1)
    {
        perror("chroot");
        umount("/proc");
        return 1;
    }

    char **argv = args->argv;
    int pid = fork();
    if (pid == 0)
    {
        if (execvp(argv[2], argv + 2))
        {
            perror("execvp");
            umount("/proc");
            return 1;
        }
    }
    else
    {
        int status;
        if (wait(&status) == -1)
        {
            perror("wait");
            umount("/proc");
            exit(1);
        }
    }
    umount("/proc");
    return 0;
}

int main(int argc, char *argv[])
{
    const int STACK_SIZE = 65536;
    char *stack = malloc(STACK_SIZE);
    if (!stack)
    {
        perror("malloc");
        exit(1);
    }

    clone_args args;
    args.argv = argv;
    if (pipe(args.pipe_fd) < 0)
    {
        perror("pipe");
        return 1;
    }

    unsigned long flags = CLONE_NEWUSER | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS;

    int pid;
    if ((pid = clone((void *)parent_clone, stack + STACK_SIZE, flags | SIGCHLD, &args)) == -1)
    {
        perror("clone");
        free(stack);
        exit(1);
    }

    setup_uid_gid_map(pid);
    dprintf(args.pipe_fd[1], "1");

    int status;
    if (wait(&status) == -1)
    {
        perror("wait");
        free(stack);
        exit(1);
    }

    close(args.pipe_fd[0]);
    close(args.pipe_fd[1]);

    return 0;
}