/* ============================================================================
 * Global vs Local Memory
 * ============================================================================
 * Showing how global uninitialized variables land in the BSS segment and get zeroed by the exec loader for free.
 * ============================================================================
 */
/* A simple program to illustrate that we can print
 * the address of any variable. */

#include <stdio.h>
#include <stdlib.h>

int
main() {
	int var;

	(void)printf("var is at: 0x%12lX\n", (unsigned long)&var);

	return EXIT_SUCCESS;
}
