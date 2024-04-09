#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int create_tarball(char* tarball, char** files);
int extract_files(int fileno);
int list_tar(int fileno);

int main(int argc, char** argv) {
	printf("CC TAR\n");
	if (strcmp(argv[1], "-t") == 0) {
		list_tar(STDIN_FILENO);
	} else if (strcmp(argv[1], "-tf") == 0) {
		int fileno = open(argv[2], 0);
		list_tar(fileno);
	} else if (strcmp(argv[1], "-xf") == 0) {
		int fileno = open(argv[2], 0);
		extract_files(fileno);
	} else if (strcmp(argv[1], "-cf") == 0) {
		create_tarball(argv[2], argv[3]);
	}
	return 0;
}

int create_tarball(char* tarball, char** files) {
	int tar_fd = open(tarball, O_WRONLY | O_CREAT);
	int i = 0;
	char header[512];
	char buf[512];
	while (files[i]) {
		strncpy(header, tarball, strlen(tarball));
		strncpy(header, "00000020", 8);
		write(tar_fd, header, 512);
		int file_fd = open(files[i], 0);
		while (read(file_fd, buf, 512)) {
			write(tar_fd, buf, 512);
		}
	}
	return 0;
}

int extract_files(int fileno) {
	char buf[512];
	int amount_to_read = -1, already_read = 0;
	int new_fd;
	while (read(fileno, buf, 512)) {
		if (buf[0] == 0) continue;
		if (amount_to_read < 0) {
			char name[100];
			char size[12]; 
			strncpy(name, buf, 100);
			strncpy(size, buf + 124, 12);
			amount_to_read = strtol(size, NULL, 8);
			new_fd = open(name, O_WRONLY | O_CREAT);
		} else {
			int remaining = amount_to_read - already_read;
			already_read += 512;
			if (already_read >= amount_to_read) {
				amount_to_read = -1;
				already_read = 0;
			}
			write(new_fd, buf, remaining > 512 ? 512 : remaining);
		}
	}
	return 0;
}

int list_tar(int fileno) {
	char buf[512];
	int amount_to_read = -1, already_read = 0;
	while (read(fileno, buf, 512)) {
		if (buf[0] == 0) continue;
		if (amount_to_read < 0) {
			char name[100];
			char size[12]; 
			strncpy(name, buf, 100);
			strncpy(size, buf + 124, 12);
			printf("%s\n", name);
			amount_to_read = strtol(size, NULL, 8);
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