#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

/* The binary lives in <root>/build/, so strip "os_toolkit" and "build". */
#define EXECUTABLE_DEPTH 2

static int executable_root(char *buffer, size_t size)
{
    char executable[PROJECT_PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", executable,
                              sizeof(executable) - 1);

    if (length <= 0 || (size_t)length >= sizeof(executable) - 1)
        return 0;

    executable[length] = '\0';

    for (int i = 0; i < EXECUTABLE_DEPTH; i++) {
        char *slash = strrchr(executable, '/');

        if (slash == NULL)
            return 0;

        if (slash == executable)
            slash[1] = '\0';
        else
            *slash = '\0';
    }

    size_t length_of_root = strlen(executable);

    if (length_of_root + 1 > size)
        return 0;

    memcpy(buffer, executable, length_of_root + 1);
    return 1;
}

int project_path(char *buffer, size_t size, const char *relative)
{
    char resolved[PROJECT_PATH_MAX];
    const char *root = getenv("OS_TOOLKIT_ROOT");

    if (root == NULL || root[0] == '\0') {
        if (!executable_root(resolved, sizeof(resolved)))
            return 0;

        root = resolved;
    }

    /* Ignore trailing slashes so "/" and "/root/" join cleanly. */
    size_t root_length = strlen(root);

    while (root_length > 1 && root[root_length - 1] == '/')
        root_length--;

    int written = snprintf(buffer, size, "%.*s/%s",
                           (int)root_length, root, relative);

    return written >= 0 && (size_t)written < size;
}

int read_integer(const char *prompt, int min, int max, int *value)
{
    char buffer[128];
    char *end;
    long number;

    for (;;) {
        printf("%s", prompt);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            return 0;

        errno = 0;
        number = strtol(buffer, &end, 10);

        if (end == buffer || errno == ERANGE) {
            printf("Invalid input. Enter an integer from %d to %d.\n",
                   min, max);
            continue;
        }

        while (*end == ' ' || *end == '\t' ||
               *end == '\n' || *end == '\r')
            end++;

        if (*end != '\0' || number < min || number > max) {
            printf("Invalid input. Enter an integer from %d to %d.\n",
                   min, max);
            continue;
        }

        *value = (int)number;
        return 1;
    }
}
