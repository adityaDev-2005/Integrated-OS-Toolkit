#!/usr/bin/env bash
#
# Regression suite for the implemented modules.
#
# Scheduling cases (every fixture not prefixed with paging_, memory_ or posix_)
# run from a temporary working directory and compare the generated CSV with a
# fixture. Page replacement cases compare the reported hit and fault counts.
# Memory allocation cases compare the full module report. POSIX cases use a
# marker checklist plus PID, temporary-file and working-directory checks,
# because PIDs, temporary paths and directory entries are inherently dynamic.
# IPC cases also use a marker checklist, because thread scheduling decides how
# work is split between producers and consumers, so only the summary lines are
# stable. Deadlock avoidance cases use the same checklist approach and compare
# the reported matrices, safety verdict and safe sequence. Disk scheduling cases
# compare the whole report block, because the three policies and their totals
# are fully deterministic. File allocation cases also compare the whole report
# block; the invalid-input case checks every rejection marker instead.
# Plot generation and image opening are disabled so the suite needs neither
# Python nor a desktop session.  The reporting checks rerun representative
# workloads under an isolated OS_TOOLKIT_ROOT and compare the generated CSV
# with a fixture, assert the save and plot-skip messages, and confirm that
# every comparison module reports through the shared writer.  A menu-dispatch
# check chains choices 1-10 through one process.  Two deliberate failure
# scenarios prove that a blocked CSV destination and an unusable plotting
# script are reported without discarding the algorithm results.

set -u
shopt -s nullglob

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BINARY="$ROOT/build/os_toolkit"
CASES="$ROOT/tests/cases"
CSV="$ROOT/data/results/cpu_scheduling.csv"
WORK_DIR="$(mktemp -d)"

PASS=0
FAIL=0

cleanup() {
    rm -rf "$WORK_DIR"
}
trap cleanup EXIT

if [ ! -x "$BINARY" ]; then
    echo "error: $BINARY not found; run 'make' first." >&2
    exit 1
fi

# Compare with carriage returns stripped, so CRLF checkouts still match.
same_text() {
    diff -q <(tr -d '\r' < "$1") <(tr -d '\r' < "$2") > /dev/null
}

record() {
    if [ "$1" -eq 0 ]; then
        echo "PASS $2"
        PASS=$((PASS + 1))
    else
        echo "FAIL $2"
        FAIL=$((FAIL + 1))
    fi
}

run_scheduling_case() {
    local input="$1"
    local expected="$2"
    local label="$3"
    local status=0

    rm -f "$CSV"

    ( cd "$WORK_DIR" &&
      OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
          "$BINARY" < "$input" > /dev/null )

    if [ ! -f "$CSV" ]; then
        echo "FAIL $label: no CSV at $CSV"
        FAIL=$((FAIL + 1))
        return
    fi

    if ! same_text "$expected" "$CSV"; then
        echo "FAIL $label: CSV mismatch"
        diff -u "$expected" "$CSV" | sed 's/^/    /'
        status=1
    fi

    if [ -e "$WORK_DIR/data" ]; then
        echo "FAIL $label: output written into the current directory"
        rm -rf "$WORK_DIR/data"
        status=1
    fi

    record "$status" "$label"
}

run_paging_case() {
    local input="$1"
    local expected="$2"
    local label="$3"
    local output="$WORK_DIR/$label.out"
    local summary="$WORK_DIR/$label.summary"
    local status=0

    ( cd "$WORK_DIR" &&
      OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
          "$BINARY" < "$input" > "$output" )

    grep -E '^(FIFO|LRU|LFU) +[0-9]+ +[0-9]+\r?$' "$output" > "$summary" || true

    if ! same_text "$expected" "$summary"; then
        echo "FAIL $label: hits/faults mismatch"
        diff -u "$expected" "$summary" | sed 's/^/    /'
        status=1
    fi

    record "$status" "$label"
}

run_memory_case() {
    local input="$1"
    local expected="$2"
    local label="$3"
    local output="$WORK_DIR/$label.out"
    local report="$WORK_DIR/$label.report"
    local status=0

    ( cd "$WORK_DIR" &&
      OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
          "$BINARY" < "$input" > "$output" )

    awk '
        /^========== MEMORY ALLOCATION ==========$/ { capture = 1 }
        /^========== MAIN MENU ==========$/ { capture = 0 }
        capture
    ' "$output" | sed \
        -e '/^Comparison data saved to data\/results\//d' \
        -e '/^Plot generation skipped (OS_TOOLKIT_NO_PLOT is set)\.$/d' \
        -e '/^[[:space:]]*$/d' > "$report"

    if ! same_text "$expected" "$report"; then
        echo "FAIL $label: allocation report mismatch"
        diff -u "$expected" "$report" | sed 's/^/    /'
        status=1
    fi

    record "$status" "$label"
}

