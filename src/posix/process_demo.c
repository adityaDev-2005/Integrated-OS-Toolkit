#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "process_demo.h"

#define TEMP_FILE_TEMPLATE "/tmp/os_toolkit_posix_XXXXXX"
#define DIRECTORY_TO_LIST "/tmp"
#define DIRECTORY_ENTRY_LIMIT 5

static int waitpid_for_child(pid_t pid, int *status, const char *label)
{
    pid_t reaped;

    do {
        reaped = waitpid(pid, status, 0);
    } while (reaped == -1 && errno == EINTR);

    if (reaped == -1) {
        fprintf(stderr, "[process] waitpid (%s): %s\n", label, strerror(errno));
        return 0;
    }

    if (reaped != pid) {
        fprintf(stderr, "[process] waitpid (%s): expected %ld, got %ld\n",
                label, (long)pid, (long)reaped);
        return 0;
    }

    return 1;
}

static void report_abnormal_exit(const char *label, pid_t pid, int status)
{
    if (WIFSIGNALED(status)) {
        fprintf(stderr, "[process] %s: child %ld terminated by signal %d\n",
                label, (long)pid, WTERMSIG(status));
    } else {
        fprintf(stderr, "[process] %s: child %ld did not exit normally\n",
                label, (long)pid);
    }
}

static int demo_waitpid(void)
{
    fflush(stdout);

    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "[process] fork (waitpid child): %s\n", strerror(errno));
        return 0;
    }

    if (pid == 0) {
        printf("[process] waitpid child: pid=%ld ppid=%ld\n",
               (long)getpid(), (long)getppid());
        fflush(stdout);
        _exit(42);
    }

    int status = 0;

    if (!waitpid_for_child(pid, &status, "waitpid"))
        return 0;

    if (!WIFEXITED(status)) {
        report_abnormal_exit("waitpid", pid, status);
        return 0;
    }

    printf("[process] waitpid: reaped child %ld, exit status %d\n",
           (long)pid, WEXITSTATUS(status));

    return WEXITSTATUS(status) == 42;
}

static int demo_wait(void)
{
    fflush(stdout);

    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "[process] fork (wait child): %s\n", strerror(errno));
        return 0;
    }

    if (pid == 0) {
        printf("[process] wait child: pid=%ld ppid=%ld\n",
               (long)getpid(), (long)getppid());
        fflush(stdout);
        _exit(7);
    }

    int status = 0;
    pid_t reaped;

    do {
        reaped = wait(&status);
    } while (reaped == -1 && errno == EINTR);

    if (reaped == -1) {
        fprintf(stderr, "[process] wait: %s\n", strerror(errno));
        return 0;
    }

    if (reaped != pid) {
        fprintf(stderr, "[process] wait: expected %ld, got %ld\n",
                (long)pid, (long)reaped);
        return 0;
    }

    if (!WIFEXITED(status)) {
        report_abnormal_exit("wait", pid, status);
        return 0;
    }

    printf("[process] wait: reaped child %ld, exit status %d\n",
           (long)pid, WEXITSTATUS(status));

    return WEXITSTATUS(status) == 7;
}

static int demo_exec_success(void)
{
    fflush(stdout);

    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "[exec] fork (successful exec): %s\n", strerror(errno));
        return 0;
    }

    if (pid == 0) {
        printf("[exec] child before exec: pid=%ld ppid=%ld\n",
               (long)getpid(), (long)getppid());
        fflush(stdout);

        execlp("echo", "echo",
               "[exec] execlp: child image replaced successfully",
               (char *)NULL);

        int saved_errno = errno;

        fprintf(stderr, "[exec] execlp failed: %s\n", strerror(saved_errno));
        _exit(127);
    }

    int status = 0;

    if (!waitpid_for_child(pid, &status, "successful exec"))
        return 0;

    if (!WIFEXITED(status)) {
        report_abnormal_exit("exec", pid, status);
        return 0;
    }

    if (WEXITSTATUS(status) != 0) {
        fprintf(stderr, "[exec] execlp child exit status: %d\n",
                WEXITSTATUS(status));
        return 0;
    }

    printf("[exec] waitpid: successful exec child exited with status 0\n");
    return 1;
}

