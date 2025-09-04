#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_LEN 256

int main(int argc, char** argv) {
    int section = 0;
    if (argc == 3) {
        if (argv[1][1] == 'f') {
            section = atoi(argv[1] + 2);
        }
    }
    const char *delim = "\t";

    FILE *fp;
    fp = fopen(argv[argc-1], "r");
    if (fp == NULL) {
        return 1;
    }

    char buf[BUFFER_LEN];
    while (fgets(buf, BUFFER_LEN - 1, fp)) {
        char* split = strtok(buf, delim);
        char slice[BUFFER_LEN];
        int count = 1;
        while (count < section && split != NULL) {
            split = strtok (NULL, delim);
            count += 1;
        }
        printf("%s\n", split);
    }

    return 0;
}