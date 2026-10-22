import csv
import math
import sys
from pathlib import Path

try:
    import matplotlib.pyplot as plt
except ImportError:
    print("Error: Matplotlib is not installed for this Python interpreter.")
    sys.exit(1)

# Resolve paths relative to the project, not the terminal's directory.
PROJECT_ROOT = Path(__file__).resolve().parent.parent
RESULTS_DIR = PROJECT_ROOT / "data" / "results"

# Each comparison report has one row per algorithm and only simulated metrics.
# The CPU layout intentionally matches the original three-panel chart.
PLOT_SPECS = {
    "cpu_scheduling": {
        "csv": "cpu_scheduling.csv",
        "output": "cpu_scheduling_comparison.png",
        "title": "CPU Scheduling Algorithm Comparison",
        "x_label": "Scheduling algorithm",
        "layout": (1, 3),
        "figsize": (15, 5),
        "metrics": [
            ("avg_waiting_time", "Average Waiting Time", "Time units", "%.2f"),
            ("avg_turnaround_time", "Average Turnaround Time", "Time units",
             "%.2f"),
            ("avg_response_time", "Average Response Time", "Time units",
             "%.2f"),
        ],
    },
    "page_replacement": {
        "csv": "page_replacement.csv",
        "output": "page_replacement_comparison.png",
        "title": "Page Replacement Algorithm Comparison",
        "x_label": "Page replacement policy",
        "layout": (1, 2),
        "metrics": [
            ("hits", "Page Hits", "Page count", "%.0f"),
            ("faults", "Page Faults", "Page count", "%.0f"),
        ],
    },
    "memory_allocation": {
        "csv": "memory_allocation.csv",
        "output": "memory_allocation_comparison.png",
        "title": "Memory Allocation Algorithm Comparison",
        "x_label": "Allocation policy",
        "layout": (2, 2),
        "metrics": [
            ("allocated_requests", "Allocated Requests", "Requests", "%.0f"),
            ("failed_requests", "Failed Requests", "Requests", "%.0f"),
            ("internal_fragmentation", "Internal Fragmentation", "Bytes",
             "%.0f"),
            ("largest_free_block", "Largest Free Block", "Bytes", "%.0f"),
        ],
    },
    "disk_scheduling": {
        "csv": "disk_scheduling.csv",
        "output": "disk_scheduling_comparison.png",
        "title": "Disk Scheduling Algorithm Comparison",
        "x_label": "Disk scheduling policy",
        "layout": (1, 1),
        "metrics": [
            ("total_head_movement", "Total Head Movement", "Cylinders",
             "%.0f"),
        ],
    },
    "file_allocation": {
        "csv": "file_allocation.csv",
        "output": "file_allocation_comparison.png",
        "title": "File Allocation Method Comparison",
        "x_label": "Allocation method",
        "layout": (2, 2),
        "metrics": [
            ("used_blocks", "Used Blocks", "Blocks", "%.0f"),
            ("allocations_failed", "Allocation Failures", "Operations",
             "%.0f"),
            ("wasted_bytes", "Wasted Space", "Bytes", "%.0f"),
            ("metadata_blocks", "Metadata Blocks", "Blocks", "%.0f"),
        ],
    },
}


def fail(message):
    print(f"Error: {message}")
    return 1


def load_report(module, spec):
    csv_file = RESULTS_DIR / spec["csv"]

    if not csv_file.exists():
        return None, None, fail(f"Results CSV not found:\n{csv_file}")

    try:
        with csv_file.open("r", newline="", encoding="utf-8") as file:
            reader = csv.DictReader(file)
            rows = list(reader)
            fieldnames = reader.fieldnames
    except OSError as error:
        return None, None, fail(f"Could not read {csv_file}:\n{error}")

    required_columns = {"algorithm"}
    required_columns.update(key for key, _, _, _ in spec["metrics"])

    if not fieldnames or not required_columns.issubset(fieldnames):
        return None, None, fail(
            f"{spec['csv']} is missing required {module} columns."
        )

    if not rows:
        return None, None, fail(
            f"{spec['csv']} contains no {module} results."
        )

    algorithms = []

    for row_number, row in enumerate(rows, start=2):
        algorithm = (row.get("algorithm") or "").strip()

        if not algorithm:
            return None, None, fail(
                f"{spec['csv']} row {row_number} has no algorithm name."
            )

        algorithms.append(algorithm)

        for key, _, _, _ in spec["metrics"]:
            try:
                value = float(row[key])
            except (ValueError, TypeError, KeyError):
                return None, None, fail(
                    f"{spec['csv']} row {row_number} has an invalid "
                    f"'{key}' value."
                )

            if not math.isfinite(value):
                return None, None, fail(
                    f"{spec['csv']} row {row_number} has a non-finite "
                    f"'{key}' value."
                )

    values = {
        key: [float(row[key]) for row in rows]
        for key, _, _, _ in spec["metrics"]
    }
    return (algorithms, values), csv_file, 0


def draw_report(module, spec, algorithms, values, csv_file):
    rows, columns = spec["layout"]
    figsize = spec.get("figsize", (5.5 * columns, 4.2 * rows))
    output_file = RESULTS_DIR / spec["output"]

    fig, axes = plt.subplots(rows, columns, figsize=figsize)
    axes = axes.flatten() if hasattr(axes, "flatten") else [axes]

    for ax, (key, title, y_label, value_format) in zip(
        axes, spec["metrics"]
    ):
        bars = ax.bar(algorithms, values[key])

        ax.set_title(title)
        ax.set_ylabel(y_label)
        ax.set_xlabel(spec["x_label"])
        ax.tick_params(axis="x", labelrotation=25)
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.set_axisbelow(True)

        ax.bar_label(bars, fmt=value_format, padding=3)
        ax.margins(y=0.15)

    for ax in axes[len(spec["metrics"]):]:
        ax.set_visible(False)

    fig.suptitle(spec["title"], fontsize=15)
    fig.tight_layout()

    try:
        output_file.parent.mkdir(parents=True, exist_ok=True)
        fig.savefig(output_file, dpi=160, bbox_inches="tight")
    except OSError as error:
        return fail(f"Could not write {output_file}:\n{error}")
    finally:
        plt.close(fig)

    print("\nGraph generated successfully!")
    print(f"Module: {module}")
    print(f"Input CSV: {csv_file}")
    print(f"Output PNG: {output_file}")
    return 0


def main(argv):
    if len(argv) > 2:
        return fail("Usage: plot_results.py [module]")

    module = argv[1] if len(argv) == 2 else "cpu_scheduling"

    if module not in PLOT_SPECS:
        supported = ", ".join(sorted(PLOT_SPECS))
        return fail(f"Unknown module '{module}'. Supported: {supported}")

    spec = PLOT_SPECS[module]
    report, csv_file, status = load_report(module, spec)

    if report is None:
        return status

    algorithms, values = report
    return draw_report(module, spec, algorithms, values, csv_file)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
