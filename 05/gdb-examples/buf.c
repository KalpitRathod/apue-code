/* ============================================================================
 * The Stack Smashing Playground
 * ============================================================================
 * One of the most famous computer security vulnerabilities in history is the Buffer Overflow (the Morris Worm exploited this in 1988). When you overstep an array in C, you overwrite the stack frame's return address. This file is a lethal playground for stack corruption.
 * ============================================================================
 */
/* This file is used together with the file 'main.c' to
 * help students use gdb(1) to understand
 * manipulations of strings and buffers.
 *
 * See https://youtu.be/hgcj7iAxhhU as well as
 * 'pointer.c'
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DATA2 "Hello, I'm buf2!"
#define DATA3 "Hello, I'm buf3!"

void
printBufs(long n) {
	char *buf = malloc(n);
	char *buf2 = malloc(8);
	char *buf3 = malloc(8);

	strcpy(buf2, DATA2);
	strcpy(buf3, DATA3);
	gets(buf);

	printf("buf is : '%s'\n", buf);
	printf("buf2 is: '%s'\n", buf2);
	printf("buf3 is: '%s'\n", buf3);
}
