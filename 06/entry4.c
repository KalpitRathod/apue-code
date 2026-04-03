/* ============================================================================
 * Exiting the Entry
 * ============================================================================
 * The C runtime wrapper that catches a returned value from main() and passes it to _exit() syscall.
 * ============================================================================
 */
#include <stdio.h>

int
main() {
	int n;
	n = printf("main is at 0x%lX\n", (unsigned long)&main);
	return n;
}
