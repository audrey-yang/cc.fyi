#include "containerize.h"

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

    // Set setgroups to deny
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

int pull_docker_image(char *image_name)
{
    char token[5012];
    if (get_token(token, image_name) < 0)
    {
        return -1;
    }
    // printf("Got Docker token: %s\n", token);

    pull_layers(image_name, token);
    return 0;
}

int run_in_container(clone_args *args)
{
    // Ensure uid_map and gid_map are written, signalled by the parent via the pipe
    close(args->pipe_fd[1]);
    char ch;
    if (read(args->pipe_fd[0], &ch, 1) < 0)
    {
        perror("read");
        return -1;
    }
    close(args->pipe_fd[0]);

    // Change host name
    sethostname("container", 9);

    // Remount the root filesystem as private
    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) < 0)
    {
        perror("remount /");
        return -1;
    }

    // Create a new directory to act as the root filesystem for the container,
    // then mount the container filesystem to it
    mkdir("/tmp/rootfs/", 0775);

    // Pull image
    if (pull_docker_image("alpine") < 0)
    {
        printf("Failed to pull image\n");
        return -1;
    }

    if (mount("/tmp/rootfs", "/tmp/rootfs", NULL, MS_BIND | MS_REC, NULL) < 0)
    {
        perror("mount fs");
        return -1;
    }

    // Change the root of the container filesystem to the new directory
    if (chroot("/tmp/rootfs") < 0)
    {
        perror("chroot");
        return -1;
    }

    // Ensure we are in the new root
    if (chdir("/") < 0)
    {
        perror("chdir");
        umount("/tmp/rootfs");
        return -1;
    }

    // Create a new tmpfs for cgroups and mount, along with controllers for cpu and memory
    if (mkdir("/sys/fs", 0755) < 0)
    {
        perror("mkdir fs");
        return -1;
    }
    if (mkdir("/sys/fs/cgroup", 0755) < 0)
    {
        perror("mkdir fs");
        return -1;
    }
    if (mount("tmpfs", "/sys/fs/cgroup", "tmpfs", 0, NULL) == -1)
    {
        perror("mount cgroup as tmpfs");
        return -1;
    }
    if (mkdir("/sys/fs/cgroup/cpu", 0755) == -1 || mkdir("/sys/fs/cgroup/memory", 0755) == -1)
    {
        perror("mkdir cgroup cpu and memory");
        return -1;
    }
    if (mkdir("/sys/fs/cgroup/cpu/container", 0755) == -1 || mkdir("/sys/fs/cgroup/memory/container", 0755) == -1)
    {
        perror("mkdir container");
        return -1;
    }

    // Limit cpu shares to 8
    char cpu_lim_path[64];
    sprintf(cpu_lim_path, "/sys/fs/cgroup/cpu/container/cpu.shares");
    FILE *cpu_lim_file = fopen(cpu_lim_path, "w");
    fprintf(cpu_lim_file, "8");
    fclose(cpu_lim_file);

    // Limit memory to 16MB
    char mem_lim_path[64];
    sprintf(mem_lim_path, "/sys/fs/cgroup/memory/container/memory.limit_in_bytes");
    FILE *mem_lim_file = fopen(mem_lim_path, "w");
    fprintf(mem_lim_file, "16000");
    fclose(mem_lim_file);

    // Add the current process to the controllers
    char cgroup_procs_path[64];
    sprintf(cgroup_procs_path, "/sys/fs/cgroup/memory/container/cgroup.procs");
    FILE *cgroup_procs_file = fopen(cgroup_procs_path, "w");
    fprintf(cgroup_procs_file, "1\n");
    fclose(cgroup_procs_file);

    sprintf(cgroup_procs_path, "/sys/fs/cgroup/memory/container/cgroup.procs");
    cgroup_procs_file = fopen(cgroup_procs_path, "w");
    fprintf(cgroup_procs_file, "1\n");
    fclose(cgroup_procs_file);

    // Mount proc filesystem to the container
    if (mount("proc", "/proc", "proc", 0, NULL) == -1)
    {
        perror("mount proc");
        return -1;
    }

    // Fork to run the specified command
    char **argv = args->argv;
    int pid = fork();
    if (pid == 0)
    {
        if (execvp(argv[2], argv + 2))
        {
            perror("execvp");
            umount("/proc");
            umount("/sys/fs/cgroup");
            return -1;
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
            return -1;
        }
    }

    umount("/proc");
    umount("/sys/fs/cgroup");
    return 0;
}