/* ============================================================================
 * Before Main Existed
 * ============================================================================
 * Another look at absolute entry points. The linker defines where execution really starts in the ELF binary format.
 * ============================================================================
 */
#include <stdio.h>

int
main(int argc, char **argv) {
	printf("main is at 0x%lX\n", (unsigned long)&main);
}