static int demo_exec_failure(void)
{
    fflush(stdout);

    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr, "[exec] fork (failed exec): %s\n", strerror(errno));
        return 0;
    }

    if (pid == 0) {
        printf("[exec] child before failed exec: pid=%ld ppid=%ld\n",
               (long)getpid(), (long)getppid());
        fflush(stdout);

        execl("/nonexistent/os_toolkit_exec_demo",
              "os_toolkit_exec_demo", (char *)NULL);

        int saved_errno = errno;

        fprintf(stderr, "[exec] expected execl failure: %s\n",
                strerror(saved_errno));
        _exit(127);
    }

    int status = 0;

    if (!waitpid_for_child(pid, &status, "failed exec"))
        return 0;

    if (!WIFEXITED(status)) {
        report_abnormal_exit("exec", pid, status);
        return 0;
    }

    if (WEXITSTATUS(status) != 127) {
        fprintf(stderr, "[exec] failed exec child exit status: %d\n",
                WEXITSTATUS(status));
        return 0;
    }

    printf("[exec] failed exec: child exited with status 127 (expected)\n");
    return 1;
}

static int write_all(int fd, const char *data, size_t length)
{
    size_t total = 0;

    while (total < length) {
        ssize_t written = write(fd, data + total, length - total);

        if (written < 0) {
            if (errno == EINTR)
                continue;
            return 0;
        }

        if (written == 0) {
            errno = EIO;
            return 0;
        }

        total += (size_t)written;
    }

    return 1;
}

static ssize_t read_retry(int fd, void *buffer, size_t length)
{
    ssize_t bytes;

    do {
        bytes = read(fd, buffer, length);
    } while (bytes == -1 && errno == EINTR);

    return bytes;
}

static int demo_file_apis(void)
{
    char path[] = TEMP_FILE_TEMPLATE;
    int fd = -1;
    int ok = 0;

    int created_fd = mkstemp(path);

    if (created_fd == -1) {
        fprintf(stderr, "[file] mkstemp: %s\n", strerror(errno));
        return 0;
    }

    printf("[file] temporary file created: %s\n", path);

    if (close(created_fd) == -1) {
        fprintf(stderr, "[file] close (mkstemp): %s\n", strerror(errno));
        goto cleanup;
    }

    fd = open(path, O_RDWR);

    if (fd == -1) {
        fprintf(stderr, "[file] open: %s\n", strerror(errno));
        goto cleanup;
    }

    printf("[file] open: O_RDWR descriptor acquired\n");

    int flags = fcntl(fd, F_GETFL);

    if (flags == -1) {
        fprintf(stderr, "[file] fcntl F_GETFL: %s\n", strerror(errno));
        goto cleanup;
    }

    printf("[file] fcntl F_GETFL: O_APPEND %s\n",
           (flags & O_APPEND) ? "set" : "not set");

    if (fcntl(fd, F_SETFL, flags | O_APPEND) == -1) {
        fprintf(stderr, "[file] fcntl F_SETFL: %s\n", strerror(errno));
        goto cleanup;
    }

    int updated_flags = fcntl(fd, F_GETFL);

    if (updated_flags == -1) {
        fprintf(stderr, "[file] fcntl F_GETFL after set: %s\n",
                strerror(errno));
        goto cleanup;
    }

    if (!(updated_flags & O_APPEND)) {
        fprintf(stderr, "[file] fcntl: O_APPEND did not remain set\n");
        goto cleanup;
    }

    printf("[file] fcntl F_SETFL: O_APPEND set\n");

    const char *first = "hello";
    const char *second = " world";

    if (!write_all(fd, first, strlen(first))) {
        fprintf(stderr, "[file] write (first): %s\n", strerror(errno));
        goto cleanup;
    }

    printf("[file] write: %ld bytes (\"%s\")\n",
           (long)strlen(first), first);

    if (!write_all(fd, second, strlen(second))) {
        fprintf(stderr, "[file] write (second): %s\n", strerror(errno));
        goto cleanup;
    }

    printf("[file] write: %ld bytes (\"%s\")\n",
           (long)strlen(second), second);

    off_t offset = lseek(fd, 0, SEEK_SET);

    if (offset == (off_t)-1) {
        fprintf(stderr, "[file] lseek (start): %s\n", strerror(errno));
        goto cleanup;
    }

    printf("[file] lseek: offset %lld\n", (long long)offset);

    char buffer[64];
    ssize_t bytes = read_retry(fd, buffer, sizeof(buffer) - 1);

    if (bytes == -1) {
        fprintf(stderr, "[file] read: %s\n", strerror(errno));
        goto cleanup;
    }

    if (bytes == 0) {
        fprintf(stderr, "[file] read: unexpected end of file\n");
        goto cleanup;
    }

    buffer[bytes] = '\0';
    printf("[file] read back: \"%s\" (%ld bytes)\n", buffer, (long)bytes);

    if (strcmp(buffer, "hello world") != 0) {
        fprintf(stderr, "[file] read: content mismatch\n");
        goto cleanup;
    }

    offset = lseek(fd, 6, SEEK_SET);

    if (offset == (off_t)-1) {
        fprintf(stderr, "[file] lseek (offset 6): %s\n", strerror(errno));
        goto cleanup;
    }

    printf("[file] lseek: offset %lld\n", (long long)offset);

    bytes = read_retry(fd, buffer, sizeof(buffer) - 1);

    if (bytes == -1) {
        fprintf(stderr, "[file] read after lseek: %s\n", strerror(errno));
        goto cleanup;
    }

    if (bytes == 0) {
        fprintf(stderr, "[file] read after lseek: unexpected end of file\n");
        goto cleanup;
    }

    buffer[bytes] = '\0';
    printf("[file] lseek+read from offset 6: \"%s\" (%ld bytes)\n",
           buffer, (long)bytes);

    if (strcmp(buffer, "world") != 0) {
        fprintf(stderr, "[file] read after lseek: content mismatch\n");
        goto cleanup;
    }

    struct stat info;

    if (stat(path, &info) == -1) {
        fprintf(stderr, "[file] stat: %s\n", strerror(errno));
        goto cleanup;
    }

    printf("[file] stat: size=%lld bytes, mode=%o\n",
           (long long)info.st_size,
           (unsigned)(info.st_mode & 07777));

    if (info.st_size != (off_t)(strlen(first) + strlen(second))) {
        fprintf(stderr, "[file] stat: unexpected size\n");
        goto cleanup;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "[file] close: %s\n", strerror(errno));
        fd = -1;
        goto cleanup;
    }

    fd = -1;
    printf("[file] close: descriptor released\n");
    ok = 1;

