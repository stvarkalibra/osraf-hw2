#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"


extern int key;
char encr_buf[512];

int encrypt_file(int fd_read, int fd_write) {
	int k = getkey();
	int n;

	while((n = read(fd_read, encr_buf, sizeof(encr_buf))) > 0) {
		for (int i = 0; i < n; i++) {
			encr_buf[i] += k;
		}

		if (fd_write < 0) {
			close(fd_read);
			close(fd_write);
			exit();
		}

		if (write(fd_write, encr_buf, n) != n) {
			printf("cat: write error\n");

			return -1;
		}
	}

	close(fd_read);
	close(fd_write);
	return 0;
}

void
encrAll(){

	char encr_buf1[512],*p;
	int fd;
	struct dirent de;
	struct stat st;

	if((fd = open(".", 0)) < 0){
		fprintf(2, "encr: cannot open %s\n", ".");
		return;
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "encr: cannot stat %s\n", ".");
		close(fd);
		return;
	}

	if(strlen(".") + 1 + DIRSIZ + 1 > sizeof encr_buf1){
		printf("encr: path too long\n");
		close(fd);
		exit();
	}

	strcpy(encr_buf1, ".");
	p = encr_buf1+strlen(encr_buf1);
	*p++ = '/';

	while(read(fd, &de, sizeof(de)) == sizeof(de)){
		if(de.inum == 0)
			continue;
		memmove(p, de.name, DIRSIZ);
		p[DIRSIZ] = 0;
		if(stat(encr_buf1, &st) < 0){
			printf("encr: cannot stat %s\n", encr_buf1);
			continue;
		}
		if(st.type == 2)  {
			int fd1 = open(encr_buf1, O_RDWR);
			if(encr(fd1) == 0){
				int fd_read = open(encr_buf1, O_RDONLY);
				int fd_write = open(encr_buf1, O_WRONLY);
				if(encrypt_file(fd_read, fd_write) == 0){
					printf("Successfully encrypted: %s\n", de.name);
				} else{
					printf("Failed to encrypt: %s\n", de.name);
				}
				close(fd_read);
				close(fd_write);
				close(fd1);
			} else{
				printf("Failed to encrypt: %s\n", de.name);
			}
		}
		else{
			printf("Failed to encrypt: %s\n", de.name);
		}
	}

}

int main(int argc, char *argv[]) {

	if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
		printf("Use this program to encrypt files written on the disk.\n");
		printf("Usage: encr [OPTION]... [FILE]...\n\n");

		printf("Command line options:\n");
		printf("        -h, --help: Show help prompt.\n");
		printf("        -a, --encrypt-all: Encrypt all files in CWD with current key.\n");
		exit();
	}

	struct stat st;
	int fd;

	if (argc > 1 && (strcmp(argv[1], "-a") == 0 || strcmp(argv[1], "--encrypt-all") == 0)) {
		encrAll();
	} else if (argc > 1) {
		fd = open(argv[1], O_RDWR);
		int fd_read = open(argv[1], O_RDONLY);
		int fd_write = open(argv[1], O_WRONLY);
		fstat(fd_read, &st);

		switch(encr(fd)){
			case(0):
				encrypt_file(fd_read, fd_write);
				printf("Successfully encrypted: %s\n", argv[1]);
				break;
			case(-1):
				printf("Key is not set\n");
				break;
			case(-2):
				printf("File is T_DEV type\n");
				break;
			case(-3):
				printf("File is already encrypted\n");
				break;
			default:
				printf("Failed to encrypt: %s\n", argv[1]);
				break;
		}

		close(fd);
		close(fd_read);
		close(fd_write);
	}

	exit();
}
