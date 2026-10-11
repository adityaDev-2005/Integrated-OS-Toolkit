
import csv
import sys
from pathlib import Path

import matplotlib.pyplot as plt

# Resolve paths relative to the project, not the terminal's directory.
PROJECT_ROOT = Path(__file__).resolve().parent.parent
CSV_FILE = PROJECT_ROOT / "data" / "results" / "cpu_scheduling.csv"
OUTPUT_FILE = (
    PROJECT_ROOT / "data" / "results" / "cpu_scheduling_comparison.png"
)

METRICS = [
    ("avg_waiting_time", "Average Waiting Time"),
    ("avg_turnaround_time", "Average Turnaround Time"),
    ("avg_response_time", "Average Response Time"),
]


def main():
    if not CSV_FILE.exists():
        print(f"Error: Results CSV not found:\n{CSV_FILE}")
        print("Run the CPU Scheduling Analyzer first.")
        return 1

    with CSV_FILE.open("r", newline="", encoding="utf-8") as file:
        reader = csv.DictReader(file)
        rows = list(reader)

    required_columns = {
        "algorithm",
        "avg_waiting_time",
        "avg_turnaround_time",
        "avg_response_time",
    }

    if not reader.fieldnames or not required_columns.issubset(
        reader.fieldnames
    ):
        print("Error: CSV is missing required scheduling columns.")
        return 1

    if not rows:
        print("Error: The CSV contains no scheduling results.")
        return 1

    algorithms = [row["algorithm"] for row in rows]

    try:
        values = {
            key: [float(row[key]) for row in rows]
            for key, _ in METRICS
        }
    except (ValueError, TypeError, KeyError):
        print("Error: The CSV contains invalid metric values.")
        return 1

    # Three panels make each metric easy to compare independently.
    fig, axes = plt.subplots(1, 3, figsize=(15, 5))

    for ax, (key, title) in zip(axes, METRICS):
        bars = ax.bar(algorithms, values[key])

        ax.set_title(title)
        ax.set_ylabel("Time units")
        ax.set_xlabel("Scheduling algorithm")
        ax.tick_params(axis="x", labelrotation=25)
        ax.grid(axis="y", linestyle="--", alpha=0.4)
        ax.set_axisbelow(True)

        ax.bar_label(bars, fmt="%.2f", padding=3)
        ax.margins(y=0.15)

    fig.suptitle(
        "CPU Scheduling Algorithm Comparison",
        fontsize=15,
    )
    fig.tight_layout()

    OUTPUT_FILE.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(OUTPUT_FILE, dpi=160, bbox_inches="tight")
    plt.close(fig)

    print("\nGraph generated successfully!")
    print(f"Input CSV: {CSV_FILE}")
    print(f"Output PNG: {OUTPUT_FILE}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
