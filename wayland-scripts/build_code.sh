#!/bin/bash
# CutefishOS Wayland-only build script.
#
# Usage is intentionally identical to the frozen script/build_code.sh:
#   ./build_code.sh 项目名          build one project
#   ./build_code.sh clean 项目名    clean one project
#   ./build_code.sh all             build all projects
#   ./build_code.sh clean           clean all new build products
#
# New rules implemented here:
#  * no hard-coded absolute path; repository root is derived from the script
#    location and may be overridden with CUTEFISH_ROOT
#  * categorized output under cutefish/output/{build,packages,symbols,logs}/<project>
#    plus cutefish/output/cache
#  * source trees are never modified; dpkg-buildpackage runs in a copied source
#    tree under cutefish/output/build/<project>/source
#  * build.cache is a file and build/ is a directory; clean never mistakes them
#  * SOURCE_DATE_EPOCH is exported for reproducible builds

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${CUTEFISH_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
SOURCE_BASE="$ROOT/cutefish/wayland-code"
OUTPUT_ROOT="$ROOT/cutefish/output"
MANIFEST="$SCRIPT_DIR/projects.list"
CACHE_DIR="$OUTPUT_ROOT/cache"
LOG_ROOT="$OUTPUT_ROOT/logs"
PACKAGES_ROOT="$OUTPUT_ROOT/packages"
SYMBOLS_ROOT="$OUTPUT_ROOT/symbols"
BUILD_ROOT="$OUTPUT_ROOT/build"

# shellcheck source=lib-build.sh
source "$SCRIPT_DIR/lib-build.sh"

if [ ! -f "$MANIFEST" ]; then
    echo "ERROR: project manifest not found: $MANIFEST" >&2
    exit 1
fi

project_names() {
    sed -e 's/#.*//' -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' -e 's/:[^:]*$//' -e '/^$/d' "$MANIFEST"
}

project_build_type() {
    local project="$1"
    local type
    type="$(awk -v p="$project" '{
        line=$0; sub(/#.*/, "", line);
        n=split(line, a, ":");
        if (n >= 1 && a[1] == p) {
            if (n >= 2 && length(a[2]) > 0) print a[2];
            else print "auto";
            exit;
        }
    }' "$MANIFEST")"
    if [ -z "$type" ]; then
        echo "ERROR: project not listed in manifest: $project" >&2
        return 1
    fi
    if [ "$type" = "auto" ]; then
        if [ -d "$SOURCE_BASE/$project/debian" ]; then
            echo dpkg
        else
            echo cmake
        fi
    else
        echo "$type"
    fi
}

ensure_output_dirs() {
    mkdir -p "$OUTPUT_ROOT" "$BUILD_ROOT" "$PACKAGES_ROOT" "$SYMBOLS_ROOT" "$LOG_ROOT" "$CACHE_DIR"
    # build.cache is a file and must remain a file.
    [ -e "$OUTPUT_ROOT/build.cache" ] || : > "$OUTPUT_ROOT/build.cache"
}

help() {
    echo "Usage: $0 {project|all|clean [project]|list}"
    echo
    echo "Projects from $MANIFEST:"
    project_names
}

clean_project() {
    local project="$1"
    local build_dir="$BUILD_ROOT/$project"
    local package_dir="$PACKAGES_ROOT/$project"
    local symbol_dir="$SYMBOLS_ROOT/$project"
    local log_dir="$LOG_ROOT/$project"

    echo "Cleaning output for project '$project' (cutefish/output/ only)"
    for p in "$build_dir" "$package_dir" "$symbol_dir" "$log_dir"; do
        if [ -e "$p" ] || [ -L "$p" ]; then
            echo "  remove $p"
            rm -rf -- "$p"
        fi
    done
}

clean_all() {
    echo "Cleaning all new build products under $OUTPUT_ROOT"
    # Only the new categorized directories. Legacy debs/dbgsym/build_log/code are
    # intentionally left untouched. build.cache is a file and must not be
    # removed as if it were the build/ directory.
    for p in "$BUILD_ROOT" "$PACKAGES_ROOT" "$SYMBOLS_ROOT" "$LOG_ROOT"; do
        if [ -e "$p" ] || [ -L "$p" ]; then
            echo "  remove $p"
            rm -rf -- "$p"
        fi
    done
    [ -e "$OUTPUT_ROOT/build.cache" ] || : > "$OUTPUT_ROOT/build.cache"
    [ -d "$CACHE_DIR" ] || mkdir -p "$CACHE_DIR"
    echo "kept file: $OUTPUT_ROOT/build.cache"
}

build_project_cmake() {
    local project="$1"
    local src="$SOURCE_BASE/$project"
    local cmake_build="$BUILD_ROOT/$project/cmake"
    local stage="$BUILD_ROOT/$project/stage"
    local log="$LOG_ROOT/$project/build.log"

    wf_require_dir "$src" "source project"

    echo "========================================================================" | tee "$log"
    echo "PROJECT: $project" | tee -a "$log"
    echo "SOURCE: $src" | tee -a "$log"
    echo "OUTPUT: $cmake_build" | tee -a "$log"
    echo "LOG FILE: $log" | tee -a "$log"
    echo "SOURCE COMMIT: $(wf_source_commit "$src")" | tee -a "$log"
    echo "SOURCE_DATE_EPOCH: $(wf_source_date_epoch "$src")" | tee -a "$log"
    echo "========================================================================" | tee -a "$log"

    mkdir -p "$cmake_build" "$stage"
    local epoch
    epoch="$(wf_source_date_epoch "$src")"

    set +e
    (
        set -e
        export SOURCE_DATE_EPOCH="$epoch"
        export CMAKE_BUILD_PARALLEL_LEVEL="${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
        cmake -S "$src" -B "$cmake_build" \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_INSTALL_PREFIX=/usr
        cmake --build "$cmake_build" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-2}"
        cmake --install "$cmake_build" --prefix "$stage"
        ctest --test-dir "$cmake_build" --output-on-failure
    ) >>"$log" 2>&1
    local rc=$?
    set -e

    echo "BUILD EXIT CODE: $rc" | tee -a "$log"
    return $rc
}

