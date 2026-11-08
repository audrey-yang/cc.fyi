#include "directory-analyzer.h"

void print_directory_entry(struct dirent* direntp, char* fullpath) {
    printf("%s (", fullpath);
    switch (direntp->d_type) {
        case DT_BLK: 
            printf("BLOCK DEVICE");
            break;
        case DT_CHR: 
            printf("CHARACTER DEVICE");
            break;
        case DT_DIR:
            printf("DIRECTORY");
            break;
        case DT_FIFO:
            printf("NAMED PIPE");
            break;
        case DT_LNK:
            printf("SYMBOLIC LINK");
            break;
        case DT_REG: 
            printf("REGULAR FILE");
            break;
        case DT_SOCK: 
            printf("UNIX DOMAIN SOCKET");
            break;
        case DT_UNKNOWN:
        default: 
            printf("UNKNOWN");
            break;
    }
    printf(")\n");
}

int discover(const char* dir_name) {
    int ret = 1;
    DIR* dirp = opendir(dir_name);
    if (dirp == NULL) { 
        perror("opendir: ");
        goto exit_opendir_error; 
    }

    struct dirent* direntp;
    // Loop through directory entries
    while ((direntp = readdir(dirp))) {
        if (errno) { 
            perror("readdir: ");
            goto exit_error; 
        }
        // Skip over . and ..
        if (strcmp(direntp->d_name, ".") == 0 || strcmp(direntp->d_name, "..") == 0) { continue; }

        char fullpath[strlen(dir_name) + 1 + strlen(direntp->d_name) + 1];
        if (snprintf(fullpath, sizeof(fullpath), "%s/%s", dir_name, direntp->d_name) < 0) {
            perror("snprintf: ");
            goto exit_error;
        }

        print_directory_entry(direntp, fullpath);
        if (direntp->d_type == DT_DIR) {
            if (discover(fullpath)) { 
                // If underlying directories error, treat as a warning and continue
                errno = 0;
            }
        }
    }
    
exit_normal:
    ret = 0;
exit_error:
    if (closedir(dirp)) { perror("closedir: "); }
exit_opendir_error:
    return ret;
}