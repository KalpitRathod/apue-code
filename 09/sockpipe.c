/* ============================================================================
 * Simulating Pipes with Sockets
 * ============================================================================
 * Why use a UNIX domain socket instead of a pipe? Because sockets allow you to decouple the creation of the IPC channel from the fork() family tree. Unrelated processes can just find a name in the filesystem and start talking, creating a much more flexible architecture.
 * ============================================================================
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NAME "socket"
#define DATA "Just some data"

int main() {
	int child;
	int sock;
	struct sockaddr_un name;
	char buf[BUFSIZ];

	if ((sock = socket(PF_LOCAL, SOCK_DGRAM, 0)) < 0) {
		perror("opening datagram socket");
		exit(EXIT_FAILURE);
	}

	name.sun_family = PF_LOCAL;
	(void)strncpy(name.sun_path, NAME, sizeof(name.sun_path));

	if ((child = fork()) == -1) {
		perror("fork");
	} else if (child) {
		if (bind(sock, (struct sockaddr *)&name, sizeof(struct sockaddr_un))) {
			perror("binding name to datagram socket");
			exit(EXIT_FAILURE);
		}
		if (read(sock, buf, BUFSIZ) < 0) {
			perror("reading from socket");
		}
		(void)printf("Parent (%d) --> reading: %s\n", getpid(), buf);
		(void)close(sock);
	} else {
		printf("Child  (%d) --> sending...\n", getpid());
		if (sendto(sock, DATA, sizeof(DATA), 0, (struct sockaddr *)&name, sizeof(struct sockaddr_un)) < 0) {
			perror("sending datagram message");
		}
		(void)close(sock);
	}

	/* A UNIX domain datagram socket is a 'file'.  If you don't unlink
	 * it, it will remain in the file system. */
	(void)unlink(NAME);
	return EXIT_SUCCESS;
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates UNIX domain datagram sockets for local IPC between a parent
 *   and child process. Unlike anonymous pipes (require fork before creation),
 *   UNIX domain sockets can be used by unrelated processes via a named path.
 *   This example shows the parent binding to a named socket and receiving from
 *   the child. The socket file is a filesystem object that must be unlinked.
 *
 * MACROS:
 *   NAME           - "socket"; the filesystem path for the UNIX domain socket.
 *                    It appears as a socket file (type 's') in ls -l.
 *   DATA           - "Just some data"; the message sent by the child.
 *   PF_LOCAL       - Protocol family for UNIX domain sockets (same as AF_UNIX).
 *   SOCK_DGRAM     - Datagram socket: connectionless, message-based, unreliable.
 *   EXIT_FAILURE   - 1; returned on socket or fork errors.
 *   EXIT_SUCCESS   - 0; returned on success.
 *   BUFSIZ         - From <stdio.h>; receive buffer size.
 *
 * VARIABLES:
 *   int child      - PID returned by fork() (>0=parent, 0=child, -1=error).
 *   int sock       - Socket file descriptor; shared between parent and child
 *                    via fork(). Both use the same fd initially.
 *   struct sockaddr_un name - Socket address structure for UNIX domain:
 *                    .sun_family = PF_LOCAL (must be set)
 *                    .sun_path   = filesystem path for the socket.
 *   char buf[]     - Receive buffer for parent's read().
 *
 * FUNCTIONS:
 *   main()         - Creates socket, forks; parent binds+receives, child sends.
 *   socket(domain, type, protocol) - Creates a socket endpoint. Returns fd.
 *                    PF_LOCAL+SOCK_DGRAM → unnamed UNIX domain datagram socket.
 *   fork()         - Duplicates process; both inherit the socket fd.
 *   bind(sock, &name, len) - Parent: assigns the socket PATH "socket" to fd.
 *                    Parent must bind to receive; child sends to this address.
 *   read(sock, buf, n) - Parent: receives next datagram from any sender.
 *   sendto(sock, data, len, flags, &dest, destlen) - Child: sends datagram to
 *                    the named address (parent's bound socket).
 *   close(sock)    - Close socket fd in each process after use.
 *   unlink(path)   - Remove the socket filesystem entry after use. If not
 *                    unlinked, next bind() on the same path will fail with EADDRINUSE.
 *   perror(msg)    - Print errno description.
 *   printf()       - Print child/parent confirmation messages.
 *   strncpy()      - Copy socket path into sun_path.
 *
 * ALGORITHM:
 *   1. socket(PF_LOCAL, SOCK_DGRAM, 0) → create socket fd.
 *   2. Set name.sun_family=PF_LOCAL, name.sun_path="socket".
 *   3. fork():
 *      PARENT (child>0):
 *        a. bind(sock, &name, ...) → bind socket to "socket" path.
 *        b. read(sock, buf, BUFSIZ) → block until child sends.
 *        c. printf the received data.
 *        d. close(sock).
 *      CHILD (child==0):
 *        a. printf "sending..."
 *        b. sendto(sock, DATA, ..., &name, ...) → send to parent's socket.
 *        c. close(sock).
 *   4. unlink("socket") → clean up the socket file from the filesystem.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   socket(2)   - Creates an unnamed socket; returns fd.
 *   bind(2)     - Assigns a name (path) to a UNIX domain socket.
 *   sendto(2)   - Sends datagram to a specific address (connectionless).
 *   read(2)     - Receives datagram from any sender on a bound socket.
 *   unlink(2)   - Removes the socket file from the filesystem.
 *
 * ============================================================================
 */