build_project_dpkg() {
    local project="$1"
    local src="$SOURCE_BASE/$project"
    local project_build="$BUILD_ROOT/$project"
    local dpkg_src="$project_build/source"
    local log="$LOG_ROOT/$project/build.log"

    wf_require_dir "$src" "source project"
    wf_require_dir "$src/debian" "debian packaging directory"

    echo "========================================================================" | tee "$log"
    echo "PROJECT: $project" | tee -a "$log"
    echo "SOURCE: $src" | tee -a "$log"
    echo "OUTPUT: $project_build" | tee -a "$log"
    echo "DPKG SOURCE COPY: $dpkg_src" | tee -a "$log"
    echo "LOG FILE: $log" | tee -a "$log"
    echo "BUILD COMMAND: dpkg-buildpackage -b -uc -us (run in source copy)" | tee -a "$log"
    echo "SOURCE COMMIT: $(wf_source_commit "$src")" | tee -a "$log"
    echo "SOURCE_DATE_EPOCH: $(wf_source_date_epoch "$src")" | tee -a "$log"
    echo "========================================================================" | tee -a "$log"

    mkdir -p "$project_build"
    # Always start from a clean copied tree inside output/build/<project>/.
    rm -rf -- "$dpkg_src"
    wf_copy_source "$src" "$dpkg_src" | tee -a "$log"

    local epoch
    epoch="$(wf_source_date_epoch "$src")"

    set +e
    (
        set -e
        cd "$dpkg_src"
        export SOURCE_DATE_EPOCH="$epoch"
        export DEB_BUILD_OPTIONS="${DEB_BUILD_OPTIONS:-parallel=2}"
        dpkg-buildpackage -b -uc -us
    ) >>"$log" 2>&1
    local rc=$?
    set -e

    echo "BUILD EXIT CODE: $rc" | tee -a "$log"
    if [ $rc -ne 0 ]; then
        return $rc
    fi

    wf_move_artifacts "$project_build" "$project" "$OUTPUT_ROOT" | tee -a "$log"
    return 0
}

build_project() {
    local project="$1"
    local type
    type="$(project_build_type "$project")"
    ensure_output_dirs
    mkdir -p "$LOG_ROOT/$project"

    echo "==> Building project '$project' (type=$type)"
    case "$type" in
        cmake) build_project_cmake "$project" ;;
        dpkg) build_project_dpkg "$project" ;;
        *)
            echo "ERROR: unknown build type '$type' for project $project" >&2
            return 1
            ;;
    esac
}

list_projects() {
    while IFS= read -r project; do
        local type
        type="$(project_build_type "$project")"
        printf '%-32s %s\n' "$project" "$type"
    done < <(project_names)
}

case "${1:-}" in
    "")
        help
        exit 0
        ;;
    list)
        list_projects
        exit 0
        ;;
    help|-h|--help)
        help
        exit 0
        ;;
    clean)
        ensure_output_dirs
        if [ $# -ge 2 ]; then
            project_build_type "$2" >/dev/null
            clean_project "$2"
        else
            clean_all
        fi
        exit 0
        ;;
    all)
        ensure_output_dirs
        rc=0
        while IFS= read -r project; do
            if ! build_project "$project"; then
                echo "ERROR: build failed for project $project" >&2
                rc=1
                break
            fi
        done < <(project_names)
        exit $rc
        ;;
    *)
        project="${1:-}"
        project_build_type "$project" >/dev/null
        build_project "$project"
        exit $?
        ;;
esac
