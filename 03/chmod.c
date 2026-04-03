/* ============================================================================
 * Flipping the Permission Bits
 * ============================================================================
 * UNIX permissions aren't complex rules; they are exactly 9 bits in a 16-bit integer (Read/Write/Execute for Owner/Group/Other). This program uses raw bitwise OR and AND operations to manipulate these bits directly inside the filesystem's inode structure.
 * ============================================================================
 */
/* This simple program illustrates the use of the chmod(2) system call,
 * and how it can be used to set explicit/absolute modes or to selectively
 * enable individual modes.
 *
 * Set your umask to 077, create file and file1, then run this command.
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>

int
main() {
	struct stat sbuf;

	if (stat("file", &sbuf) == -1) {
		perror("can't stat file");
		exit(EXIT_FAILURE);
	}

	/* turn off owner read permissions and turn on setgid */
	if (chmod("file", (sbuf.st_mode & ~S_IRUSR) | S_ISGID) == -1) {
		perror("can't chmod file");
		exit(EXIT_FAILURE);
	}

	/* set absolute mode to rw-r--r-- */
	if (chmod("file1", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) == -1) {
		perror("can't chmod file1");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates two styles of chmod(2): selective bit manipulation (turn a
 *   bit off, add another) and absolute mode assignment. Shows how to compose
 *   permission bitmasks with bitwise operators. Create "file" and "file1"
 *   first, then run.  Set umask to 077 before creating files for clarity.
 *
 * MACROS:
 *   S_IRUSR      - Bit 0400: owner read permission.
 *   S_ISGID      - Bit 02000: set-group-ID bit. On a directory, new files
 *                  inherit the directory's group. On executables, process
 *                  runs with file's group ID.
 *   S_IWUSR      - Bit 0200: owner write permission.
 *   S_IRGRP      - Bit 040: group read permission.
 *   S_IROTH      - Bit 04: other (world) read permission.
 *   EXIT_SUCCESS - 0; returned on success.
 *   EXIT_FAILURE - 1; returned if stat() or chmod() fails.
 *
 * VARIABLES:
 *   struct stat sbuf - Holds current file metadata. sbuf.st_mode contains
 *                      the 16-bit mode word (file type + permission bits).
 *
 * FUNCTIONS:
 *   main()              - Entry point; demonstrates two chmod styles.
 *   stat(path, &sbuf)   - Reads file metadata into sbuf. Needed so we can
 *                         do selective bit manipulation on the existing mode.
 *   chmod(path, mode)   - Changes file permission bits. Requires caller to
 *                         be the file owner or root.
 *   perror(msg)         - Prints "msg: strerror(errno)" to stderr.
 *   exit(status)        - Terminates.
 *
 * ALGORITHM:
 *   1. stat("file") to get current st_mode into sbuf.
 *   2. chmod("file", (sbuf.st_mode & ~S_IRUSR) | S_ISGID):
 *      - (~S_IRUSR): all bits EXCEPT owner-read are set (bitwise NOT).
 *      - (st_mode & ~S_IRUSR): clears owner-read bit, keeps all others.
 *      - | S_ISGID: sets the setgid bit.
 *      Result: owner-read OFF, setgid ON, everything else unchanged.
 *   3. chmod("file1", S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH):
 *      Absolute mode = rw-r--r-- (0644). Ignores existing bits entirely.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   stat(2)    - Reads inode metadata; needed for selective bit manipulation.
 *   chmod(2)   - Sets permission bits; must own the file or be root.
 *
 * ============================================================================
 */
