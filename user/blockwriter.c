#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

#define DEFAULT_BLOCKS 150
#define DEFAULT_NAME "long.txt"
#define BLOCK_SIZE 512

void
print_help()
{
	printf(
		"\nUse this program to create a big file filled with a-z characters\n"
		"Default filename: long.txt\n"
		"Default block: 150\n"
		"Usage: blockwriter [OPTION]...\n\n"
		"Command line options:\n"
		"            -h, --help: Show help prompt.\n"
		"            -b, --blocks: Number of blocks to write.\n"
		"            -o, --output-file: Set output filename.\n"
	);
}

void
write_file(const char *filename, int blocks)
{
	int fd = open(filename, O_CREATE | O_RDWR);
	if (fd < 0) {
		printf("error: could not create file %s\n", filename);
		exit();
	}

	char buf[BLOCK_SIZE];
	int total_bytes = blocks * BLOCK_SIZE;
	int written = 0;
	int c = 'a';

	for (int block_num = 1; block_num <= blocks; block_num++) {
		for (int i = 0; i < BLOCK_SIZE; i++) {
			buf[i] = c;
			c++;
			if (c > 'z')
				c = 'a';
		}

		printf("Writing Block %d\n", block_num);

		if (write(fd, buf, BLOCK_SIZE) != BLOCK_SIZE) {
			printf("error: failed to write block\n");
			close(fd);
			exit();
		}

		written += BLOCK_SIZE;
	}

	close(fd);
	printf("Created file %s with %d blocks (%d bytes)\n", filename, blocks, total_bytes);
}

int
main(int argc, char *argv[])
{
	char *filename = DEFAULT_NAME;
	int blocks = DEFAULT_BLOCKS;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_help();
			exit();
		} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output-file") == 0) {
			if (i + 1 >= argc) {
				printf("error: missing filename after -o\n");
				exit();
			}
			filename = argv[++i];
		} else if (strcmp(argv[i], "-b") == 0 || strcmp(argv[i], "--blocks") == 0) {
			if (i + 1 >= argc) {
				printf("error: missing block number after -b\n");
				exit();
			}
			blocks = atoi(argv[++i]);
			if (blocks <= 0) {
				printf("error: invalid block count\n");
				exit();
			}
		} else {
			printf("error: unknown option '%s'\n", argv[i]);
			exit();
		}
	}

	write_file(filename, blocks);
	exit();
}
