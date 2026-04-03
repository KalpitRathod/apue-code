#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct flex {
	char c;
	char array[];
};

int main(int argc, char **argv) {
	struct flex f1;
	struct flex f2;

	strncpy(f2.array, "flex", 32);

	struct flex *f3;
	int n = 32;

	if (argc == 2) {
		n = strlen("flex");
		if ((f3 = malloc(sizeof(char) + n)) == NULL) {
			fprintf(stderr, "Unable to allocate memory: %s\n",
				strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	strncpy(f3->array, "flex", n);

	exit(EXIT_SUCCESS);
}
