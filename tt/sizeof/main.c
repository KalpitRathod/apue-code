#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv) {
	printf("sizeof(argc): %ld\n", sizeof(argc));
	printf("sizeof(int): %ld\n", sizeof(int));

	printf("sizeof(argv): %ld\n", sizeof(argv));
}
