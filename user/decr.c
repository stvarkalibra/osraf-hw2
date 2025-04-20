#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"


extern int key;
char decr_buf[512];

int decrypt_file(int fd_read, int fd_write){
	int k = getkey();
	int n;

	while((n = read(fd_read, decr_buf, sizeof(decr_buf))) > 0) {
		for (int i = 0; i < n; i++) {
			decr_buf[i] -= k;
		}

		if (fd_write < 0) {
			close(fd_read);
			close(fd_write);
			exit();
		}

		if (write(fd_write, decr_buf, n) != n) {
			printf("cat: write error\n");
			return -1;
		}
	}
	close(fd_read);
	close(fd_write);
	return 0;
}

void
decrAll(){

	char decr_buf1[512],*p;
	int fd;
	struct dirent de;
	struct stat st;

	if((fd = open(".", 0)) < 0){
		fprintf(2, "decr: cannot open %s\n", ".");
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "decr: cannot stat %s\n", ".");
		close(fd);
		return;
	}

	if(strlen(".") + 1 + DIRSIZ + 1 > sizeof decr_buf1){
		printf("decr: path too long\n");
		close(fd);
		exit();
	}

	strcpy(decr_buf1, ".");
	p = decr_buf1+strlen(decr_buf1);
	*p++ = '/';

	while(read(fd, &de, sizeof(de)) == sizeof(de)){
		if(de.inum == 0)
			continue;
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;
		if(stat(decr_buf1, &st) < 0){
			printf("decr: cannot stat %s\n", decr_buf1);
			continue;
		}
		if(st.type == 2)  {
			int fd1 = open(decr_buf1, O_RDWR);
			if(decr(fd1) == 0){
				int fd_read = open(decr_buf1, O_RDONLY);
				int fd_write = open(decr_buf1, O_WRONLY);
				if(decrypt_file(fd_read, fd_write) == 0){
					printf("Successfully decrypted: %s\n", de.name);
				} else{
					printf("Failed to decrypt: %s\n", de.name);
				}
				close(fd_read);
				close(fd_write);
			} else{
				printf("Failed to decrypt: %s\n", de.name);
			}
			close(fd1);
		}
		else{
			printf("Failed to decrypt: %s\n", de.name);
		}
	}
}

int
main(int argc, char *argv[])
{
	if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
		printf("Use this program to decrypt files written on the disk.\n");
		printf("Usage: decr [OPTION]... [FILE]...\n\n");

		printf("Command line options:\n");
		printf("        -h, --help: Show help prompt.\n");
		printf("        -a, --decrypt-all: Decrypt all files in CWD with current key.\n");
		exit();
	}

	int fd;
	if (argc > 1 && (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--decrypt-all") == 0)) {
		decrAll();
	} else if (argc > 1) {
		struct stat st;

		fd = open(argv[1], O_RDWR);
		int fd_read = open(argv[1], O_RDONLY);
		int fd_write = open(argv[1], O_WRONLY);
		fstat(fd_read, &st);
		switch(decr(fd)){
			case(0):
				decrypt_file(fd_read, fd_write);
				printf("Successfully decrypted: %s\n", argv[1]);
				break;
			case(-1):
				printf("Key is not set\n");
				break;
			case(-2):
				printf("File is T_DEV type\n");
				break;
			case(-3):
				printf("File is already decrypted or it was never encrypted.\n");
				break;
			default:
				printf("Failed to encrypt: %s\n", argv[1]);
				break;
		}
		close(fd_read);
		close(fd_write);
	}
	close(fd);
	exit();
}