run_disk_case() {
    local input="$1"
    local expected="$2"
    local label="$3"
    local output="$WORK_DIR/$label.out"
    local report="$WORK_DIR/$label.report"
    local status=0

    ( cd "$WORK_DIR" &&
      OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
          "$BINARY" < "$input" > "$output" 2>&1 )

    if [ $? -ne 0 ]; then
        echo "FAIL $label: non-zero exit status"
        status=1
    fi

    awk '
        /^========== DISK SCHEDULING ==========$/ { capture = 1 }
        /^========== MAIN MENU ==========$/ { capture = 0 }
        capture
    ' "$output" | sed \
        -e '/^Comparison data saved to data\/results\//d' \
        -e '/^Plot generation skipped (OS_TOOLKIT_NO_PLOT is set)\.$/d' \
        -e '/^[[:space:]]*$/d' > "$report"

    if ! same_text "$expected" "$report"; then
        echo "FAIL $label: disk scheduling report mismatch"
        diff -u "$expected" "$report" | sed 's/^/    /'
        status=1
    fi

    record "$status" "$label"
}

run_file_case() {
    local input="$1"
    local expected="$2"
    local label="$3"
    local output="$WORK_DIR/$label.out"
    local report="$WORK_DIR/$label.report"
    local status=0

    ( cd "$WORK_DIR" &&
      OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
          "$BINARY" < "$input" > "$output" 2>&1 )

    if [ $? -ne 0 ]; then
        echo "FAIL $label: non-zero exit status"
        status=1
    fi

    awk '
        /^========== FILE ALLOCATION ==========$/ { capture = 1 }
        /^========== MAIN MENU ==========$/ { capture = 0 }
        capture
    ' "$output" | sed \
        -e '/^Comparison data saved to data\/results\//d' \
        -e '/^Plot generation skipped (OS_TOOLKIT_NO_PLOT is set)\.$/d' \
        -e '/^[[:space:]]*$/d' > "$report"

    if ! same_text "$expected" "$report"; then
        echo "FAIL $label: file allocation report mismatch"
        diff -u "$expected" "$report" | sed 's/^/    /'
        status=1
    fi

    record "$status" "$label"
}

run_report_case() {
    local input="$1"
    local expected="$2"
    local filename="$3"
    local label="$4"
    local alt
    local output="$WORK_DIR/$label.out"
    local status=0

    alt="$(mktemp -d "$WORK_DIR/report.XXXXXX")"

    if ! ( cd "$WORK_DIR" &&
           OS_TOOLKIT_ROOT="$alt" OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
               "$BINARY" < "$input" > "$output" 2>&1 ); then
        echo "FAIL $label: non-zero exit status"
        status=1
    fi

    if [ ! -f "$alt/data/results/$filename" ]; then
        echo "FAIL $label: missing data/results/$filename"
        status=1
    elif ! same_text "$expected" "$alt/data/results/$filename"; then
        echo "FAIL $label: CSV mismatch"
        diff -u "$expected" "$alt/data/results/$filename" | sed 's/^/    /'
        status=1
    fi

    if ! grep -Fq -- "Comparison data saved to data/results/$filename" \
            "$output"; then
        echo "FAIL $label: CSV save message missing"
        status=1
    fi

    if ! grep -Fq -- "Plot generation skipped (OS_TOOLKIT_NO_PLOT is set)." \
            "$output"; then
        echo "FAIL $label: plot-skip message missing"
        status=1
    fi

    if [ -e "$WORK_DIR/data" ]; then
        echo "FAIL $label: output written into the current directory"
        rm -rf "$WORK_DIR/data"
        status=1
    fi

    record "$status" "$label"
}

