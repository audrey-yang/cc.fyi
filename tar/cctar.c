#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

int list_tar();

int main(int argc, char** argv) {
	printf("CC TAR\n");
	// process -t
	list_tar();
	return 0;
}

int list_tar() {
	char buf[512];
	int amount_to_read = -1, already_read = 0;
	while (read(STDIN_FILENO, buf, 512)) {
		if (buf[0] == 0) continue;
		if (amount_to_read < 0) {
			char name[100];
			char size[12]; 
			strncpy(name, buf, 100);
			strncpy(size, buf + 124, 12);
			printf("%s\n", name);
			amount_to_read = atoi(size); // TODO convert to octal
		} else {
			already_read += 512;
			if (already_read >= amount_to_read) {
				amount_to_read = -1;
				already_read = 0;
			}
		}
	}
	return 0;
}