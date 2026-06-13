#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

int directory_size(char* dirname) {
    DIR* dirp = opendir(dirname);
    struct dirent* dp;
    struct stat st;

    int size = 0;

    while ((dp = readdir(dirp)) != NULL) {
        int fullpath_len = strlen(dirname) + dp->d_namlen + 2;
        char* fullpath = malloc(fullpath_len);
        snprintf(fullpath, fullpath_len, "%s/%s", dirname, dp->d_name);
        if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            size += directory_size(fullpath);
        } else if (dp->d_type == DT_REG) {
            if (stat(fullpath, &st) == 0) {
                size += st.st_blocks;
            }
        }
        free(fullpath);
    }

    printf("%d\t%s\n", size, dirname);
    return size;
}

int main(int argc, char** argv) {
    if (argc == 1) {
        directory_size(".");
    } else {
        directory_size(argv[1]);
    }
    return 0;
}