cleanup:
    if (fd >= 0 && close(fd) == -1)
        fprintf(stderr, "[file] cleanup close: %s\n", strerror(errno));

    if (unlink(path) == -1) {
        fprintf(stderr, "[file] unlink: %s\n", strerror(errno));
        return 0;
    }

    printf("[file] cleanup: temporary file unlinked\n");
    return ok;
}

static int demo_directory_apis(void)
{
    DIR *directory = opendir(DIRECTORY_TO_LIST);

    if (directory == NULL) {
        fprintf(stderr, "[directory] opendir %s: %s\n",
                DIRECTORY_TO_LIST, strerror(errno));
        return 0;
    }

    printf("[directory] opendir: %s\n", DIRECTORY_TO_LIST);

    int count = 0;

    for (;;) {
        errno = 0;
        struct dirent *entry = readdir(directory);

        if (entry == NULL) {
            if (errno != 0) {
                fprintf(stderr, "[directory] readdir: %s\n", strerror(errno));
                if (closedir(directory) == -1)
                    fprintf(stderr, "[directory] closedir: %s\n",
                            strerror(errno));
                return 0;
            }
            break;
        }

        printf("[directory] readdir: %s\n", entry->d_name);
        count++;

        if (count == DIRECTORY_ENTRY_LIMIT)
            break;
    }

    printf("[directory] readdir: listed %d entries\n", count);

    if (closedir(directory) == -1) {
        fprintf(stderr, "[directory] closedir: %s\n", strerror(errno));
        return 0;
    }

    printf("[directory] closedir: directory stream closed\n");

    return count > 0;
}

void run_posix_demo(void)
{
    int ok = 1;

    printf("\n========== POSIX PROCESS AND FILE DEMO ==========\n");
    printf("These results come from real Linux system calls; none are simulated.\n\n");
    printf("[process] parent: pid=%ld ppid=%ld\n",
           (long)getpid(), (long)getppid());

    if (!demo_waitpid())
        ok = 0;

    if (!demo_wait())
        ok = 0;

    if (!demo_exec_success())
        ok = 0;

    if (!demo_exec_failure())
        ok = 0;

    if (!demo_file_apis())
        ok = 0;

    if (!demo_directory_apis())
        ok = 0;

    printf("\nPOSIX demonstration: %s\n",
           ok ? "all checks passed" : "completed with errors");
}
