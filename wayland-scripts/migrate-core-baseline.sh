#!/bin/bash
# Stage-0 migration copy helper for cutefish/core.
#
# This script never modifies the frozen cutefish/code/ tree. It copies an
# explicit, auditable allow-list of Wayland-clean components into
# cutefish/wayland-code/core and records the source commit.
#
# Components that have been rewritten as Wayland-native migration copies
# (session, settings-daemon, powerman, notificationd) are validated in place
# and are NOT overwritten by a blind copy. Legacy display-server components
# are intentionally absent from the new build manifest.

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
    echo "migrated component copy: $component"
done

authored_wayland_components=(
    session
    settings-daemon
    powerman
    notificationd
)

for component in "${authored_wayland_components[@]}"; do
    if [ ! -d "$NEW/$component" ]; then
        echo "ERROR: authored Wayland migration missing: $component" >&2
        exit 1
    fi
    echo "validated authored Wayland migration: $component"
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
echo "Legacy display-server components are not copied and are not in the new build manifest."
