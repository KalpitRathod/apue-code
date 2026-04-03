/* ============================================================================
 * The Executioner
 * ============================================================================
 * Exec replaces the current process image completely. It is not a function call; there is no return. It's software reincarnation.
 * ============================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
func2() {
	_exit(0xcafe);
}

int
func() {
	exit(func2());
}

int
main() {
	printf("%d\n", func());
}