run_menu_dispatch() {
    local alt
    local input_fixture
    local input="$WORK_DIR/menu_dispatch.in"
    local output="$WORK_DIR/menu_dispatch.out"
    local saved
    local marker
    local status=0

    alt="$(mktemp -d "$WORK_DIR/menu.XXXXXX")"

    # Each fixture starts with its menu choice and ends with 10; drop the
    # exit line so one process can visit every module in turn.
    {
        for input_fixture in \
            "$CASES/01_documented.in" \
            "$CASES/paging_01_classic.in" \
            "$CASES/memory_01_classic.in" \
            "$CASES/posix_01_demo.in" \
            "$CASES/ipc_01_producer_consumer.in" \
            "$CASES/banker_01_safe_state.in" \
            "$CASES/disk_01_classic_both_sides.in" \
            "$CASES/file_01_mixed.in"; do
            sed '$d' "$input_fixture" | tr -d '\r'
        done
        printf '9\n11\n10\n'
    } > "$input"

    if ! ( cd "$WORK_DIR" &&
           OS_TOOLKIT_ROOT="$alt" OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
               "$BINARY" < "$input" > "$output" 2>&1 ); then
        echo "FAIL menu-dispatch: non-zero exit status"
        status=1
    fi

    for marker in \
        "========== ALGORITHM COMPARISON ==========" \
        "========== PAGE REPLACEMENT ==========" \
        "========== MEMORY ALLOCATION ==========" \
        "========== POSIX PROCESS AND FILE DEMO ==========" \
        "========== IPC AND SYNCHRONIZATION ==========" \
        "========== DEADLOCK AVOIDANCE ==========" \
        "========== DISK SCHEDULING ==========" \
        "========== FILE ALLOCATION ==========" \
        "ABOUT THE PROJECT" \
        "Invalid choice. Select 1 to 10." \
        "Exiting Integrated OS Toolkit."; do
        if ! grep -Fq -- "$marker" "$output"; then
            echo "FAIL menu-dispatch: missing marker: $marker"
            status=1
        fi
    done

    saved="$(grep -c '^Comparison data saved to data/results/' "$output")"

    if [ "$saved" -ne 5 ]; then
        echo "FAIL menu-dispatch: expected 5 CSV save messages, saw $saved"
        status=1
    fi

    record "$status" "menu-dispatch"
}

run_csv_failure_case() {
    local input="$1"
    local label="$2"
    local marker="$3"
    local alt
    local output="$WORK_DIR/$label.out"
    local status=0

    alt="$(mktemp -d "$WORK_DIR/csvfail.XXXXXX")"

    # A regular file where the data directory must go forces every CSV
    # operation to fail.
    : > "$alt/data"

    if ! ( cd "$WORK_DIR" &&
           OS_TOOLKIT_ROOT="$alt" OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
               "$BINARY" < "$input" > "$output" 2>&1 ); then
        echo "FAIL $label: non-zero exit status"
        status=1
    fi

    if ! grep -Fq -- \
            "CSV export failed; the algorithm results above are still valid." \
            "$output"; then
        echo "FAIL $label: CSV failure message missing"
        status=1
    fi

    if grep -Fq -- "Comparison data saved to data/results/" "$output"; then
        echo "FAIL $label: reported a saved CSV after a write failure"
        status=1
    fi

    if grep -Fq -- "Generating " "$output"; then
        echo "FAIL $label: attempted a plot after a failed CSV export"
        status=1
    fi

    if ! grep -Fq -- "$marker" "$output"; then
        echo "FAIL $label: algorithm results missing"
        status=1
    fi

    record "$status" "$label"
}

run_plot_failure_case() {
    local input="$1"
    local filename="$2"
    local marker="$3"
    local label="$4"
    local alt
    local output="$WORK_DIR/$label.out"
    local status=0

    alt="$(mktemp -d "$WORK_DIR/plotfail.XXXXXX")"

    # The isolated root has no scripts/plot_results.py, so plot generation
    # must fail after the CSV has been written.
    if ! ( cd "$WORK_DIR" &&
           OS_TOOLKIT_ROOT="$alt" OS_TOOLKIT_NO_OPEN=1 \
               "$BINARY" < "$input" > "$output" 2>&1 ); then
        echo "FAIL $label: non-zero exit status"
        status=1
    fi

    if [ ! -s "$alt/data/results/$filename" ]; then
        echo "FAIL $label: CSV was not kept after a plot failure"
        status=1
    fi

    if ! grep -Fq -- "Comparison data saved to data/results/$filename" \
            "$output"; then
        echo "FAIL $label: CSV save message missing"
        status=1
    fi

    if ! grep -Fq -- "Could not generate the graph." "$output"; then
        echo "FAIL $label: plot failure message missing"
        status=1
    fi

    if ! grep -Fq -- \
            "The CSV file and the algorithm results are still valid." \
            "$output"; then
        echo "FAIL $label: explanation after plot failure missing"
        status=1
    fi

    if grep -Fq -- "Graph saved to data/results/" "$output"; then
        echo "FAIL $label: reported a saved graph that was never produced"
        status=1
    fi

    if ! grep -Fq -- "$marker" "$output"; then
        echo "FAIL $label: algorithm results missing"
        status=1
    fi

    record "$status" "$label"
}

