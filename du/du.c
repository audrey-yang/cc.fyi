#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "convert.h"


int directory_size(char* dirname, enum Unit unit, bool is_human_readable) {
    DIR* dirp = opendir(dirname);
    struct dirent* dp;
    struct stat st;

    int num_blocks = 0;

    while ((dp = readdir(dirp)) != NULL) {
        int fullpath_len = strlen(dirname) + dp->d_namlen + 2;
        char* fullpath = malloc(fullpath_len);
        snprintf(fullpath, fullpath_len, "%s/%s", dirname, dp->d_name);

        if (dp->d_type == DT_DIR && strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            num_blocks += directory_size(fullpath, unit, is_human_readable);
        } else if (dp->d_type == DT_REG) {
            if (stat(fullpath, &st) == 0) {
                num_blocks += st.st_blocks;
            }
        }
        free(fullpath);
    }

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
    
    return num_blocks;
}

int main(int argc, char** argv) {
    char opt;

    bool is_human_readable = false;
    enum Unit unit = BLOCKS;

    while ((opt = getopt(argc, argv, "hkmgt")) != -1) {
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
        }
    }

    directory_size(
        optind >= argc ? "." : argv[optind], unit, is_human_readable
    );

    return 0;
}
