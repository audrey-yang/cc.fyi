#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>

/**
 * @brief Structure to hold arguments for the container process.
 *
 * This structure contains the file descriptors for the pipe used for
 * communication between the parent and child processes, as well as
 * the arguments to be passed to the container process.
 */
typedef struct clone_args_def
{
    int pipe_fd[2];
    char **argv;
} clone_args;

/**
 * @brief Sets up the UID and GID mapping for the container process.
 *
 * @param pid The PID of the container process.
 * @return 0 on success, -1 on failure.
 */
int setup_uid_gid_map(int pid);

/**
 * @brief Runs the container process with the specified arguments.
 *
 * @param args The arguments for the container process and the pipe file descriptors.
 * @return 0 on success, 1 on failure.
 */
int run_in_container(clone_args *args);
