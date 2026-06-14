#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "convert.h"

void print_size(
    char* dirname, enum Unit unit, bool is_human_readable, int num_blocks
) {
    if (is_human_readable) {
        double display_size = autosize(num_blocks, &unit);
        char format[12]; 
        if (display_size < 10 && display_size > 0) {
            // For single-digit integers, make sure the ".0" appears
            snprintf(format, sizeof(format), "%s", "%#.*g%c\t%s\n");
        } else {
            snprintf(format, sizeof(format), "%s", "%.*g%c\t%s\n");
        }
        printf(
            format, display_size > 99 ? 3 : 2,
            display_size, display_unit(unit), dirname
        ); 
    } else {
        printf("%.0f\t%s\n", get_size_in_unit(num_blocks, unit), dirname);
    }
}


int get_directory_size(
    char* dirname, enum Unit unit, bool is_human_readable, bool print_all
) {
    DIR* dirp = opendir(dirname);
    struct dirent* dp;
    struct stat st;

    int num_blocks = 0;

    while ((dp = readdir(dirp)) != NULL) {
        int fullpath_len = strlen(dirname) + dp->d_namlen + 2;
        char* fullpath = malloc(fullpath_len);
        snprintf(fullpath, fullpath_len, "%s/%s", dirname, dp->d_name);

        if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            num_blocks += get_directory_size(fullpath, unit, is_human_readable, print_all);
        } else if (dp->d_type == DT_REG) {
            if (stat(fullpath, &st) == 0) {
                num_blocks += st.st_blocks;
            }
        }
        free(fullpath);
    }
    
    if (print_all) {
        print_size(dirname, unit, is_human_readable, num_blocks);
    }
    
    return num_blocks;
}

int main(int argc, char** argv) {
    char opt;

    bool is_human_readable = false;
    bool is_summary = false;
    enum Unit unit = BLOCKS;

    while ((opt = getopt(argc, argv, "hkmgts")) != -1) {
        switch (opt) {
            case 'h':
                is_human_readable = true;
            case 'k': 
                unit = KB;
                break;
            case 'm':
                unit = MB;
                break;
            case 'g':
                unit = GB;
                break;
            case 't':
                unit = TB;
                break;
            case 's':
                is_summary = true;
        }
    }

    if (optind >= argc) {
        get_directory_size(".", unit, is_human_readable, !is_summary);
    }

    for (int i = optind; i < argc; i++) {
        int dir_blocks = get_directory_size(argv[i], unit, is_human_readable, !is_summary);
        if (is_summary) {
            print_size(argv[i], unit, is_human_readable, dir_blocks);
        }
    }

    return 0;
}