run_posix_case() {
    local input="$1"
    local expected="$2"
    local label="$3"
    local output="$WORK_DIR/$label.out"
    local cwd="$WORK_DIR/$label.cwd"
    local marker
    local parent_pid
    local child_ppid
    local child_ppids
    local child_count
    local temp_path
    local status=0

    mkdir -p "$cwd"

    ( cd "$cwd" &&
      OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
          "$BINARY" < "$input" > "$output" 2>&1 )

    if [ $? -ne 0 ]; then
        echo "FAIL $label: non-zero exit status"
        status=1
    fi

    while IFS= read -r marker; do
        marker="${marker%$'\r'}"
        [ -z "$marker" ] && continue

        if ! grep -Fq -- "$marker" "$output"; then
            echo "FAIL $label: missing marker: $marker"
            status=1
        fi
    done < "$expected"

    parent_pid="$(sed -n 's/^\[process\] parent: pid=\([0-9][0-9]*\).*/\1/p' \
                  "$output" | head -n 1)"
    child_ppids="$(sed -n \
        -e 's/^\[process\] waitpid child: pid=[0-9][0-9]* ppid=\([0-9][0-9]*\).*/\1/p' \
        -e 's/^\[process\] wait child: pid=[0-9][0-9]* ppid=\([0-9][0-9]*\).*/\1/p' \
        -e 's/^\[exec\] child before exec: pid=[0-9][0-9]* ppid=\([0-9][0-9]*\).*/\1/p' \
        -e 's/^\[exec\] child before failed exec: pid=[0-9][0-9]* ppid=\([0-9][0-9]*\).*/\1/p' \
        "$output")"
    child_count="$(printf '%s\n' "$child_ppids" | grep -c '^[0-9][0-9]*$')"

    if [ -z "$parent_pid" ] || [ "$child_count" -ne 4 ]; then
        echo "FAIL $label: expected four child PID records"
        status=1
    fi

    while IFS= read -r child_ppid; do
        [ -z "$child_ppid" ] && continue

        if [ "$child_ppid" != "$parent_pid" ]; then
            echo "FAIL $label: child PPID $child_ppid does not match parent PID $parent_pid"
            status=1
        fi
    done <<< "$child_ppids"

    temp_path="$(sed -n 's/^\[file\] temporary file created: //p' \
                 "$output" | head -n 1)"

    if [ -z "$temp_path" ]; then
        echo "FAIL $label: temporary file path not reported"
        status=1
    elif [ -e "$temp_path" ]; then
        echo "FAIL $label: temporary file was not removed: $temp_path"
        status=1
    fi

    if [ -n "$(ls -A "$cwd")" ]; then
        echo "FAIL $label: files written into the working directory"
        ls -la "$cwd" | sed 's/^/    /'
        status=1
    fi

    record "$status" "$label"
}

run_marker_case() {
    local input="$1"
    local expected="$2"
    local label="$3"
    local output="$WORK_DIR/$label.out"
    local marker
    local status=0

    ( cd "$WORK_DIR" &&
      OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
          "$BINARY" < "$input" > "$output" 2>&1 )

    if [ $? -ne 0 ]; then
        echo "FAIL $label: non-zero exit status"
        status=1
    fi

    while IFS= read -r marker; do
        marker="${marker%$'\r'}"
        [ -z "$marker" ] && continue

        if ! grep -Fq -- "$marker" "$output"; then
            echo "FAIL $label: missing marker: $marker"
            status=1
        fi
    done < "$expected"

    record "$status" "$label"
}

