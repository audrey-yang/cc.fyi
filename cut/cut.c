#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_LEN 256

/**
 * Represents a range of field positions for column extraction.
 * 
 * Used to specify which columns to extract from input lines. The range
 * is inclusive on both ends.
 * 
 * @field start The starting column number (1-indexed)
 * @field end   The ending column number (1-indexed, inclusive)
 */
typedef struct {
    int start;
    int end;
} range;

/**
 * Parses a range string and populates a range structure.
 * 
 * Takes a string in the format "start-end" or just "start" and extracts
 * the numeric values. If only one number is provided, both start and end
 * are set to that value (representing a single column selection).
 * 
 * @param str String containing the range specification (e.g., "1-3" or "5")
 * @param rng Pointer to range structure to be populated with parsed values
 */
void parse_range(char* str, range* rng) {
    if (sscanf(str, "%d-%d", &rng->start, &rng->end) < 2) {
        rng->end = rng->start;
    }
}

/**
 * Parses a comma-separated list of field ranges and returns an array of ranges
 * 
 * Takes a string like "1-3,5,7-9" and converts it into an array of range structures.
 * Memory is dynamically allocated for the ranges array, which must be freed by the caller.
 * 
 * @param arg String containing comma-separated range specifications
 * @param ranges Pointer to range array pointer (will be allocated and populated)
 * @return Number of ranges parsed and stored in the ranges array
 */
int get_sections(char* arg, range** ranges) {
    int len = 1;
    for (int i = 0; i < strlen(arg); i++) {
        if (arg[i] == ',') {
            len += 1;
        }
    }
    *ranges = (range*) malloc(len * sizeof(range));
    if (*ranges == NULL) {
        perror("malloc");
        return -1;
    }

    char* split = strtok(arg, ",");
    int i = 0;
    while (split != NULL) {
        parse_range(split, &(*ranges)[i]);
        split = strtok(NULL, ",");
        i += 1;
    }
    return len;
}

/**
 * Extracts the delimiter character from a command-line argument.
 * 
 * Assumes the argument is in the format "-d<delim>" where <delim>
 * is the character to use as field separator. Returns the character at
 * position 2 in the argument string.
 * 
 * @param arg Command-line argument string (e.g., "-d," or "-d:")
 * @return The delimiter character
 */
char get_delim(char* arg) {
    return arg[2]; // -d[,]
}

/**
 * Main function implementing a custom version of the Unix cut command.
 * 
 * Supports the following command-line options:
 *   -f<ranges>  Specify field ranges to extract (e.g., -f1-3,5)
 *   -d<delim>   Specify field delimiter character (default: tab)
 *   <filename>  Input file (default: stdin if no file specified)
 * 
 * The program reads input line by line, splits each line using the specified
 * delimiter, and outputs only the fields specified in the ranges.
 * 
 * @return 0 on success, 1 on error
 */
int main(int argc, char** argv) {
    range* sections;
    int num_ranges;
    char delim = '\t';
    char filename[BUFFER_LEN];

    for (int i = 1; i < argc; i++ ) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            if (argv[i][1] == 'f') {
                if ((num_ranges = get_sections(argv[i] + 2, &sections)) < 0) {
                    return -1;
                }
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
            perror("fopen");
            free(sections);
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
        
        int col_count = 0;
        int last_end = 0;
        while (split != NULL) {
            bool was_in = in;
            if (in) {
                printf("%s", split);
            } 
            split = strtok(NULL, &delim);
            count += 1;
            if (count >= sections[range_ind].start && count <= sections[range_ind].end) {
                in = true;
            } else if (count > sections[range_ind].end) {
                last_end = sections[range_ind].end;
                range_ind += 1;
                in = range_ind < num_ranges && count >= sections[range_ind].start;
            }
            if (range_ind < num_ranges && was_in) {
                printf("%c", delim);
            }
            col_count++;
        }
        if (last_end != col_count) {
            printf("\n");
        }
    }

    free(sections);
    return 0;
}