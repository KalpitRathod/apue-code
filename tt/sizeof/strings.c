#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA "1234567890"

int main() {
	char buf1[] = "abc";
	char buf2[BUFSIZ];
	char buf3[BUFSIZ] = { 'a', 'b', 'c' };
	char *s1 = "abc";
	char *s2 = buf3;

	char *s3;
	if ((s3 = malloc(BUFSIZ)) == NULL) {
		fprintf(stderr, "Unable to allocate memory: %s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	(void)strncat(s3, "0123456789", BUFSIZ);

	return EXIT_SUCCESS;
}
