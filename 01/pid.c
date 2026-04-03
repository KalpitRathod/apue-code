/* ============================================================================
 * The Birth Certificate of a Process
 * ============================================================================
 * Early systems had no concept of multitasking. When UNIX introduced concurrent programs, the kernel needed a way to tell them apart to deliver signals and manage resources. Thus, the Process ID (PID) was born. Running this file simply asks the kernel: 'Who am I?'
 * ============================================================================
 */
/*
 * This trivial program simply prints out its own PID and exits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main() {
	printf("Our process id is %d.\n", getpid());
	return(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates getpid(2) — the most basic process-identity syscall.
 *   Every UNIX process has a unique Process ID (PID) assigned by the kernel.
 *   PIDs are used to send signals, query /proc/PID (Linux), and track
 *   parent/child relationships.
 *
 * MACROS:
 *   EXIT_SUCCESS   - Defined in <stdlib.h>; value 0; indicates success.
 *
 * VARIABLES:
 *   (none)        - No local variables; getpid() result is used directly
 *                   inside printf's argument list.
 *
 * FUNCTIONS:
 *   main()        - Program entry point; no arguments needed here.
 *   getpid()      - Syscall (unistd.h) returning the calling process's PID
 *                   as a pid_t (typically a typedef for int). Never fails.
 *   printf()      - Writes formatted string to stdout. %d formats an int.
 *
 * ALGORITHM:
 *   1. Call getpid() to get this process's PID from the kernel.
 *   2. Print it via printf to stdout.
 *   3. Return EXIT_SUCCESS (0) to the shell.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   getpid(2)    - Kernel syscall; always succeeds; returns pid_t.
 *   printf(3)    - C library; buffered write to stdout.
 *
 * ============================================================================
 */
