#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stddef.h>
#include <stdio.h>

#include "common.h"

#define BENCHMARK_CSV_NAME_MAX 128

/*
 * Shared CSV writer for algorithm-comparison reports.
 *
 * All reports are written under data/results/ and use one row per algorithm,
 * an `algorithm` first column, snake_case metric names and the same escaping
 * rules. Module-specific columns follow the metrics.
 */
typedef struct {
    FILE *stream;
    char relative[BENCHMARK_CSV_NAME_MAX];
    char path[PROJECT_PATH_MAX];
    const char *label;
    int failed;
    int first_field;
} BenchmarkCsv;

int benchmark_csv_open(BenchmarkCsv *csv, const char *filename,
                       const char *label);
void benchmark_csv_header(BenchmarkCsv *csv, const char *const fields[],
                          size_t field_count);
void benchmark_csv_row_begin(BenchmarkCsv *csv);
void benchmark_csv_text(BenchmarkCsv *csv, const char *text);
void benchmark_csv_int(BenchmarkCsv *csv, long long value);
void benchmark_csv_double(BenchmarkCsv *csv, double value);
void benchmark_csv_row_end(BenchmarkCsv *csv);
int benchmark_csv_close(BenchmarkCsv *csv);
const char *benchmark_csv_relative(const BenchmarkCsv *csv);

/* Consistent console messages for successful and failed CSV export. */
void benchmark_report_csv_saved(const char *relative);
void benchmark_report_csv_failure(void);

/*
 * Run scripts/plot_results.py for one module. The script reads the module's
 * CSV and writes image_relative. Returns 1 when the image was generated,
 * 0 when plotting was skipped or failed. A plotting failure never affects the
 * algorithm results or the already-written CSV.
 */
int benchmark_generate_plot(const char *module, const char *image_relative,
                            const char *description);

#endif
