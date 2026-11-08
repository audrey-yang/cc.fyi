#include <stdio.h>
#include "directory-analyzer.h"

int main(int argc, char** argv) {
    printf("CC Disk Analyzer\n");
    if (argc != 2) {
        printf("Usage: ccda <root>\n");
        return 1;
    }
    printf("Root directory: %s\n", argv[1]);
    return discover(argv[1]);
}
