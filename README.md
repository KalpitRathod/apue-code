# Advanced Programming in the UNIX Environment — Code Reference

> Sample code from the graduate course **"Advanced Programming in the UNIX Environment"** taught by Jan Schaumann at Stevens Institute of Technology.
> Course website: https://stevens.netmeister.org/631/

This repository is a hands-on exploration of systems programming concepts from Stevens & Rago's *APUE*. Each numbered directory corresponds to a lecture topic. The code is written in C and targets POSIX-compliant systems (Linux, macOS, NetBSD).

---

## Table of Contents

1. [01 — UNIX Fundamentals & Introduction](#01--unix-fundamentals--introduction)
2. [02 — File I/O (Unbuffered)](#02--file-io-unbuffered)
3. [03 — File System: Permissions, Ownership & stat](#03--file-system-permissions-ownership--stat)
4. [04 — Directory & Link Operations](#04--directory--link-operations)
5. [05 — Toolchain: Make & GDB](#05--toolchain-make--gdb)
6. [06 — Process Environment & Memory Layout](#06--process-environment--memory-layout)
7. [07 — Signals](#07--signals)
8. [08 — Interprocess Communication (IPC)](#08--interprocess-communication-ipc)
9. [09 — Network IPC & Sockets](#09--network-ipc--sockets)
10. [11 — Linker, Shared Libraries & Dynamic Loading](#11--linker-shared-libraries--dynamic-loading)
11. [12 — Advanced I/O & Security](#12--advanced-io--security)
12. [13 — Process Scheduling & Priority](#13--process-scheduling--priority)

---

## 01 — UNIX Fundamentals & Introduction

**Directory:** `01/`

This chapter introduces the foundational UNIX programming model: processes, file descriptors, system calls, and the fork-exec pattern.

### Core Concepts

- Every UNIX process starts with three open file descriptors: `stdin` (0), `stdout` (1), `stderr` (2).
- `fork()` creates a copy of the current process (child). The child returns 0; the parent gets the child's PID.
- `exec*()` replaces the current process image with a new program.
- `waitpid()` allows a parent to collect the exit status of a child, preventing zombie processes.
- The kernel identifies every process by a unique **PID** (Process ID) returned by `getpid()`.

### Files

#### `hw.c` — Hello, World

The simplest C program; demonstrates correct use of `exit(EXIT_SUCCESS)` rather than `return 0`, which is preferred for clarity.

```c
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    (void)argc;   // suppress unused-variable warnings
    (void)argv;
    printf("Hello, World!\n");
    exit(EXIT_SUCCESS);
}
```

**Key point:** Casting unused arguments to `(void)` avoids `-Werror=unused-parameter` compile errors while documenting intent.

---

#### `pid.c` — Getting a Process ID

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("Our process id is %d.\n", getpid());
    return(EXIT_SUCCESS);
}
```

`getpid()` is a syscall that returns the PID of the calling process. PIDs are assigned sequentially by the kernel and are used to send signals, wait for children, and track process ownership.

---

#### `simple-cat.c` — Unbuffered I/O Cat Clone

Implements the classic `cat` utility using raw POSIX I/O (`read`/`write`) instead of stdio.

```c
#define BUFFSIZE 32768

int main(int argc, char **argv) {
    int n;
    char buf[BUFFSIZE];

    while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0) {
        if (write(STDOUT_FILENO, buf, n) != n) {
            fprintf(stderr, "Unable to write: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    if (n < 0) {
        fprintf(stderr, "Unable to read: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return(EXIT_SUCCESS);
}
```

**Concepts:**
- `STDIN_FILENO` (0), `STDOUT_FILENO` (1) — integer file descriptors for standard streams.
- `read(fd, buf, count)` returns bytes read, 0 on EOF, -1 on error.
- `write(fd, buf, count)` must check that all `n` bytes were written (partial writes happen on pipes/network).
- Buffer size of 32768 (32 KiB) is chosen to match typical disk block sizes for performance.
- This program is also a primitive `cp`: `./simple-cat < src > dst`.

---

#### `simple-ls.c` — Directory Listing

```c
#include <dirent.h>

int main(int argc, char **argv) {
    DIR *dp;
    struct dirent *dirp;

    if ((dp = opendir(argv[1])) == NULL) { ... }

    while ((dirp = readdir(dp)) != NULL)
        printf("%s\n", dirp->d_name);

    closedir(dp);
    return EXIT_SUCCESS;
}
```

**Concepts:**
- `DIR *` is an opaque handle representing an open directory stream.
- `readdir()` returns each directory entry as a `struct dirent`, whose `d_name` field holds the filename.
- `opendir()` / `closedir()` manage the lifecycle of the directory stream.

---

#### `simple-shell.c` — Minimal Shell

Implements a read-fork-exec-wait loop — the heart of every shell.

```c
while (getinput(buf, sizeof(buf))) {
    buf[strlen(buf) - 1] = '\0';   // strip newline

    if ((pid = fork()) == -1) {
        // fork failed
    } else if (pid == 0) {         // child process
        execlp(buf, buf, (char *)0);
        fprintf(stderr, "couldn't exec %s\n", buf);
        exit(EX_UNAVAILABLE);
    }
    // parent waits for child
    waitpid(pid, &status, 0);
}
```

**Concepts:**
- `fork()` — child PID = 0; parent PID = child's actual PID.
- `execlp()` — searches `PATH` for the executable; returns only on error.
- `waitpid()` — blocks the parent until the child finishes; prevents zombies.
- `EX_UNAVAILABLE` (from `<sysexits.h>`) — standard exit code for "command not found".

---

## 02 — File I/O (Unbuffered)

**Directory:** `02/`

Deep dive into the low-level POSIX I/O interface: `open`, `read`, `write`, `lseek`, `close`, and file descriptor management.

### Core Concepts

- **File descriptors** are small non-negative integers that index into a per-process file descriptor table maintained by the kernel.
- The three standard descriptors (0, 1, 2) are always open when a process starts.
- `lseek()` repositions the file offset. Not all file types are seekable (pipes, sockets are not).
- **Sparse files** have "holes" — regions that consume no disk space but read back as zeros.
- The kernel imposes a limit on open file descriptors per process, queryable via `sysconf(_SC_OPEN_MAX)` or `getrlimit(RLIMIT_NOFILE, ...)`.

### Files

#### `lseek.c` — Testing Seekability

```c
if (lseek(STDIN_FILENO, 0, SEEK_CUR) == -1)
    printf("cannot seek\n");
else
    printf("seek OK\n");
```

- `SEEK_CUR` with offset 0 is a no-op — it just returns the current position.
- Returns -1 with `errno = ESPIPE` for pipes, FIFOs, and sockets.
- Test: `./lseek < file.txt` vs `cat file.txt | ./lseek`.

---

#### `hole.c` — Sparse File Creation

```c
char buf1[] = "abcdefghij";
char buf2[] = "ABCDEFGHIJ";
#define BIGNUM 10240000

int main(void) {
    int fd = creat("file.hole", S_IRUSR | S_IWUSR);
    write(fd, buf1, strlen(buf1));       // write at offset 0
    lseek(fd, BIGNUM, SEEK_CUR);         // jump 10 MB forward
    write(fd, buf2, strlen(buf2));       // write at offset 10MB+10
    close(fd);
}
```

**Concepts:**
- The resulting file is ~10 MB in reported size but consumes almost no disk.
- The "hole" reads back as all-zero bytes — the kernel supplies them without storing them.
- Verify with: `ls -lh file.hole` (size) vs `du -sh file.hole` (actual disk use).
- Copying with `cp` fills the hole with real zeros, expanding disk usage.

---

#### `fds.c` — Understanding File Descriptor Allocation

```c
fd1 = open("/dev/zero", O_RDONLY);   // gets fd 3
fd2 = open("/dev/zero", O_RDONLY);   // gets fd 4
close(fd1);                          // fd 3 is now free
fd3 = open("/dev/zero", O_RDONLY);   // reuses fd 3
```

**Key rules:**
- `open()` always assigns the **lowest available** file descriptor.
- After closing fd 3, the next `open()` reuses it.
- Each open file description is independent — two `open()` calls to the same file give separate offsets.
- `fileno(FILE *)` converts a stdio `FILE *` pointer to its underlying integer fd.

---

#### `openmax.c` — Maximum Open Files

Demonstrates three ways to query the open-file limit:

| Method | API |
|--------|-----|
| Compile-time constant | `OPEN_MAX` (from `<limits.h>`) |
| Runtime system config | `sysconf(_SC_OPEN_MAX)` |
| Per-process resource limit | `getrlimit(RLIMIT_NOFILE, &rlp)` |

```c
// Runtime query
openmax = sysconf(_SC_OPEN_MAX);

// Per-process soft limit
struct rlimit rlp;
getrlimit(RLIMIT_NOFILE, &rlp);
openmax = (int)rlp.rlim_cur;
```

- Soft limits can be raised up to the hard limit without root privileges.
- `ulimit -n` in the shell adjusts the soft limit for the current session.

---

## 03 — File System: Permissions, Ownership & stat

**Directory:** `03/`

The `stat` family of syscalls reveals metadata about files. This chapter covers file types, permission bits, the setuid mechanism, and the umask.

### Core Concepts

- `stat(path, &sb)` follows symlinks; `lstat(path, &sb)` reports on the symlink itself.
- `struct stat` contains: inode number, mode (type + permissions), link count, uid, gid, size, timestamps.
- The **9 permission bits** (rwxrwxrwx) are tested by macros like `S_IRUSR`, `S_IWGRP`, `S_IXOTH`.
- **setuid bit** (`S_ISUID`): when set on an executable, the process runs with the *file owner's* UID, not the invoking user's UID. This is how `passwd(1)` can write `/etc/shadow`.
- The **umask** is a bitmask that *clears* permission bits when files are created.

### Files

#### `simple-ls-stat.c` — File Type Detection

```c
char *getType(const struct stat sb) {
    if (S_ISREG(sb.st_mode))  return "regular file";
    if (S_ISDIR(sb.st_mode))  return "directory";
    if (S_ISCHR(sb.st_mode))  return "character special";
    if (S_ISBLK(sb.st_mode))  return "block special";
    if (S_ISFIFO(sb.st_mode)) return "FIFO";
    if (S_ISLNK(sb.st_mode))  return "symbolic link";
    if (S_ISSOCK(sb.st_mode)) return "socket";
    return "unknown";
}
```

The program calls both `stat()` and `lstat()` per entry. If they disagree, the entry is a symlink. The type macros test the `st_mode` field using predefined bit masks.

---

#### `setuid.c` — Privilege Escalation via setuid

Three UIDs exist per process:

| UID | Meaning |
|-----|---------|
| **Real UID** (ruid) | Who you actually are (the user who ran the program) |
| **Effective UID** (euid) | Who you appear to be (determines privilege checks) |
| **Saved set-UID** (suid) | Saved copy of original euid, for regaining privilege |

```c
// Drop privileges temporarily
myseteuid(UNPRIVILEGED_UID);

// Try to open a privileged file — will FAIL
open(argv[1], O_RDONLY);

// Regain privileges using saved set-uid
myseteuid(suid);

// Try again — will SUCCEED
open(argv[1], O_RDONLY);

// Drop privileges PERMANENTLY (clears saved set-uid when root)
setuid(ruid);
```

**Security rule:** Always drop privileges permanently (`setuid(real_uid)`) as soon as elevated access is no longer needed.

---

#### `access.c` — Real vs Effective UID Check

```c
access(argv[1], R_OK);    // checks using REAL uid
open(argv[1], O_RDONLY);  // checks using EFFECTIVE uid
```

`access()` checks permissions against the **real** UID, ignoring the setuid bit. This is critical for setuid programs to check if the *actual user* (not the elevated euid) has permission. However, there's a TOCTOU (Time-of-Check/Time-of-Use) race condition between `access()` and `open()`.

---

#### `chmod.c` — Changing File Permissions

```c
// Selectively: turn off owner-read, turn on setgid
chmod("file", (sbuf.st_mode & ~S_IRUSR) | S_ISGID);

// Absolute: set rw-r--r-- (0644)
chmod("file1", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
```

Bitmask manipulation with `&`, `|`, `~` is the standard approach. The `setgid` bit on a directory causes new files to inherit the directory's group — useful for shared project directories.

---

#### `umask.c` — How umask Filters Permissions

```c
myOpen("file1");         // inherits shell's umask (e.g., 022 → 0755)
umask(0);                // turn OFF umask: allow all bits
myOpen("file2");         // gets exactly 0777
umask(S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH); // block group/other rw
myOpen("file3");         // gets 0711
```

The umask is a **per-process** value, inherited by children. It ANDs out bits — you cannot *add* permissions through umask, only *remove* them.

---

## 04 — Directory & Link Operations

**Directory:** `04/`

### Core Concepts

- `chdir()` / `getcwd()` change and query the working directory.
- Hard links (`link()`) create additional directory entries pointing to the same inode. The inode is freed only when all links are removed.  
- Symbolic links (`symlink()`) create a new file whose content is a path — the kernel follows the link transparently.
- `rename()` atomically replaces the target if it exists (same filesystem).
- `unlink()` removes a directory entry; the data remains until the last file descriptor referencing it is closed.

### Files

#### `wait-unlink.c` — Unlink vs Close

Demonstrates that a file's data persists after `unlink()` as long as there are open file descriptors:

```c
fd = open("tmpfile", O_CREAT|O_RDWR, 0600);
unlink("tmpfile");    // removes directory entry — ls won't show it
// fd is still valid; we can read/write
write(fd, data, len);
read(fd, buf, len);
close(fd);            // NOW the inode and data are freed
```

This is a common technique for creating truly temporary files that are cleaned up even on crash.

---

#### `cd.c` — Working Directory

```c
chdir(newpath);         // changes process working directory
getcwd(buf, sizeof(buf)); // retrieves current path
```

`chdir()` only affects the current process (and its subsequently forked children). It cannot change the shell's directory — which is why `cd` must be a shell *builtin*.

---

## 05 — Toolchain: Make & GDB

**Directory:** `05/`

### Core Concepts

#### Make

`make` reads a `Makefile` and builds targets only when dependencies have changed (using timestamps).

```makefile
CC      = cc
CFLAGS  = -Wall -Werror -Wextra

target: dependency1.o dependency2.o
    $(CC) $(CFLAGS) -o target dependency1.o dependency2.o

dependency1.o: dependency1.c
    $(CC) $(CFLAGS) -c dependency1.c
```

Key `make` concepts:
- **Phony targets** (`.PHONY: clean`) — don't correspond to real files.
- **Automatic variables**: `$@` = target, `$<` = first dependency, `$^` = all dependencies.
- **Pattern rules**: `%.o: %.c` applies to any `.c` → `.o` compilation.

#### The Compile Chain

```
Source (.c) → Preprocessor → Compiler → Assembler → Linker → Executable
```

1. **Preprocessor** (`cpp`): expands `#include`, `#define`, `#ifdef`
2. **Compiler** (`cc -S`): produces assembly (`.s` file)
3. **Assembler** (`as`): produces object files (`.o` — ELF/Mach-O)
4. **Linker** (`ld`): resolves symbols, links libraries → final executable

#### GDB Essentials

```bash
cc -g -o prog prog.c        # compile with debug symbols
gdb ./prog                  # launch debugger

(gdb) break main            # set breakpoint at main()
(gdb) run arg1 arg2         # start execution
(gdb) next                  # step over (one source line)
(gdb) step                  # step into (follow function calls)
(gdb) print var             # inspect variable value
(gdb) backtrace             # show call stack
(gdb) info registers        # show CPU registers
(gdb) x/10x $sp             # examine 10 hex words at stack pointer
```

---

## 06 — Process Environment & Memory Layout

**Directory:** `06/`

### Core Concepts

Understanding how a process is organized in virtual memory is essential for debugging, security analysis, and performance tuning.

### Process Memory Segments (Low → High Address)

```
╔══════════════════════╗  ← High addresses
║  Command-line args   ║    argv, envp
║  Environment         ║
╠══════════════════════╣
║  Stack               ║  ← grows DOWN
║  (local variables,   ║    each function call = new frame
║   return addresses)  ║
╠══════════════════════╣
║  ↕ (gap)             ║    ASLR randomizes positions
╠══════════════════════╣
║  Heap                ║  ← grows UP (malloc/free)
╠══════════════════════╣
║  BSS Segment         ║    uninitialized globals (zeroed by OS)
╠══════════════════════╣
║  Data Segment        ║    initialized globals and statics
╠══════════════════════╣
║  Text Segment        ║    compiled machine code (read-only)
╚══════════════════════╝  ← Low addresses (0x0 not mapped)
```

### Files

#### `memory-layout.c` — Visualizing Memory

This program prints the addresses of every segment, including heap (via `malloc`), shared memory (via `shmget`/`shmat`), and stack frames.

```c
// BSS: uninitialized globals — zero-initialized by the OS
char array[ARRAY_SIZE];    // in BSS
int num2;                  // in BSS

// Data: initialized globals
char *string = "a string"; // pointer in Data; literal in Text (rodata)
int num = 10;              // in Data

// Stack: local to the function
char func_array[ARRAY_SIZE]; // on stack in main()

// Heap: dynamically allocated
char *ptr = malloc(MALLOC_SIZE);
```

**Interesting observation:** A `static int n = 0;` inside a function is in BSS (uninitialized). Change it to `static int n = 1;` and it moves to the Data segment.

**Stack overflow:** Compile with `-DSTACKOVERFLOW` to trigger infinite mutual recursion between `func()` and `func2()`, demonstrating a stack overflow via SIGSEGV.

---

#### `exit-handlers.c` — `atexit()` and Exit Cleanup

```c
void my_exit1(void) { printf("first exit handler\n"); }
void my_exit2(void) { printf("second exit handler\n"); }

int main() {
    atexit(my_exit2);   // registered first → runs LAST
    atexit(my_exit1);   // registered second → runs SECOND
    atexit(my_exit1);   // registered third → runs FIRST
    // Output on normal exit: "first", "first", "second"
}
```

**Key rules:**
- `atexit()` handlers are called in **LIFO** (last-registered, first-called) order.
- Called by `exit()` — not by `_exit()` or `abort()`.
- `abort()` sends SIGABRT → generates a core dump; no cleanup.
- `_exit()` bypasses all C library cleanup (stdio buffers not flushed).

---

#### `zombies.c` — Zombie Processes

```c
for (int i = 0; i < 5; i++) {
    if ((pid = fork()) < 0) { err(...); }
    if (pid == 0) {
        exit(EXIT_SUCCESS);   // child exits immediately
    } else {
        // parent does NOT wait → child becomes a zombie
        system("ps a | grep 'a.ou[t]'");
        sleep(1);
    }
}
sleep(30);  // zombies visible during this sleep
// When parent exits, init(8) reaps zombies
```

A **zombie** is a child that has exited but whose exit status has not yet been collected by the parent. It holds a minimal kernel structure (PID, exit status). Zombies cannot be killed — only the parent's `wait()` or the parent's death can remove them. When the parent dies, `init(1)` (PID 1) inherits and reaps orphans.

---

#### `forkflush.c` — Buffering and fork()

`fork()` duplicates the parent's memory including stdio buffers. If output is buffered at fork time, it appears twice in the output. Using `fflush(stdout)` before `fork()`, or using line-buffered/unbuffered mode, prevents duplicate output.

---

## 07 — Signals

**Directory:** `07/`

### Core Concepts

Signals are asynchronous notifications delivered to a process. The kernel delivers them in between instructions (at safe points).

| Signal | Default Action | Common Cause |
|--------|----------------|--------------|
| `SIGINT` | Terminate | `Ctrl+C` |
| `SIGQUIT` | Core dump | `Ctrl+\` |
| `SIGKILL` | Terminate (uncatchable) | `kill -9` |
| `SIGSEGV` | Core dump | Invalid memory access |
| `SIGCHLD` | Ignore | Child exited |
| `SIGALRM` | Terminate | `alarm()` timer expired |
| `SIGUSR1/2` | Terminate | User-defined |

### Files

#### `signals1.c` — Signal Handlers and Reentrancy

```c
static void sig_quit(int signo) {
    (void)signo;
    printf("In sig_quit, s=%d. Now sleeping...\n", ++s);
    sleep(SLEEP);
    printf("sig_quit, s=%d: exiting\n", s);
}

static void sig_int(int signo) {
    (void)signo;
    printf("Now in sig_int, s=%d. Returning immediately.\n", ++s);
}

int main(void) {
    signal(SIGQUIT, sig_quit);
    signal(SIGINT, sig_int);
    sleep(SLEEP);
    ...
}
```

**Key behaviors demonstrated:**
- While inside `sig_quit`, the same signal (`SIGQUIT`) is **blocked** — it won't re-enter.
- A *different* signal (`SIGINT` / `^C`) **can** interrupt `sig_quit` mid-sleep.
- Multiple `^\` presses while in `sig_quit` are **merged** → delivered as a single signal after the handler returns.
- **Reentrancy warning:** `printf()` is not async-signal-safe. The only truly safe functions inside signal handlers are those listed in POSIX as async-signal-safe (e.g., `write()`, `_exit()`, `sem_post()`).

---

#### `signals3.c` — Signal Masks / Blocking

```c
sigset_t newmask, oldmask, pendmask;

// Build a mask containing SIGQUIT
sigemptyset(&newmask);
sigaddset(&newmask, SIGQUIT);

// Block SIGQUIT — signals sent now are PENDING
sigprocmask(SIG_BLOCK, &newmask, &oldmask);

sleep(SLEEP);  // ^\ during this won't be delivered

// Check if SIGQUIT is pending
sigpending(&pendmask);
if (sigismember(&pendmask, SIGQUIT))
    printf("Pending SIGQUIT found.\n");

// Unblock — pending signal is delivered NOW
sigprocmask(SIG_SETMASK, &oldmask, NULL);
```

**Concepts:**
- `sigprocmask(SIG_BLOCK, ...)` adds signals to the process signal mask (blocked set).
- Blocked signals are queued as **pending** and delivered when unblocked.
- Multiple pending signals of the same type are **merged** (not queued individually) in traditional POSIX signals.
- `sigpending()` queries which signals are currently pending.

---

#### `pending.c` — Resetting Signal Handlers

```c
void sig_quit_reset(int signo) {
    sleep(SLEEP);
    signal(SIGQUIT, SIG_DFL);   // reset to default (core dump)
}
```

Demonstrates the old `signal()` API's "reset-on-delivery" behavior. After the handler fires once, it's reset to default — so a second `^\` terminates the process. The modern `sigaction()` API avoids this by default.

---

#### `eintr.c` — Interrupted System Calls

When a signal is delivered while blocking in a syscall (like `read()`, `sleep()`), the syscall returns -1 with `errno = EINTR`. Portable code must handle this:

```c
restart:
    n = read(fd, buf, sizeof(buf));
    if (n == -1 && errno == EINTR)
        goto restart;   // or use a loop
```

Linux's `SA_RESTART` flag in `sigaction()` causes many syscalls to restart automatically.

---

## 08 — Interprocess Communication (IPC)

**Directory:** `08/`

### Core Concepts

IPC mechanisms allow separate processes to exchange data and synchronize.

| Mechanism | Persistence | Notes |
|-----------|-------------|-------|
| Pipes | Process lifecycle | Unidirectional, between related processes |
| FIFOs (named pipes) | Until deleted | Can be used by unrelated processes |
| Message queues (SysV) | Until removed | Typed messages, survive process exit |
| Semaphores (SysV) | Until removed | Synchronization primitives |
| Shared memory (SysV) | Until removed | Fastest IPC, requires synchronization |
| POSIX MQ (`mq_*`) | Until closed/unlinked | Better API than SysV MQ |

### Files

#### `pipe1.c` — Anonymous Pipe

```c
int fd[2];
pipe(fd);      // fd[0] = read end, fd[1] = write end

if ((pid = fork()) > 0) {
    // PARENT: writes to child
    close(fd[0]);                        // close unused read end
    snprintf(line, BUFSIZ, "Hello child! I'm your parent, pid %d!\n", getpid());
    write(fd[1], line, strlen(line));
    close(fd[1]);
} else {
    // CHILD: reads from parent
    close(fd[1]);                        // close unused write end
    n = read(fd[0], line, BUFSIZ);
    write(STDOUT_FILENO, line, n);
    close(fd[0]);
}
wait(NULL);
```

**Rules:**
- A pipe is **unidirectional** — data flows one way.
- Both ends are file descriptors; closing unused ends is critical.
- Reading from a pipe with no writers returns EOF (0 bytes).
- Writing to a pipe with no readers raises `SIGPIPE`.
- Buffer size is typically 64 KiB on Linux.

---

#### `pipe2.c` — Piping Between Parent and Child (bidirectional)

Uses two pipes to implement bidirectional communication. Each pipe is one-way; you need two for full-duplex.

---

#### `popen.c` — Connecting to Shell Commands

```c
FILE *fp = popen("ls -1", "r");
while (fgets(buf, BUFSIZ, fp) != NULL)
    puts(buf);
pclose(fp);
```

`popen()` creates a pipe and a child running `/bin/sh -c command`. The returned `FILE *` is connected to the command's stdin (write mode) or stdout (read mode). Always use `pclose()`, not `fclose()`, to reap the child process.

---

#### `msgsend.c` + `msgrecv.c` — System V Message Queues

```c
// Sender
key_t key = atoi(argv[1]);
int msqid = msgget(key, IPC_CREAT | 0644);

message_buf sbuf;
sbuf.mtype = 1;
strncpy(sbuf.mtext, argv[2], MSGSZ);
msgsnd(msqid, &sbuf, strlen(sbuf.mtext)+1, IPC_NOWAIT);

// Receiver
msgrcv(msqid, &rbuf, MSGSZ, 1, 0);   // type=1, blocking
```

**Key properties:**
- Queue persists after all processes exit — inspect with `ipcs -q`, remove with `ipcrm -q`.
- Messages have a **type** field for prioritized or selective retrieval.
- `IPC_NOWAIT` makes the call non-blocking (returns `EAGAIN` if full).

---

#### `shmdemo.c` — Shared Memory

```c
key_t key = ftok("./shmdemo.c", 42);         // derive key from file+id
int shmid = shmget(key, SHM_SIZE, 0666 | IPC_CREAT);
char *data = shmat(shmid, 0, 0);             // attach to address space

if (argc == 2)
    strncpy(data, argv[1], SHM_SIZE);        // write
else
    printf("%d: segment contains: \"%s\"\n", getpid(), data);  // read

shmdt(data);                                 // detach (not destroy)
// To destroy: shmctl(shmid, IPC_RMID, 0)
```

**Shared memory** is the fastest IPC because processes read/write directly to the same physical memory pages — no data copying occurs. However, it requires external synchronization (semaphores) to coordinate access.

---

#### `semdemo.c` — System V Semaphores

Semaphores implement mutual exclusion and synchronization between processes. A semaphore value ≥ 1 means a resource is available; `semop()` with sem_op = -1 decrements (waits), and +1 increments (signals).

---

#### `mqsend.c` + `mqrecv.c` — POSIX Message Queues

A cleaner alternative to SysV message queues:

```c
mqd_t mq = mq_open("/myqueue", O_CREAT|O_WRONLY, 0644, NULL);
mq_send(mq, message, strlen(message), priority);
mq_close(mq);
mq_unlink("/myqueue");   // removes the queue
```

---

## 09 — Network IPC & Sockets

**Directory:** `09/`

### Core Concepts

Sockets provide a uniform abstraction for network and local IPC. The socket API supports multiple domains and types:

| Domain | Usage |
|--------|-------|
| `PF_INET` | IPv4 networking |
| `PF_INET6` | IPv6 networking |
| `PF_LOCAL` / `AF_UNIX` | Local (on-host) IPC via filesystem paths |

| Type | Description |
|------|-------------|
| `SOCK_STREAM` | Reliable, ordered, connection-oriented (TCP) |
| `SOCK_DGRAM` | Unreliable, connectionless, message-based (UDP) |

### Files

#### `sockpipe.c` — UNIX Domain Socket (Local IPC)

```c
int sock = socket(PF_LOCAL, SOCK_DGRAM, 0);

struct sockaddr_un name;
name.sun_family = PF_LOCAL;
strncpy(name.sun_path, "socket", sizeof(name.sun_path));

if (child = fork()) {
    // Parent: bind and receive
    bind(sock, (struct sockaddr *)&name, sizeof(name));
    read(sock, buf, BUFSIZ);
} else {
    // Child: send to named socket
    sendto(sock, DATA, sizeof(DATA), 0, (struct sockaddr *)&name, ...);
}
unlink("socket");   // clean up the socket file
```

**Key point:** A UNIX domain socket is a filesystem object. If you don't `unlink()` it on exit, it persists and `bind()` will fail next time.

---

#### `streamread.c` — TCP Server (IPv6)

Implements a listening server that accepts TCP connections and prints received data:

```c
// Create IPv6 stream socket
int sock = socket(PF_INET6, SOCK_STREAM, 0);

// Bind to any address, OS-assigned port
struct sockaddr_in6 server = {0};
server.sin6_family = PF_INET6;
server.sin6_addr   = in6addr_any;
server.sin6_port   = 0;          // let OS pick port
bind(sock, (struct sockaddr *)&server, sizeof(server));

// Find out which port was assigned
getsockname(sock, (struct sockaddr *)&server, &length);
printf("Socket has port #%d\n", ntohs(server.sin6_port));

listen(sock, BACKLOG);

while (1) {
    int fd = accept(sock, (struct sockaddr *)&client, &length);
    // read/print data from client
    while ((rval = read(fd, buf, BUFSIZ)) > 0)
        printf("Client (%s) sent: \"%s\"\n", rip, buf);
    close(fd);
}
```

The server loop:
1. `listen()` marks the socket as passive (able to accept connections).
2. `accept()` blocks until a client connects, returning a new socket fd for that connection.
3. The *listening* socket remains open for future connections.

---

#### `streamwrite.c` — TCP Client

```c
struct addrinfo hints = {0}, *res;
hints.ai_family   = AF_INET6;
hints.ai_socktype = SOCK_STREAM;

getaddrinfo(hostname, port_str, &hints, &res);
int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
connect(sock, res->ai_addr, res->ai_addrlen);
write(sock, DATA, sizeof(DATA));
```

`getaddrinfo()` is the modern, protocol-agnostic name resolution function. It returns a linked list of `addrinfo` structs.

---

#### `dgramread.c` / `dgramsend.c` — UDP Communication

UDP is connectionless. The server uses `recvfrom()` to receive messages and knows the sender's address. The client uses `sendto()` to send without establishing a connection first.

---

#### `one-socket-select.c` — Multiplexing with `select()`

`select()` allows a single thread to monitor multiple file descriptors simultaneously, waking up when any of them is readable/writable:

```c
fd_set readfds;
FD_ZERO(&readfds);
FD_SET(sock1, &readfds);
FD_SET(sock2, &readfds);

struct timeval timeout = {5, 0};  // 5 second timeout
int n = select(maxfd+1, &readfds, NULL, NULL, &timeout);

if (FD_ISSET(sock1, &readfds)) { /* sock1 has data */ }
if (FD_ISSET(sock2, &readfds)) { /* sock2 has data */ }
```

---

## 11 — Linker, Shared Libraries & Dynamic Loading

**Directory:** `11/`

### Core Concepts

The linker resolves symbol references between object files and libraries.

| Library Type | Extension | Linked At |
|-------------|-----------|-----------|
| Static library | `.a` | Compile time (copied into binary) |
| Dynamic (shared) library | `.so` (Linux) / `.dylib` (macOS) | Runtime (loaded by `ld.so`) |

### Files

#### `dlopenex.c` — Runtime Dynamic Loading

Load and use a function from a shared library *without linking against it at compile time*:

```c
#include <dlfcn.h>

// Open the library at runtime
void *dlhandle = dlopen("libcrypt.so", RTLD_LAZY);

// Look up the symbol
char *(*_crypt)(const char *, const char *);
*(void **)(&_crypt) = dlsym(dlhandle, "crypt");

// Use it
printf("%s\n", _crypt(password, "$1"));
```

**Compilation:** `cc -Wall -rdynamic dlopenex.c` (no `-lcrypt`!)

**`RTLD_LAZY`**: Resolve symbols only when first called (vs `RTLD_NOW` which resolves all at load time).

The double-cast `*(void **)(&_crypt) = dlsym(...)` is the POSIX-compliant way to convert a `void *` to a function pointer (direct casting violates the C standard).

---

#### `ldtest1.c` / `ldtest2.c` — Symbol Resolution Order

Illustrates how the linker resolves multiply-defined symbols and how link order matters:

```bash
cc -c ldtest1.c && cc -c ldtest2.c
cc ldtest1.o ldtest2.o   # which definition wins?
```

The linker processes object files left-to-right. For static libraries, it only pulls in object files that define symbols needed by previously seen files — order matters.

---

#### `dlopenex-setget.c` — Exposing and Retrieving Global State

Demonstrates that with `RTLD_GLOBAL` (or `-rdynamic`), symbols from the main executable are visible to dynamically loaded libraries, and vice versa.

---

#### `elf.c` — ELF Binary Format Inspection

Reads and displays the ELF header of a binary, showing magic bytes, architecture, entry point, and section headers. The ELF (Executable and Linkable Format) is the standard binary format on Linux.

---

#### `crypt.c` — Cryptographic Hashing

```c
#include <unistd.h>
char *result = crypt(key, salt);
```

`crypt(3)` produces a one-way hash of `key` using the algorithm specified in `salt`. The `$1$` prefix in the salt selects MD5; `$6$` selects SHA-512.

---

## 12 — Advanced I/O & Security

**Directory:** `12/`

### Core Concepts

- **Non-blocking I/O:** Setting `O_NONBLOCK` on a file descriptor causes `read()`/`write()` to return immediately with `EAGAIN` instead of blocking.
- **File locking:** `flock()` and POSIX record locking (`fcntl(F_SETLK)`) prevent concurrent modification.
- **`EINTR` handling:** A signal can interrupt any blocking I/O call; robust code retries on `EINTR`.
- **Secure coding:** Secrets must be zeroed from memory (`memset`), not just dereferenced away.

### Files

#### `nonblock.c` — Non-Blocking Output

```c
// Get current flags
int flags = fcntl(STDOUT_FILENO, F_GETFL, 0);

// Enable non-blocking
fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

// Write loop: handles partial writes and EAGAIN
while (num > 0) {
    count = write(STDOUT_FILENO, ptr, num);
    if (count >= 0) {
        ptr += count;
        num -= count;
    } else {
        fprintf(stderr, "write error: %s\n", strerror(errno));
    }
}

// Restore flags
fcntl(STDOUT_FILENO, F_SETFL, flags);
```

When stdout is a pipe or socket with a full buffer, a non-blocking `write()` returns -1 with `errno = EAGAIN` — not an error, just "try again later".

---

#### `getpass.c` — Secrets and Password Handling

This program demonstrates several security pitfalls:

```c
// Anti-pattern: secret visible in argv (process table)
// ./a.out mypassword    → visible in 'ps aux' briefly

// Better: read from tty
secret = getpass("Password: ");    // reads from /dev/tty, disables echo

// Wrong way to "erase" a secret:
secret = "not a secret";    // WRONG: only changes the pointer, original bytes remain in memory

// Correct way:
memset(secret, 0, strlen(secret));  // overwrite the actual bytes

// Problem: printf() buffers the secret in user space!
// Use unbuffered write() instead:
write(STDOUT_FILENO, secret, strlen(secret));
```

Key security lessons:
1. Passing passwords as CLI arguments is a TOCTOU race — briefly visible in `/proc/PID/cmdline`.
2. Just reassigning a pointer doesn't erase the data — the bytes remain in memory.
3. Use `memset()` to zero sensitive data before freeing/returning.
4. Inspect core dumps with `strings a.out.core | grep password` to verify erasure.

---

#### `flock.c` — Advisory File Locking

```c
struct flock fl;
fl.l_type   = F_WRLCK;   // write (exclusive) lock
fl.l_whence = SEEK_SET;
fl.l_start  = 0;
fl.l_len    = 0;          // 0 = lock entire file

fcntl(fd, F_SETLKW, &fl);  // W = wait (blocking)
// ... critical section ...
fl.l_type = F_UNLCK;
fcntl(fd, F_SETLK, &fl);   // release lock
```

POSIX record locks are **advisory** — they only work if all cooperating processes check for them. They are associated with the `(PID, file)` pair, not the fd, so `close()`-ing any fd to the file releases all locks on it.

---

#### `const.c` — The `const` Qualifier and Memory

Explores what `const` means in C — it's a compiler directive, not a runtime guarantee. A `const char *` can still be cast to `char *` and modified (undefined behavior, but often works). Use `const` in function signatures to document read-only intent.

---

#### `pwhash.c` — Password Hashing with Salt

Demonstrates `crypt(3)` for one-way password hashing. The salt is embedded in the result, so verification is done by hashing the attempt with the same salt and comparing.

---

## 13 — Process Scheduling & Priority

**Directory:** `13/`

### Core Concepts

The UNIX scheduler assigns CPU time to processes based on priority. "Nice" values range from -20 (highest priority) to +19 (lowest priority). Regular users can only *lower* their priority (increase the nice value); root can raise it.

### Files

#### `priority.c` — Getting and Setting Process Priority

```c
// Get current priority
int p1 = getpriority(PRIO_PROCESS, getpid());
printf("My current priority is: %d\n", p1);

// Lower priority (nice = higher number = less CPU)
setpriority(PRIO_PROCESS, getpid(), n);

int p2 = getpriority(PRIO_PROCESS, getpid());
printf("My new priority is: %d\n", p2);

// Try to RAISE priority back — fails unless root
if (setpriority(PRIO_PROCESS, getpid(), p1) == -1)
    fprintf(stderr, "Unable to setpriority(): %s\n", strerror(errno));
```

**Run experiments:**
```bash
./a.out 5             # lower priority by 5
sudo ./a.out 5        # root can also raise priority
nice -n 10 ./a.out 5  # start at nice 10, then try to go to 15
nice -n -5 ./a.out 5  # root: start at -5 priority
```

**`PRIO_PROCESS`**: Set priority of a specific process (by PID). Alternatives: `PRIO_PGRP` (process group) and `PRIO_USER` (all processes of a user).

---

#### `break-chroot.c` — chroot Escape

Demonstrates the classic chroot escape technique:
1. Create a directory inside the chroot.
2. `chroot()` into the subdirectory.
3. Repeat many times to walk above the original chroot's root.

**Key lesson:** `chroot()` without `chdir("/")` into the new root is a security vulnerability. Also, root inside a chroot can still escape — use containers (namespaces + seccomp) for proper isolation.

---

#### `busy.c` — CPU-Bound Process

A tight infinite loop (`while(1) {}`) that consumes 100% CPU. Useful for:
- Observing scheduling behavior under load.
- Testing `nice` / `setpriority`.
- Profiling with `perf` or `gprof`.

---

## Building the Code

```bash
# Build a single file
cc -Wall -Werror -Wextra -o mybin source.c

# Some files have Makefiles
cd 02/ && make

# Run a specific chapter's code
cd 01/ && cc -o simple-cat simple-cat.c && ./simple-cat < simple-cat.c
```

**Common compiler flags:**
| Flag | Purpose |
|------|---------|
| `-Wall` | Enable all standard warnings |
| `-Werror` | Treat warnings as errors |
| `-Wextra` | Enable extra warnings |
| `-g` | Include debug symbols (for GDB) |
| `-O2` | Optimization level 2 |
| `-DFOO` | Define preprocessor macro `FOO` |
| `-rdynamic` | Export all symbols to dynamic linker |

---

## Key UNIX System Calls Reference

| Category | Syscall | Description |
|----------|---------|-------------|
| **Processes** | `fork()` | Create child process |
| | `exec*()` | Replace process image |
| | `wait()` / `waitpid()` | Collect child exit status |
| | `getpid()` / `getppid()` | Get process IDs |
| | `exit()` / `_exit()` | Terminate process |
| **File I/O** | `open()` / `creat()` | Open/create file |
| | `read()` / `write()` | Unbuffered I/O |
| | `lseek()` | Reposition file offset |
| | `close()` | Close file descriptor |
| | `dup()` / `dup2()` | Duplicate file descriptor |
| **File System** | `stat()` / `lstat()` | Get file metadata |
| | `chmod()` / `chown()` | Change permission/ownership |
| | `mkdir()` / `rmdir()` | Directory operations |
| | `link()` / `unlink()` | Create/remove hard links |
| | `symlink()` / `readlink()` | Symbolic links |
| | `rename()` | Rename/move files |
| **Signals** | `signal()` / `sigaction()` | Install signal handler |
| | `kill()` | Send signal to process |
| | `sigprocmask()` | Block/unblock signals |
| | `sigpending()` | Query pending signals |
| | `sigsuspend()` | Atomically unmask and suspend |
| **IPC** | `pipe()` | Create anonymous pipe |
| | `msgget()` / `msgsnd()` / `msgrcv()` | System V message queues |
| | `shmget()` / `shmat()` / `shmdt()` | System V shared memory |
| | `semget()` / `semop()` | System V semaphores |
| **Sockets** | `socket()` | Create socket |
| | `bind()` | Assign address |
| | `listen()` / `accept()` | Accept connections |
| | `connect()` | Initiate connection |
| | `send()` / `recv()` | Socket I/O |
| | `select()` / `poll()` | I/O multiplexing |
| **User/Group** | `getuid()` / `geteuid()` | Get real/effective UID |
| | `setuid()` / `seteuid()` | Set UID |
| | `umask()` | Set file creation mask |

---

## Further Reading

- **Book:** W. Richard Stevens & Stephen A. Rago, *Advanced Programming in the UNIX Environment*, 3rd Ed.
- **Course:** https://stevens.netmeister.org/631/
- **Man Pages:** `man 2 <syscall>` (system calls), `man 3 <function>` (library functions)
- **POSIX Standard:** https://pubs.opengroup.org/onlinepubs/9699919799/
