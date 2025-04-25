#define _GNU_SOURCE
#define pivot_root(new, old) syscall(SYS_pivot_root, new, old)

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>

typedef struct clone_args_def
{
    int pipe_fd[2];
    char **argv;
} clone_args;

int setup_uid_gid_map(int pid)
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
        return -1;
    }
    if (fprintf(uid_map_file, "0 %d 1\n", getuid()) < 0)
    {
        perror("fprintf uid_map");
        fclose(uid_map_file);
        return -1;
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
        return -1;
    }
    if (fprintf(gid_map_file, "0 %d 1\n", getgid()) < 0)
    {
        perror("fprintf gid_map");
        fclose(gid_map_file);
        return -1;
    }
    fclose(gid_map_file);
    return 0;
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

    if (setresgid(0, 0, 0) | setresuid(0, 0, 0))
    {
        perror("setresuid/setresgid");
        return 1;
    }

    // printf("Inside the container: %d of group %d\n", getuid(), getgid());

    sethostname("container", 9);

    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) < 0)
    {
        perror("remount /");
        return 1;
    }

    mkdir("/tmp/rootfs/", 0775);

    if (mount("/home/vagrant/c/alpine-minirootfs", "/tmp/rootfs", NULL, MS_BIND | MS_REC, NULL) < 0)
    {
        perror("mount fs");
        return 1;
    }

    if (chroot("/tmp/rootfs") < 0)
    {
        perror("chroot");
        return 1;
    }

    if (chdir("/") < 0)
    {
        perror("chdir");
        umount("/tmp/rootfs");
        return 1;
    }

    if (mount("tmpfs", "/sys/fs/cgroup", "tmpfs", 0, NULL) == -1)
    {
        perror("mount cgroup as tmpfs");
        return 1;
    }

    if (mkdir("/sys/fs/cgroup/cpu", 0755) == -1 || mkdir("/sys/fs/cgroup/memory", 0755) == -1)
    {
        perror("mkdir cgroup cpu and memory");
        return 1;
    }

    if (mkdir("/sys/fs/cgroup/cpu/container", 0755) == -1 || mkdir("/sys/fs/cgroup/memory/container", 0755) == -1)
    {
        perror("mkdir container");
        return 1;
    }

    char cpu_lim_path[64];
    sprintf(cpu_lim_path, "/sys/fs/cgroup/cpu/container/cpu.shares");
    FILE *cpu_lim_file = fopen(cpu_lim_path, "w");
    fprintf(cpu_lim_file, "8");
    fclose(cpu_lim_file);

    char mem_lim_path[64];
    sprintf(mem_lim_path, "/sys/fs/cgroup/memory/container/memory.limit_in_bytes");
    FILE *mem_lim_file = fopen(mem_lim_path, "w");
    fprintf(mem_lim_file, "16000");
    fclose(mem_lim_file);

    char cgroup_procs_path[64];
    sprintf(cgroup_procs_path, "/sys/fs/cgroup/memory/container/cgroup.procs");
    FILE *cgroup_procs_file = fopen(cgroup_procs_path, "w");
    fprintf(cgroup_procs_file, "1\n");
    fclose(cgroup_procs_file);

    sprintf(cgroup_procs_path, "/sys/fs/cgroup/memory/container/cgroup.procs");
    cgroup_procs_file = fopen(cgroup_procs_path, "w");
    fprintf(cgroup_procs_file, "1\n");
    fclose(cgroup_procs_file);

    if (mount("proc", "/proc", "proc", 0, NULL) == -1)
    {
        perror("mount proc");
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
            umount("/sys/fs/cgroup");
            // umount("/");
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
            umount("/sys/fs/cgroup");
            // umount("/");
            exit(1);
        }
    }
    umount("/proc");
    umount("/sys/fs/cgroup");
    // umount("/");
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

    unsigned long flags = CLONE_NEWUSER | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWIPC;

    int pid;
    if ((pid = clone((void *)parent_clone, stack + STACK_SIZE, flags | SIGCHLD, &args)) == -1)
    {
        perror("clone");
        free(stack);
        exit(1);
    }

    if (setup_uid_gid_map(pid))
    {
        free(stack);
        if (pid)
            kill(pid, SIGKILL);
        exit(1);
    }
    if (dprintf(args.pipe_fd[1], "1") < 0)
    {
        perror("dprintf");
        free(stack);
        if (pid)
            kill(pid, SIGKILL);
        exit(1);
    }

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