#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int pid = fork();
    if (pid > 0) {
        execvp(argv[2], argv + 2);
        perror(argv[2]); 
    } else {
        printf("Hello\n");
    }
    return 0;
}