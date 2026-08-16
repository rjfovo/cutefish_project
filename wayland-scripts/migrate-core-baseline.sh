#!/bin/bash
# Stage-0 migration copy helper for cutefish/core.
#
# This script never modifies the frozen cutefish/code/ tree. It copies an
# explicit, auditable allow-list of Wayland-clean components into
# cutefish/wayland-code/core and records the source commit. X11/KWin/SDDM
# components (cupdatecursor, xembed-sni-proxy, sddm-helper, chotkeys,
# gmenuproxy, session X11/KWin paths, powerman X DPMS, settings-daemon
# X11 backends) are intentionally NOT copied.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${CUTEFISH_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
OLD="$ROOT/cutefish/code/core"
NEW="$ROOT/cutefish/wayland-code/core"

if [ ! -d "$OLD" ]; then
    echo "ERROR: audit source not found: $OLD" >&2
    exit 1
fi

mkdir -p "$NEW"

# Explicit component allow-list. Each component is copied as a migration copy;
# the new top-level CMake and debian packaging are authored separately below.
components=(
    cpufreq
    screen-brightness
    shutdown-ui
    polkit-agent
    clipboard
)

for component in "${components[@]}"; do
    rm -rf "$NEW/$component"
    mkdir -p "$NEW/$component"
    cp -a "$OLD/$component/." "$NEW/$component/"
    echo "migrated component: $component"
done

for misc in LICENSE README.md cutefish; do
    if [ -e "$OLD/$misc" ]; then
        cp -a "$OLD/$misc" "$NEW/$misc"
        echo "migrated file: $misc"
    fi
done

if git -C "$ROOT" rev-parse --verify HEAD >/dev/null 2>&1; then
    printf 'source=%s\nsource_commit=%s\nmigration_tool=%s\n' \
        "$OLD" "$(git -C "$ROOT" rev-parse --verify HEAD)" \
        "wayland-scripts/migrate-core-baseline.sh" > "$NEW/.migration-source"
    echo "recorded migration source commit"
fi

echo "Core baseline migration copy complete."
echo "Forbidden components were not copied and are not in the new build manifest."
