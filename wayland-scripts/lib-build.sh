# Shared helpers for the CutefishOS Wayland-only build system.
# This file must not hard-code absolute paths. Root defaults to the repository
# that contains wayland-scripts/ and can be overridden with CUTEFISH_ROOT.

set -u

wf_log() {
    local line
    line="[$(date '+%Y-%m-%d %H:%M:%S %z')] $*"
    echo "$line" | tee -a "${WF_LOG_FILE:-/dev/null}"
}

wf_run_step() {
    local desc="$1"; shift
    local log="${WF_LOG_FILE:?wf_run_step requires WF_LOG_FILE}"
    {
        echo
        echo "================================================================================"
        echo "STEP: $desc"
        echo "COMMAND: $*"
        echo "START: $(date '+%Y-%m-%d %H:%M:%S %z')"
        echo "--------------------------------------------------------------------------------"
    } | tee -a "$log"

    set +e
    "$@" >>"$log" 2>&1
    local rc=$?
    set -e
    {
        echo "--------------------------------------------------------------------------------"
        echo "STEP RESULT: $desc"
        echo "EXIT CODE: $rc"
        echo "END: $(date '+%Y-%m-%d %H:%M:%S %z')"
    } | tee -a "$log"
    return $rc
}

wf_source_commit() {
    local src="$1"
    if git -C "$src" rev-parse --verify HEAD >/dev/null 2>&1; then
        git -C "$src" rev-parse --verify HEAD
    else
        echo "no-git"
    fi
}

wf_source_date_epoch() {
    local src="$1"
    if [ -n "${CUTEFISH_SOURCE_DATE_EPOCH:-}" ]; then
        echo "$CUTEFISH_SOURCE_DATE_EPOCH"
    elif git -C "$src" show -s --format=%ct HEAD >/dev/null 2>&1; then
        git -C "$src" show -s --format=%ct HEAD
    else
        date -u +%s
    fi
}

wf_copy_source() {
    local src="$1"; local dest="$2"
    mkdir -p "$dest"
    # Copy a clean source tree without .git or build residue. Intermediate files
    # live only under cutefish/output/, never in the source tree.
    tar -C "$src" \
        --exclude=.git \
        --exclude=build \
        --exclude='.cache' \
        --exclude='CMakeFiles' \
        --exclude='CMakeCache.txt' \
        -cf - . | tar -C "$dest" -xf -
    local commit
    commit="$(wf_source_commit "$src")"
    printf '%s\n' "$commit" > "$dest/.cutefish-source-commit"
    printf 'source=%s\nsource_commit=%s\n' "$src" "$commit"
}

wf_move_artifacts() {
    local build_dir="$1"; local project="$2"
    local out_root="$3"
    local deb
    local changes

    mkdir -p "$out_root/packages/$project" "$out_root/symbols/$project" "$out_root/logs/$project"

    # Move .deb files produced by the build. dbgsym packages are classified into
    # symbols/<project>/, regular packages into packages/<project>/.
    while IFS= read -r deb; do
        case "$deb" in
            *-dbgsym_*.deb|*dbgsym_*.deb)
                mv -f "$deb" "$out_root/symbols/$project/" || true ;;
            *.deb)
                mv -f "$deb" "$out_root/packages/$project/" || true ;;
        esac
    done < <(find "$build_dir" -maxdepth 2 -name '*.deb' -type f 2>/dev/null)

    # Build metadata is kept with the build log set.
    while IFS= read -r changes; do
        case "$changes" in
            *.changes|*.buildinfo)
                mv -f "$changes" "$out_root/logs/$project/" || true ;;
        esac
    done < <(find "$build_dir" -maxdepth 2 \( -name '*.changes' -o -name '*.buildinfo' \) -type f 2>/dev/null)
}

wf_require_dir() {
    local p="$1"; local what="$2"
    if [ ! -d "$p" ]; then
        echo "ERROR: $what not found: $p" >&2
        return 1
    fi
}
