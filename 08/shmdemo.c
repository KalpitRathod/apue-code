/* ============================================================================
 * The Fastest IPC: Shared Memory
 * ============================================================================
 * When databases grew enormous, copying rows back and forth between processes became too slow. Shared Memory maps the same RAM pages into both programs. It's the ultimate 'zero-copy' speed hack, making it the bedrock of high-performance localized systems like PostgreSQL.
 * ============================================================================
 */
/* A simple example to illustrate the use of shared
 * memory for IPC.
 *
 * Note that multiple invocations as a writer
 * overwrite the same location (unlike message
 * queues), and that shared memory, as all SysV IPC
 * structures, remain available after the program
 * exits and need to be cleaned up explicitly.
 *
 * Use ipcs(1) to show the use of the shared memory.
 */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Code examples often use a char as the ftok(3) id,
  * but really any int will do. */
#define SHM_ID     42
#define SHM_SIZE 1024

int
main(int argc, char **argv) {
	key_t key;
	int shmid;
	char *data;

	if (argc > 2) {
		(void)fprintf(stderr, "usage: shmdemo [data_to_write]\n");
		exit(EXIT_FAILURE);
		/* NOTREACHED */
	}

	if ((key = ftok("./shmdemo.c", SHM_ID)) == -1) {
		err(EXIT_FAILURE, "ftok");
		/* NOTREACHED */
	}

	if ((shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT)) == -1) {
		err(EXIT_FAILURE, "shmget");
		/* NOTREACHED */
	}

	data = shmat(shmid, (void *)0, 0);
	if (data == (char *)(-1)) {
		err(EXIT_FAILURE, "shmat");
		/* NOTREACHED */
	}

	if (argc == 2) {
		(void)printf("%d: writing to segment: \"%s\"\n", getpid(), argv[1]);
		(void)strncpy(data, argv[1], SHM_SIZE);
	} else
		(void)printf("%d: segment contains: \"%s\"\n", getpid(), data);

	if (shmdt(data) == -1) {
		err(EXIT_FAILURE, "shmdt");
		/* NOTREACHED */
	}

	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates System V shared memory IPC — the fastest IPC mechanism since
 *   processes read/write shared physical memory pages directly with NO copying.
 *   Two invocations share the same memory region:
 *     ./shmdemo "hello"    → writer: stores "hello" in the segment.
 *     ./shmdemo             → reader: prints the stored string.
 *   Note: shared memory survives process exit; use 'ipcs -m' and 'ipcrm -m'
 *   to inspect and delete. Multiple writers OVERWRITE the same location.
 *
 * MACROS:
 *   SHM_ID       - 42; integer ID passed to ftok() to derive a unique key.
 *                  Any non-zero int works; part of the key derivation.
 *   SHM_SIZE     - 1024 bytes; size of the shared memory segment.
 *   IPC_CREAT    - shmget() flag: create segment if it doesn't exist.
 *   EXIT_FAILURE - 1; returned on any IPC error.
 *   EXIT_SUCCESS - 0; returned on success.
 *
 * VARIABLES:
 *   key_t key    - IPC key derived from a pathname+id by ftok(). Used to
 *                  get the same shared memory segment across processes.
 *   int shmid    - Shared memory segment identifier returned by shmget().
 *                  Analogous to a file descriptor for the segment.
 *   char *data   - Pointer to the shared memory region after shmat() attach.
 *                  Writing through 'data' modifies the shared segment.
 *
 * FUNCTIONS:
 *   main(int, char**) - Attaches to (or creates) shared memory; reads or writes.
 *   ftok(path, id)    - Derives a System V IPC key from a file's inode+dev
 *                       and the id byte. Both processes must use the same args.
 *   shmget(key, size, flags) - Creates or opens a shared memory segment of
 *                       'size' bytes. Returns shmid. If IPC_CREAT set, creates
 *                       if it doesn't exist. Mode bits are 0666 here (any user).
 *   shmat(shmid, addr, flags) - Attaches the segment to the process's address
 *                       space. addr=0 means let kernel choose the address.
 *                       Returns pointer to start of shared region. Returns
 *                       (void*)(-1) on error.
 *   strncpy(dst, src, n) - Copies string into shared memory (bounded by SHM_SIZE).
 *   printf()          - Prints current segment contents (reader) or confirmation.
 *   shmdt(data)       - Detaches the shared memory from this process's address
 *                       space. Does NOT destroy the segment.
 *   err(status, msg)  - Prints error and exits.
 *
 * ALGORITHM:
 *   1. ftok("./shmdemo.c", SHM_ID) → derive key from this source file.
 *      (Both writer and reader call ftok with same args → same key.)
 *   2. shmget(key, SHM_SIZE, 0666|IPC_CREAT) → create/open segment.
 *   3. shmat(shmid, 0, 0) → attach to our address space; get pointer 'data'.
 *   4a. Writer (argc==2): strncpy(data, argv[1], SHM_SIZE) — write to segment.
 *   4b. Reader (argc==1): printf(data) — read from segment.
 *   5. shmdt(data) — detach; segment STILL EXISTS in kernel.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   ftok(3)      - Derives IPC key; based on file inode + project ID.
 *   shmget(2)    - Creates/opens shared memory segment; returns shmid.
 *   shmat(2)     - Maps segment into process address space.
 *   shmdt(2)     - Unmaps segment; does NOT free kernel resources.
 *   shmctl(2)    - Control/destroy segment (not shown here; use IPC_RMID).
 *
 * CLEANUP:
 *   ipcs -m              → list shared memory segments
 *   ipcrm -m <shmid>     → delete a specific segment
 *
 * ============================================================================
 */
