#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "benchmark.h"

#define PLOT_COMMAND_EXTRA 256

void benchmark_report_csv_saved(const char *relative)
{
    printf("\nComparison data saved to %s\n", relative);
}

void benchmark_report_csv_failure(void)
{
    printf("\nCSV export failed; the algorithm results above are still valid.\n");
}

static int run_python_script(const char *python, const char *script,
                             const char *module)
{
    char command[PROJECT_PATH_MAX + PLOT_COMMAND_EXTRA];
    int length = snprintf(command, sizeof(command), "\"%s\" \"%s\" \"%s\"",
                          python, script, module);

    if (length < 0 || (size_t)length >= sizeof(command))
        return 0;

    return system(command) == 0;
}

static int open_image(const char *image)
{
    char command[PROJECT_PATH_MAX + 32];
    int length;

    if (getenv("OS_TOOLKIT_NO_OPEN") != NULL)
        return 1;

    if (getenv("DISPLAY") == NULL && getenv("WAYLAND_DISPLAY") == NULL) {
        printf("No desktop session detected; open the image manually.\n");
        return 1;
    }

    /* No trailing '&': a backgrounded command always reports success. */
    length = snprintf(command, sizeof(command),
                      "xdg-open \"%s\" >/dev/null 2>&1", image);

    if (length < 0 || (size_t)length >= sizeof(command)) {
        printf("The graph was saved, but its path is too long to open.\n");
        return 1;
    }

    if (system(command) != 0) {
        printf("The graph was saved, but could not be opened automatically.\n");
    }

    return 1;
}

int benchmark_generate_plot(const char *module, const char *image_relative,
                            const char *description)
{
    char script[PROJECT_PATH_MAX];
    char image[PROJECT_PATH_MAX];
    char venv_python[PROJECT_PATH_MAX];
    int generated = 0;

    if (getenv("OS_TOOLKIT_NO_PLOT") != NULL) {
        printf("\nPlot generation skipped (OS_TOOLKIT_NO_PLOT is set).\n");
        return 0;
    }

    if (!project_path(script, sizeof(script), "scripts/plot_results.py") ||
        !project_path(image, sizeof(image), image_relative) ||
        !project_path(venv_python, sizeof(venv_python),
                      ".venv/bin/python")) {
        printf("\nCould not resolve the plot paths under the project root.\n");
        return 0;
    }

    printf("\nGenerating %s...\n", description);

    /* Prefer the project's virtual environment. */
    if (access(venv_python, X_OK) == 0)
        generated = run_python_script(venv_python, script, module);

    /* Fall back to the system Python if needed. */
    if (!generated) {
        printf("Trying system Python...\n");
        generated = run_python_script("python3", script, module);
    }

    if (!generated || access(image, R_OK) != 0) {
        printf("Could not generate the graph.\n");
        printf("Check that Matplotlib and the plotting script are installed.\n");
        printf("The CSV file and the algorithm results are still valid.\n");
        return 0;
    }

    printf("Graph saved to %s\n", image_relative);
    return open_image(image);
}
