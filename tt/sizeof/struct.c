#include <stdlib.h>

struct struct1 {
	char *s;
};

struct struct2 {
	char *s;
	int i;
};

struct struct3 {
	char *s;
	int i;
	int j;
};

struct struct4 {
	struct struct1 s1;
	struct struct2 s2;
	struct struct3 s3;
};

int main() {
	struct struct1 s1;
	struct struct2 s2;
	struct struct3 s3;
	struct struct4 s4;
	exit(EXIT_SUCCESS);
}
