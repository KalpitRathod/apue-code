/* ============================================================================
 * The Magic of Dynamic Loading
 * ============================================================================
 * In early UNIX, binaries were massive because they contained all their library code (static linking). In the 90s, 'Dynamic Shared Objects' became standard. dlopen() allows a running program to open a library file, look up a function by its string name, and execute new code on the fly.
 * ============================================================================
 */
/* This program can be used to illustrate that we can load symbols from
 * dynamic libraries at runtime without actually linking against them.
 *
 * Note that we do _not_ link against libcrypt, nor do we include the
 * unistd.h header.
 *
 * cc -Wall -Werror -Wextra crypt.c
 * cc -Wall -Werror -Wextra -rdynamic dlopenex.c
 */

#include <err.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
printCrypt(const char *s) {
	void *dlhandle;
	char *(*_crypt)(const char *, const char *);
	char *error;

	dlhandle = dlopen("libcrypt.so", RTLD_LAZY);
	if (!dlhandle) {
		err(EXIT_FAILURE, "%s\n", dlerror());
		/* NOTREACHED */
	}

	*(void **) (&_crypt) = dlsym(dlhandle, "crypt");
	if ((error = dlerror()) != NULL)  {
		err(EXIT_FAILURE, "%s\n", error);
		/* NOTREACHED */
	}

	(void)printf("%s\n", _crypt(s, "$1"));
}

int
main(int argc, char **argv) {
	if (argc != 2) {
		(void)fprintf(stderr, "Usage: %s string\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	printCrypt(argv[1]);
	exit(EXIT_SUCCESS);
}

/* ============================================================================
 * DOCUMENTATION
 * ============================================================================
 *
 * INTENT:
 *   Demonstrates runtime dynamic library loading using dlopen/dlsym. The
 *   program uses libcrypt WITHOUT linking against it at compile time and
 *   WITHOUT including the crypt.h header. The symbol is resolved at runtime.
 *   This technique is used by plugin systems, optional feature loading, and
 *   programs that need to run on systems where a library may not be present.
 *   Compile: cc -Wall -Werror -Wextra -rdynamic dlopenex.c  (no -lcrypt!)
 *
 * MACROS:
 *   RTLD_LAZY      - dlopen() flag: resolve symbols lazily (on first use).
 *                    Alternative: RTLD_NOW (resolve all at load time).
 *   EXIT_FAILURE   - 1; returned on bad args or dlopen/dlsym failure.
 *   EXIT_SUCCESS   - 0; returned on success.
 *
 * VARIABLES:
 *   void *dlhandle - Opaque handle to the opened shared library. Used as a
 *                    token for all subsequent dlsym() and dlclose() calls.
 *   char *(*_crypt)(const char *, const char *) - Function pointer variable
 *                    holding the address of the crypt() function looked up
 *                    via dlsym(). The double-cast idiom is used to convert
 *                    void* → function pointer (required by C standard).
 *   char *error    - Receives error string from dlerror() after dlsym();
 *                    dlerror() returns NULL if no error occurred.
 *
 * FUNCTIONS:
 *   printCrypt(s)  - Opens libcrypt.so at runtime; looks up "crypt" symbol;
 *                    calls it and prints the hash of string s.
 *   main(int, char**) - Validates argc; calls printCrypt(argv[1]).
 *   dlopen(path, flags) - Loads the shared library into the process.
 *                    Returns NULL on failure; call dlerror() after.
 *   dlsym(handle, symbol) - Looks up symbol name in the loaded library.
 *                    Returns void*; must be cast to the correct function type.
 *                    Returns NULL both on error AND if symbol maps to NULL —
 *                    always call dlerror() after to distinguish.
 *   dlerror()      - Returns human-readable error string for last dl* failure.
 *                    Clears the error state on each call.
 *   err(status, msg) - Prints error and exits.
 *   printf()       - Prints the computed hash.
 *   exit(status)   - Terminates.
 *
 * ALGORITHM:
 *   0. main(): validate argc==2; call printCrypt(argv[1]).
 *   1. dlopen("libcrypt.so", RTLD_LAZY) → load library; get handle.
 *   2. dlerror() → clear any stale error.
 *   3. dlsym(dlhandle, "crypt") → look up the crypt function symbol.
 *      The double-cast: *(void **)(&_crypt) = dlsym(...) is POSIX-required
 *      because C99 doesn't allow direct void* → function pointer assignment.
 *   4. dlerror() → if non-NULL, symbol not found; exit with error.
 *   5. Call _crypt(s, "$1") → MD5-based hash of s with salt "$1".
 *   6. printf the result.
 *
 * KEY SYSCALLS / LIBRARY FUNCTIONS:
 *   dlopen(3)    - Opens a shared object; handled by ld.so (dynamic linker).
 *   dlsym(3)     - Looks up symbol address in a dll handle; returns void*.
 *   dlerror(3)   - Returns last dynamic linking error string.
 *   dlclose(3)   - Closes handle (not called here; process exits anyway).
 *
 * NOTE ON THE DOUBLE-CAST:
 *   *(void **)(&_crypt) = dlsym(...)
 *   This casts &_crypt (a pointer-to-function-pointer) to (void**) and
 *   writes the void* from dlsym through it. The alternative (cast of void*
 *   directly to a function pointer) is undefined behavior per C99 §6.3.2.3.
 *
 * ============================================================================
 */
