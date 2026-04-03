/* ============================================================================
 * The Resource Limits Struggle
 * ============================================================================
 * Early UNIX hardcoded the maximum number of open files (often 15 or 20) to save kernel memory. As programs like web servers grew, they needed thousands of descriptors. This file demonstrates the chaotic history of discovering just how many files a system will actually let you open simultaneously.
 * ============================================================================
 */
/*
 * This trivial program attempts to determine how many
 * open files a process can have.  It illustrates the
 * use of pre-processor directives and sysconf(3) to
 * identify a resource that can be changed at system
 * run time.
 *
 * This program also asks getconf(1) and explicitly
 * asks getrlimit(2).  Look at the source code for
 * getconf(1) and sysconf(3) to show how they are
 * implemented, as well as manual page and
 * implementation of getdtablesize(3).
 *
 * Run this once, then use 'ulimit -n' to change the
 * limit and run again.
 *
 * If you (as root) set 'ulimit -n unlimited', what
 * value will you get?  Why?
 *
 * What happens if you set this limit to, say, 0?
 * Why?
 */

#include <sys/resource.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int
countOpenFiles(int num) {
	struct stat stats;
	int count = 0;
	for (int i = 0; i < num; i++) {
		if (fstat(i, &stats) == 0) {
			printf("Currently open: fd #%d (inode %lld)\n", i,
					stats.st_ino);
			count++;
		}
	}

	return count;
}

void
openFiles(int num) {
	int count, fd;

	count = countOpenFiles(num);

	printf("Currently open files: %d\n", count);

	for (int i = count; i <= num ; i++) {
		if ((fd = open("/dev/null", O_RDONLY)) < 0) {
			if (errno == EMFILE) {
				printf("Opened %d additional files, then failed: %s (%d)\n", i - count, strerror(errno), errno);
				break;
			} else {
				fprintf(stderr, "Unable to open '/dev/null' on fd#%d: %s (errno %d)\n",
						i, strerror(errno), errno);
				break;
			}
		}
	}
}

int
main() {
	int openmax;
	struct rlimit rlp;

#ifdef OPEN_MAX
	printf("OPEN_MAX is defined as %d.\n", OPEN_MAX);
#else
	printf("OPEN_MAX is not defined on this platform.\n");
#endif


	printf("'getconf OPEN_MAX' says: ");
	(void)fflush(stdout);
	(void)system("getconf OPEN_MAX");

	/* see also: getdtablesize(3), notably the HISTORY section */
	errno = 0;
	if ((openmax = sysconf(_SC_OPEN_MAX)) < 0) {
		if (errno == 0) {
			fprintf(stderr, "sysconf(3) considers _SC_OPEN_MAX unsupported?\n");
		} else {
			fprintf(stderr, "sysconf(3) error for _SC_OPEN_MAX: %s\n",
					strerror(errno));
		}
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}
	printf("sysconf(3) says this process can open %d files.\n", openmax);

	if (getrlimit(RLIMIT_NOFILE, &rlp) != 0) {
		fprintf(stderr, "Unable to get per process rlimit: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	openmax = (int)rlp.rlim_cur;
	printf("getrlimit(2) says this process can open %d files.\n", openmax);

	printf("Which one is it?\n\n");

	openFiles(openmax);

	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Determines the maximum number of file descriptors a process can have
 *   open simultaneously, using three independent methods and comparing them.
 *   Illustrates compile-time vs runtime limit discovery, and resource limits.
 *   Exercise: run, then 'ulimit -n 256' and run again to see changes.
 *
 * MACROS:
 *   OPEN_MAX         - Compile-time constant from <limits.h>; may not be
 *                      defined on all platforms (checked with #ifdef).
 *   _SC_OPEN_MAX     - sysconf(3) query key for runtime max open files.
 *   RLIMIT_NOFILE    - getrlimit(2) resource ID for number of open files.
 *   EMFILE           - errno value: "too many open files" (process limit).
 *   O_RDONLY         - Open flag: read-only, used in openFiles() to open fds.
 *   EXIT_FAILURE     - 1; returned on sysconf or getrlimit failure.
 *   EXIT_SUCCESS     - 0; returned on success.
 *
 * VARIABLES:
 *   int openmax           - Holds the reported maximum open file count.
 *   struct rlimit rlp     - Receives the current/max resource limits.
 *                           rlim_cur = soft limit, rlim_max = hard limit.
 *   struct stat stats     - Used by fstat() in countOpenFiles() to check
 *                           whether a given fd is open (valid inode).
 *   int count             - Count of currently open fds found by fstat loop.
 *   int fd                - Fd returned by open() in openFiles().
 *   int i                 - Loop counter.
 *
 * FUNCTIONS:
 *   countOpenFiles(num) - Iterates fds 0..num-1 calling fstat() on each.
 *                         A successful fstat means the fd is open.
 *   openFiles(num)      - Opens /dev/null repeatedly until EMFILE (limit hit).
 *                         Demonstrates actual max in practice.
 *   main()              - Queries OPEN_MAX via 3 methods; calls openFiles().
 *   sysconf(_SC_OPEN_MAX) - Runtime limit; may differ from compile-time value.
 *   getrlimit(RLIMIT_NOFILE, &rlp) - Per-process limit; adjustable with ulimit.
 *   system("getconf OPEN_MAX") - Spawns child shell to run getconf(1).
 *   fstat(fd, &stats)   - Gets metadata of an open fd; fails if fd is closed.
 *   open(path, flags)   - Opens a file; used to exhaust the fd table.
 *   fflush(stdout)      - Flushes stdout before calling system() to avoid
 *                         double-buffered output interleaving.
 *
 * ALGORITHM:
 *   1. Print OPEN_MAX if defined; skip if not (platform-dependent).
 *   2. Shell out to 'getconf OPEN_MAX' for another perspective.
 *   3. Call sysconf(_SC_OPEN_MAX) — runtime kernel value.
 *   4. Call getrlimit(RLIMIT_NOFILE) — per-process soft/hard limits.
 *   5. Call openFiles(openmax) to empirically exhaust the fd table,
 *      confirming which limit is actually enforced.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   sysconf(3)      - Queries runtime system limits; returns long.
 *   getrlimit(2)    - Gets/sets per-process resource limits.
 *   fstat(2)        - Stats an open file descriptor (not a path).
 *   open(2)         - Opens a file; used to fill the fd table.
 *   system(3)       - C library; fork+exec a shell command; waits for it.
 *
 * ============================================================================
 */
