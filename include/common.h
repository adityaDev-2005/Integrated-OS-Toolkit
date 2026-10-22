#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>

#define PROJECT_PATH_MAX 4096

/*
 * Build an absolute path inside the project root.
 *
 * Root resolution order:
 *   1. OS_TOOLKIT_ROOT, when set and not empty.
 *   2. The directory two levels above the running executable, because the
 *      Makefile links the binary into <root>/build/.
 *
 * Returns 1 on success, 0 when the root cannot be resolved or the result does
 * not fit in `size` bytes.
 */
int project_path(char *buffer, size_t size, const char *relative);

/*
 * Read a validated integer in [min, max] from stdin, re-prompting on invalid
 * input. Returns 1 on success and 0 on end of input.
 */
int read_integer(const char *prompt, int min, int max, int *value);

#endif
