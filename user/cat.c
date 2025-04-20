#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define MAJOR_ENCR 1

char buf[512];

void
cat(int fd)
{
	int major = getmajor(fd);
	int n;
	int key = getkey();
	while((n = read(fd, buf, sizeof(buf))) > 0) {
		if(major != MAJOR_ENCR || key == 0){
			if (write(1, buf, n) != n) {
				printf("cat: write error\n");
				exit();
			}
		} else{
			for(int i = 0; i < n; i++){
				buf[i] -= key;
			}
			if (write(1, buf, n) != n) {
				printf("cat: write error\n");
				exit();
			}
		}
	}
	if(n < 0){
		printf("cat: read error\n");
		exit();
	}
	printf("\n");
}

int
main(int argc, char *argv[])
{
	int fd, i;

	if(argc <= 1){
		cat(0);
		exit();
	}

	for(i = 1; i < argc; i++){
		if((fd = open(argv[i], 0)) < 0){
			printf("cat: cannot open %s\n", argv[i]);
			exit();
		}
		cat(fd);
		close(fd);
	}
	exit();
}
