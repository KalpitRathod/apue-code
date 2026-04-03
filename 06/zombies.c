/* ============================================================================
 * The Undead Process
 * ============================================================================
 * In UNIX, a process cannot entirely die until its parent reads its final exit status (via wait). If the parent forgets to check, a structural corpse—a Zombie—lingers indefinitely in the kernel's process table, holding a PID hostage. The only cure is to kill the parent, letting init(1) sweep them up.
 * ============================================================================
 */
/* This program illustrates how zombies are created, and how they
 * disappear again.  If you like, you can try killing some of the zombies.
 */

#include <sys/wait.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main() {
	(void)printf("Let's create some zombies!\n");

	for (int i=0; i<5; i++) {
		pid_t pid;

		if ((pid = fork()) < 0) {
			err(EXIT_FAILURE, "fork error");
			/* NOTREACHED */
		}

		if (pid == 0) {
			/* Do nothing in the child, i.e. immediately exit.  This
			 * creates a zombie until the parent decides to wait for
			 * the child. */
			exit(EXIT_SUCCESS);
		} else {
			(void)printf("====\n");
			system("ps a | grep '[^ ]'a.ou[t]");
			/* We don't wait for our children.  This allows
			 * them to become zombies.  We sleep for a short
			 * time to delay the next iteration of the loop.
			 * When the parent exits, init will reap the zombies. */
			sleep(1);
		}
	}
	(void)printf("I'm going to sleep - try to kill my zombie children, if you like.\n");
	sleep(30);
	(void)printf("That's enough zombies. Let's have init clean them up.\n");
	(void)printf("Remember to run 'ps a | grep a.ou[t]' to verify.\n");
	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates zombie process creation. A zombie is a child that has exited
 *   but whose exit status has NOT yet been collected by its parent via wait().
 *   The child's kernel entry (PID, exit status) remains until reaped.
 *   Shows the lifecycle: child exits → becomes zombie → parent exits →
 *   init(1) (PID 1) inherits and reaps all orphaned zombies.
 *
 * MACROS:
 *   EXIT_SUCCESS   - 0; child exits cleanly, parent returns normally.
 *   EXIT_FAILURE   - 1; returned if fork() fails.
 *
 * VARIABLES:
 *   int i          - Loop counter; creates 5 child processes.
 *   pid_t pid      - PID returned by fork(): 0 in child, child's PID in parent.
 *
 * FUNCTIONS:
 *   main()         - Entry point; fork loop; parent sleeps while zombies exist.
 *   fork()         - Creates a copy of the process. Child exits immediately;
 *                    parent does NOT wait, allowing zombie to form.
 *   exit(EXIT_SUCCESS) - Child exits; kernel keeps exit status until reaped.
 *   system(cmd)    - Parent runs 'ps a | grep...' to show zombie states (Z).
 *   sleep(n)       - Parent delays between loop iterations; also at end so
 *                    the user can manually try 'kill' on zombies (futile).
 *   err(status, msg) - Prints error and exits; from <err.h>.
 *   printf()       - Informational messages.
 *
 * ALGORITHM:
 *   1. Loop 5 times:
 *      a. fork(): child immediately exits → becomes zombie (state 'Z' in ps).
 *      b. Parent does NOT call wait() → zombie persists.
 *      c. Parent calls system("ps a | grep 'a.ou[t]'") to show zombie in ps.
 *      d. Parent sleeps 1 second before next iteration.
 *   2. Parent prints message then sleeps 30 seconds.
 *      During this time user can observe zombies with 'ps a'.
 *      Trying 'kill -9 <zombie_pid>' has no effect (zombie is already dead).
 *   3. Parent's main() returns. Parent exits → init(1) adopts orphaned
 *      zombies and calls wait() for each, clearing them from the ps table.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   fork(2)    - Duplicates process; both continue from the fork() point.
 *   exit(3)    - Terminates process; kernel keeps minimal info until wait().
 *   wait(2)    - Reaps a zombie. The parent NEVER calls it here (intentional).
 *   system(3)  - Runs a shell command; internally fork+exec+waitpid.
 *   sleep(3)   - Suspends execution for N seconds.
 *
 * ZOMBIE LIFECYCLE:
 *   CHILD exits → state=Z (zombie: holds PID + exit code only)
 *   PARENT calls wait() → zombie reaped, entry removed from process table
 *   If PARENT exits first → init(1) inherits children and reaps them
 *
 * ============================================================================
 */
