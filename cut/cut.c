#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_LEN 256

typedef struct {
    int start;
    int end;
} range;

void parse_range(char* str, range* rng) {
    if (sscanf(str, "%d-%d", &rng->start, &rng->end) < 2) {
        rng->end = rng->start;
    }
}

int get_sections(char* arg, range** ranges) {
    int len = 1;
    for (int i = 0; i < strlen(arg); i++) {
        if (arg[i] == ',') {
            len += 1;
        }
    }
    *ranges = (range*) malloc(len * sizeof(range));
    char* split = strtok(arg, ",");
    int i = 0;
    while (split != NULL) {
        parse_range(split, &(*ranges)[i]);
        split = strtok(NULL, ",");
        i += 1;
    }
    return len;
}

char get_delim(char* arg) {
    return arg[2]; // -d[,]
}

int main(int argc, char** argv) {
    range* sections;
    int num_ranges;
    char delim = '\t';
    char filename[BUFFER_LEN];

    for (int i = 1; i < argc; i++ ) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            if (argv[i][1] == 'f') {
                num_ranges = get_sections(argv[i] + 2, &sections);
            } else if (argv[i][1] == 'd') {
                delim = get_delim(argv[i]);
            }
        } else {
            strcpy(filename, argv[i]);
        }
    }

    FILE *fp;
    if (strlen(filename) > 0 && filename[0] != '-') {
        fp = fopen(filename, "r");
        if (fp == NULL) {
            return 1;
        }
    } else {
        fp = stdin;
    }
    
    char buf[BUFFER_LEN];
    while (fgets(buf, BUFFER_LEN - 1, fp)) {
        int count = 1, range_ind = 0;
        bool in = false;
        if (count >= sections[range_ind].start) {
            in = true;
        }

        char* split = strtok(buf, &delim);
        char slice[BUFFER_LEN];
        
        while (split != NULL && range_ind < num_ranges) {
            if (in) {
                printf("%s", split);
            } 
            split = strtok(NULL, &delim);
            if (split != NULL && in) {
                printf("%c", delim);
            }
            count += 1;
            if (count >= sections[range_ind].start && count <= sections[range_ind].end) {
                in = true;
            } else if (count > sections[range_ind].end) {
                range_ind += 1;
                in = range_ind < num_ranges && count >= sections[range_ind].start;
            }
        }
        printf("\n");
    }

    free(sections);
    return 0;
}