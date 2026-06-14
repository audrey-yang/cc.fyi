#include "du.h"

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
    char* dirname, 
    enum Unit unit, 
    bool is_human_readable, 
    bool print_files,
    bool follow_symlinks,
    bool has_depth_limit,
    int cur_depth,
    int max_depth,
    inode *seen_inodes
) {
    DIR* dirp = opendir(dirname);
    struct dirent* dp;
    struct stat st;

    int num_blocks = 0;

    while ((dp = readdir(dirp)) != NULL) {
        // Create new file path
        int fullpath_len = strlen(dirname) + dp->d_namlen + 2;
        char* fullpath = malloc(fullpath_len);
        snprintf(fullpath, fullpath_len, "%s/%s", dirname, dp->d_name);

        inode* inode_ptr;
        HASH_FIND_INT(seen_inodes, &dp->d_ino, inode_ptr);
        if (inode_ptr != NULL) {
            goto end_processpath;
        }

        inode_ptr = malloc(sizeof(inode));
        inode_ptr->ino = dp->d_ino;
        HASH_ADD_INT(seen_inodes, ino, inode_ptr);

        if (
            dp->d_type == DT_DIR && 
            strcmp(dp->d_name, ".") != 0 && 
            strcmp(dp->d_name, "..") != 0
        ) {
            // For directories, recurse to get size
            num_blocks += get_directory_size(
                fullpath, unit, is_human_readable, print_files, follow_symlinks, has_depth_limit, cur_depth + 1, max_depth, seen_inodes
            );
        } else if (dp->d_type == DT_REG) {
            if (stat(fullpath, &st) == 0) {
                num_blocks += st.st_blocks;
            }
            if (print_files) {
                print_size(fullpath, unit, is_human_readable, st.st_blocks);
            }
        } else if (dp->d_type == DT_LNK) {
            printf("I found a link: %s\n", fullpath);
        }

    end_processpath:
        free(fullpath);
    }
    
    if (!has_depth_limit || cur_depth <= max_depth) {
        print_size(dirname, unit, is_human_readable, num_blocks);
    }
    
    return num_blocks;
}

int main(int argc, char** argv) {
    char opt;

    bool is_human_readable = false;
    bool should_calculate_total = false;
    bool should_show_files = false;
    bool has_depth_limit = false;
    int max_depth;
    bool follow_symlinks = false;
    enum Unit unit = BLOCKS;

    while ((opt = getopt(argc, argv, "hkmgscad:L")) != -1) {
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
            case 's':
                has_depth_limit = true;
                max_depth = 0;
                break;
            case 'c':
                should_calculate_total = true;
                break;
            case 'a':
                should_show_files = true;
                break;
            case 'd':
                has_depth_limit = true;
                char* end;
                max_depth = (int) strtol(optarg, &end, 10);
                if (optarg == end) {
                    goto print_hint_and_fail;
                }
                break;
            case 'L':
                follow_symlinks = true;
            default:
                goto print_hint_and_fail;
        }
    }

    if (has_depth_limit && should_show_files) {
        goto print_hint_and_fail;
    }

    int total_blocks = 0;
    inode* seen_inodes = NULL;

    if (optind >= argc) {
        total_blocks += get_directory_size(".", unit, is_human_readable, should_show_files, follow_symlinks, has_depth_limit, 0, max_depth, seen_inodes);
    }

    for (int i = optind; i < argc; i++) {
        int dir_blocks = get_directory_size(argv[i], unit, is_human_readable, should_show_files, follow_symlinks, has_depth_limit, 0, max_depth, seen_inodes);
        total_blocks += dir_blocks;
    }

    inode *cur_inode, *tmp;
    HASH_ITER(hh, seen_inodes, cur_inode, tmp) {
        HASH_DEL(seen_inodes, cur_inode);
        free(cur_inode); 
    }

    if (should_calculate_total) {
        print_size("total", unit, is_human_readable, total_blocks);
    }

    return 0;

print_hint_and_fail:
    fprintf(stderr, "usage: ccdu [-c] [-g | -h | -k | -m] [-a | -s | -d depth] [file ...]\n");
    exit(1);
}