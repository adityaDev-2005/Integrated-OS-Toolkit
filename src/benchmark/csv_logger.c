#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "benchmark.h"

static void mark_failed(BenchmarkCsv *csv)
{
    if (csv != NULL)
        csv->failed = 1;
}

static void write_separator(BenchmarkCsv *csv)
{
    if (csv == NULL || csv->stream == NULL || csv->failed)
        return;

    if (!csv->first_field && fputc(',', csv->stream) == EOF) {
        mark_failed(csv);
        return;
    }

    csv->first_field = 0;
}

static void write_quoted(BenchmarkCsv *csv, const char *text)
{
    if (fputc('"', csv->stream) == EOF) {
        mark_failed(csv);
        return;
    }

    for (const char *cursor = text; *cursor != '\0'; cursor++) {
        if (*cursor == '"' && fputc('"', csv->stream) == EOF) {
            mark_failed(csv);
            return;
        }

        if (fputc((unsigned char)*cursor, csv->stream) == EOF) {
            mark_failed(csv);
            return;
        }
    }

    if (fputc('"', csv->stream) == EOF)
        mark_failed(csv);
}

int benchmark_csv_open(BenchmarkCsv *csv, const char *filename,
                       const char *label)
{
    char data_dir[PROJECT_PATH_MAX];
    char results_dir[PROJECT_PATH_MAX];
    int written;

    if (csv == NULL || filename == NULL)
        return 0;

    memset(csv, 0, sizeof(*csv));
    csv->label = label != NULL ? label : "CSV report";
    csv->failed = 1;

    written = snprintf(csv->relative, sizeof(csv->relative),
                       "data/results/%s", filename);

    if (written < 0 || (size_t)written >= sizeof(csv->relative)) {
        fprintf(stderr, "The %s filename is too long.\n", csv->label);
        return 0;
    }

    if (!project_path(data_dir, sizeof(data_dir), "data") ||
        !project_path(results_dir, sizeof(results_dir), "data/results") ||
        !project_path(csv->path, sizeof(csv->path), csv->relative)) {
        fprintf(stderr, "Could not resolve the %s paths under the project "
                        "root.\n", csv->label);
        return 0;
    }

    if (mkdir(data_dir, 0777) != 0 && errno != EEXIST) {
        fprintf(stderr, "Creating data directory for %s: %s\n",
                csv->label, strerror(errno));
        return 0;
    }

    if (mkdir(results_dir, 0777) != 0 && errno != EEXIST) {
        fprintf(stderr, "Creating results directory for %s: %s\n",
                csv->label, strerror(errno));
        return 0;
    }

    csv->stream = fopen(csv->path, "w");

    if (csv->stream == NULL) {
        fprintf(stderr, "Could not open %s: %s\n",
                csv->relative, strerror(errno));
        return 0;
    }

    csv->failed = 0;
    csv->first_field = 1;
    return 1;
}

void benchmark_csv_header(BenchmarkCsv *csv, const char *const fields[],
                          size_t field_count)
{
    if (csv == NULL || fields == NULL)
        return;

    benchmark_csv_row_begin(csv);

    for (size_t i = 0; i < field_count; i++)
        benchmark_csv_text(csv, fields[i]);

    benchmark_csv_row_end(csv);
}

void benchmark_csv_row_begin(BenchmarkCsv *csv)
{
    if (csv == NULL)
        return;

    csv->first_field = 1;
}

void benchmark_csv_text(BenchmarkCsv *csv, const char *text)
{
    if (csv == NULL || csv->stream == NULL || csv->failed)
        return;

    if (text == NULL)
        text = "";

    write_separator(csv);

    if (csv->failed)
        return;

    if (strpbrk(text, ",\"\r\n") != NULL) {
        write_quoted(csv, text);
        return;
    }

    if (fputs(text, csv->stream) == EOF)
        mark_failed(csv);
}

void benchmark_csv_int(BenchmarkCsv *csv, long long value)
{
    if (csv == NULL || csv->stream == NULL || csv->failed)
        return;

    write_separator(csv);

    if (!csv->failed && fprintf(csv->stream, "%lld", value) < 0)
        mark_failed(csv);
}

void benchmark_csv_double(BenchmarkCsv *csv, double value)
{
    if (csv == NULL || csv->stream == NULL || csv->failed)
        return;

    write_separator(csv);

    if (!csv->failed && fprintf(csv->stream, "%.2f", value) < 0)
        mark_failed(csv);
}

void benchmark_csv_row_end(BenchmarkCsv *csv)
{
    if (csv == NULL || csv->stream == NULL || csv->failed)
        return;

    if (fputc('\n', csv->stream) == EOF)
        mark_failed(csv);
}

int benchmark_csv_close(BenchmarkCsv *csv)
{
    int failed;

    if (csv == NULL || csv->stream == NULL)
        return 0;

    failed = csv->failed || ferror(csv->stream);

    if (fclose(csv->stream) != 0)
        failed = 1;

    csv->stream = NULL;

    if (!failed)
        return 1;

    fprintf(stderr, "Failed to write %s\n", csv->relative);

    /* Do not leave a partial report that could be mistaken for a result. */
    if (remove(csv->path) != 0 && errno != ENOENT) {
        fprintf(stderr, "Could not remove partial %s: %s\n",
                csv->relative, strerror(errno));
    }

    return 0;
}

const char *benchmark_csv_relative(const BenchmarkCsv *csv)
{
    return csv != NULL ? csv->relative : "";
}