for input in "$CASES"/*.in; do
    name="$(basename "$input" .in)"

    case "$name" in
        memory_*)
            expected="$CASES/$name.expected.txt"
            kind="memory"
            ;;
        paging_*)
            expected="$CASES/$name.expected.txt"
            kind="paging"
            ;;
        posix_*)
            expected="$CASES/$name.expected.txt"
            kind="posix"
            ;;
        ipc_*)
            expected="$CASES/$name.expected.txt"
            kind="ipc"
            ;;
        banker_*)
            expected="$CASES/$name.expected.txt"
            kind="banker"
            ;;
        disk_06_*)
            # The invalid-input case is a marker checklist: its purpose is to
            # prove that every rejection message appeared before the final,
            # valid analysis ran.
            expected="$CASES/$name.expected.txt"
            kind="disk_marker"
            ;;
        disk_*)
            expected="$CASES/$name.expected.txt"
            kind="disk"
            ;;
        file_06_*)
            # The invalid-input case is a marker checklist that proves every
            # rejection message appeared before the final valid run.
            expected="$CASES/$name.expected.txt"
            kind="file_marker"
            ;;
        file_*)
            expected="$CASES/$name.expected.txt"
            kind="file"
            ;;
        *)
            expected="$CASES/$name.expected.csv"
            kind="scheduling"
            ;;
    esac

    if [ ! -f "$expected" ]; then
        echo "FAIL $name: missing fixture $expected"
        FAIL=$((FAIL + 1))
        continue
    fi

    case "$kind" in
        memory) run_memory_case "$input" "$expected" "$name" ;;
        paging) run_paging_case "$input" "$expected" "$name" ;;
        posix)  run_posix_case "$input" "$expected" "$name" ;;
        ipc)    run_marker_case "$input" "$expected" "$name" ;;
        banker) run_marker_case "$input" "$expected" "$name" ;;
        disk)   run_disk_case "$input" "$expected" "$name" ;;
        disk_marker) run_marker_case "$input" "$expected" "$name" ;;
        file)   run_file_case "$input" "$expected" "$name" ;;
        file_marker) run_marker_case "$input" "$expected" "$name" ;;
        *)      run_scheduling_case "$input" "$expected" "$name" ;;
    esac
done

# ---------------------------------------------------------------------------
# Benchmark reporting and final integration checks.
# ---------------------------------------------------------------------------

run_report_case "$CASES/01_documented.in" \
    "$CASES/01_documented.expected.csv" "cpu_scheduling.csv" \
    "reporting-cpu"
run_report_case "$CASES/paging_01_classic.in" \
    "$CASES/reporting_page_replacement.expected.csv" "page_replacement.csv" \
    "reporting-paging"
run_report_case "$CASES/memory_01_classic.in" \
    "$CASES/reporting_memory_allocation.expected.csv" "memory_allocation.csv" \
    "reporting-memory"
run_report_case "$CASES/disk_01_classic_both_sides.in" \
    "$CASES/reporting_disk_scheduling.expected.csv" "disk_scheduling.csv" \
    "reporting-disk"
run_report_case "$CASES/file_01_mixed.in" \
    "$CASES/reporting_file_allocation.expected.csv" "file_allocation.csv" \
    "reporting-file"

run_menu_dispatch

run_csv_failure_case "$CASES/01_documented.in" "csv-failure-cpu" \
    "ALGORITHM COMPARISON"
run_csv_failure_case "$CASES/paging_01_classic.in" "csv-failure-paging" \
    "LFU tie-break"

run_plot_failure_case "$CASES/01_documented.in" "cpu_scheduling.csv" \
    "ALGORITHM COMPARISON" "plot-failure-cpu"
run_plot_failure_case "$CASES/paging_01_classic.in" "page_replacement.csv" \
    "LFU tie-break" "plot-failure-paging"

# OS_TOOLKIT_ROOT must redirect every generated file.
alt="$(mktemp -d "$WORK_DIR/alt.XXXXXX")"
status=0

OS_TOOLKIT_ROOT="$alt" OS_TOOLKIT_NO_PLOT=1 OS_TOOLKIT_NO_OPEN=1 \
    "$BINARY" < "$CASES/01_documented.in" > /dev/null

if [ ! -f "$alt/data/results/cpu_scheduling.csv" ]; then
    echo "FAIL root-override: no CSV under OS_TOOLKIT_ROOT"
    status=1
elif ! same_text "$CASES/01_documented.expected.csv" \
                "$alt/data/results/cpu_scheduling.csv"; then
    echo "FAIL root-override: CSV mismatch"
    status=1
fi

record "$status" "root-override"

echo
echo "Passed: $PASS  Failed: $FAIL"

[ "$FAIL" -eq 0 ]
