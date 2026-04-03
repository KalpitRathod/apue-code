/* ============================================================================
 * The Salting Defense
 * ============================================================================
 * In the very early days, systems stored plain text passwords. Then they stored encrypted passwords. But if two people used 'password123', their encrypted hashes matched! The concept of a 'Salt'—a random string added before hashing—was invented to guarantee every hash was visually unique.
 * ============================================================================
 */
/* A simple program to illustrate how to generate a
 * password hash.  We show use of different hashing
 * algorithms together with the effect of using a
 * random salt.  Repeated invocations will thus
 * produce a different password hash for the same
 * input, but identical hashes for the static salt.
 */
#include <openssl/rand.h>

#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SALT_LEN 64

const char CHARSET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789";

void
genSalt(char *salt, int len) {
	unsigned char bytes[len];
	if (RAND_bytes(bytes, len) == 0) {
		err(EXIT_FAILURE, "Unable to generate random data: %s\n",
				strerror(errno));
		/* NOTREACHED */
	}

	for (int i=0; i<len; i++) {
		salt[i] = CHARSET[bytes[i] % (sizeof(CHARSET) - 1)];
	}
}

void
printCrypt2(const char *s) {
	unsigned char bytes[MAX_SALT_LEN];
	char salt[MAX_SALT_LEN + 4] = { 0 };
	char *buf = salt;

	if (RAND_bytes(bytes, MAX_SALT_LEN) == 0) {
		err(EXIT_FAILURE, "Unable to generate random data: %s\n",
				strerror(errno));
		/* NOTREACHED */
	}

	(void)snprintf(buf, 4, "$1$");
	buf += 3;
	for (int i=0; i<MAX_SALT_LEN; i++) {
		buf[i] = CHARSET[bytes[i] % (sizeof(CHARSET) - 1)];
	}

	(void)printf("%s\n", crypt(s, salt));
}

int
main(int argc, char **argv) {
	char salt[MAX_SALT_LEN] = { 0 };
	char *buf = salt;

	if (argc != 2) {
		(void)fprintf(stderr, "Usage: %s string\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	(void)printf("MD5 (static salt): %s\n", crypt(argv[1], "$1$salt"));

	(void)snprintf(salt, 4, "$1$");
	buf += 3;
	genSalt(buf, 8);
	(void)printf("MD5 (random salt): %s\n", crypt(argv[1], salt));

	memset(salt, 0, MAX_SALT_LEN);
	(void)snprintf(salt, 8, "$2a$12$");
	buf = salt + 7;
	genSalt(buf, 22);
	(void)printf("Blowfish         : %s\n", crypt(argv[1], salt));

	memset(salt, 0, MAX_SALT_LEN);
	(void)snprintf(salt, 31, "$argon2id$v=19$m=4096,t=6,p=1$");
	buf = salt + 30;
	genSalt(buf, 32);
	(void)printf("Argon2Id         : %s\n", crypt(argv[1], salt));

	exit(EXIT_SUCCESS);
}
