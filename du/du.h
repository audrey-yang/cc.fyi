#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "uthash.h"
#include "convert.h"

typedef struct inode_struct {
    ino_t ino;
    UT_hash_handle hh;
} inode;
