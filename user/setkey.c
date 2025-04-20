#include "kernel/stat.h"
#include "user.h"
#include "kernel/fs.h"

int setkeysecret(int); // syscall

int
main(int argc, char *argv[])
{
	if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
		printf("Use this program to set the current active key.\n");
		printf("After setting the key, you can use encr and decr with that key.\n");
		printf("Usage: setkey [OPTION]... [KEY]\n\n");

		printf("Command line options:\n");
		printf("        -h, --help: Show help prompt.\n");
		printf("        -s, --secret: Enter the key via STDIN. Hide key when entering it.\n");
		exit();
	}

	char buf[16];
	int key;
	int is_key_secret = 0;

	if (argc > 1 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--secret") == 0)) {
		printf("Enter the key: ");
		is_key_secret = 1;
		setkeysecret(is_key_secret);

		int i = 0;
		char ch;
		while (i < sizeof(buf) - 1) {
			if (read(0, &ch, 1) < 1)
				break;

			if (ch == '\n') {
				buf[i] = 0;
				break;
			}
			buf[i++] = ch;
		}

		key = atoi(buf);
		setkey(key);
	} else if (argc > 1) {
		key = atoi(argv[1]);
		setkey(key);
	}

	exit();
}
