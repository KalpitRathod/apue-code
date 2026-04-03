/* ============================================================================
 * The Sender's Delivery
 * ============================================================================
 * This program tosses a tagged message into the vast, persistent mailbox of the System V queue. An interesting historical quirk of SysV IPC is that these queues persist even if the creator dies. They live in the kernel indefinitely until explicitly deleted by a user or reboot.
 * ============================================================================
 */
/* A simple program to illustrate the use of Message
 * Queues.
 *
 * Note that message queues continue to exist after
 * all processes have terminated; messages continue to
 * remain in the queues as well.  This is desired, but
 * requires that processes clean up after themselves
 * when they are done using the queues.
 *
 * Note that in this example we manually specify the
 * key instead of using ftok(2).
 *
 * Use this tool to create/send messages, then run
 * msgrecv to retrieve them (in order).  Note that
 * msgrecv will block if no messages are in the queue.
 *
 * Use ipcs(1) to inspect the usage.
 *
 * Derived from:
 * https://users.cs.cf.ac.uk/Dave.Marshall/C/node25.html
 */

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MSGSZ     128

typedef struct msgbuf {
	long mtype;
	char mtext[MSGSZ];
} message_buf;

int
main(int argc, char **argv)
{
	int msqid;
	key_t key;
	message_buf sbuf;
	size_t len;

	if (argc != 3) {
		(void)fprintf(stderr, "Usage: msgsend key message\n");
		exit(EXIT_FAILURE);
	}

	if ((key = atoi(argv[1])) < 1) {
		(void)fprintf(stderr, "Invalid key: %s\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	if ((msqid = msgget(key, IPC_CREAT | 0644)) < 0) {
		perror("msgget");
		exit(EXIT_FAILURE);
	}

	sbuf.mtype = 1;

	(void)strncpy(sbuf.mtext, argv[2], MSGSZ);

	len = strlen(sbuf.mtext) + 1;

	if (msgsnd(msqid, &sbuf, len, IPC_NOWAIT) < 0) {
		(void)fprintf (stderr, "%d, %ld, %s, %d\n",
				msqid, sbuf.mtype, sbuf.mtext, (int)len);
		perror("msgsnd");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Sends a message to a System V message queue. Demonstrates the sender side
 *   of the SysV MQ API. Message queues persist after process exit; messages
 *   remain queued until explicitly received. Use with msgrecv to see the full
 *   lifecycle. Multiple sends accumulate in the queue (unlike shared memory).
 *   Inspect with: ipcs -q; remove with: ipcrm -q <msqid>.
 *
 * MACROS:
 *   MSGSZ          - 128 bytes; max message text size.
 *   IPC_CREAT      - msgget() flag: create queue if it doesn't exist.
 *   IPC_NOWAIT     - msgsnd() flag: return immediately (EAGAIN) if queue full.
 *                    Omit for blocking behavior (process sleeps until space).
 *   EXIT_FAILURE   - 1; returned on bad arguments or IPC errors.
 *   EXIT_SUCCESS   - 0; returned on successful send.
 *
 * VARIABLES:
 *   int msqid          - Message queue identifier returned by msgget().
 *   key_t key          - IPC key; derived from argv[1] (user-chosen integer).
 *   message_buf sbuf   - Struct holding mtype + mtext:
 *     long mtype       - Message type (>0); used by receiver to filter.
 *                        Set to 1 here (any positive long works).
 *     char mtext[]     - Payload text (up to MSGSZ bytes).
 *   size_t len         - Actual message length including null terminator.
 *
 * TYPES:
 *   typedef struct msgbuf { long mtype; char mtext[MSGSZ]; } message_buf;
 *   - mtype must be > 0; mtext is the payload; total size passed to msgsnd.
 *
 * FUNCTIONS:
 *   main(int, char**) - Validates args, opens queue, sends message.
 *   atoi(str)         - Converts argv[1] (key string) to int. Simple; no
 *                       error checking. A value ≤0 is rejected.
 *   msgget(key, flags) - Opens or creates a message queue identified by key.
 *                        Returns msqid (like an fd for the queue).
 *   msgsnd(msqid, &sbuf, sz, flags) - Appends message to queue. sz = length
 *                        of mtext (not the whole struct). Returns 0 on success.
 *   strncpy(dst,src,n) - Copies message text into sbuf.mtext (bounded).
 *   strlen(s)          - To compute message payload length.
 *   fprintf(stderr,...) - Error reporting.
 *   perror(msg)        - Prints errno description.
 *   exit(status)       - Terminates.
 *
 * ALGORITHM:
 *   1. Validate argc == 3 (program key message).
 *   2. key = atoi(argv[1]); reject if ≤ 0.
 *   3. msgget(key, IPC_CREAT | 0644) → open/create the queue.
 *   4. sbuf.mtype = 1; strncpy(sbuf.mtext, argv[2], MSGSZ).
 *   5. len = strlen(sbuf.mtext) + 1 (include null terminator).
 *   6. msgsnd(msqid, &sbuf, len, IPC_NOWAIT) → add to queue.
 *   7. exit(EXIT_SUCCESS).
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   msgget(2)   - Opens/creates queue; returns msqid.
 *   msgsnd(2)   - Enqueues message; blocks (or returns EAGAIN) when full.
 *   msgrcv(2)   - Dequeues message (in msgrecv.c, not shown here).
 *   ipcs(1)     - Shell tool to inspect IPC resources.
 *   ipcrm(1)    - Shell tool to remove IPC resources.
 *
 * ============================================================================
 */
