#!/bin/bash
# Read-only manifest check. It never modifies script/, build_iso/ or doc/future/.
# It fails if a new product manifest names a package from the removal list.
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
target="$SCRIPT_DIR/target-packages.list"
live="$SCRIPT_DIR/live-packages.list"
removed="$SCRIPT_DIR/removed-from-product.list"

fail=0
while IFS= read -r name; do
    case "$name" in
        ''|\#*) continue ;;
    esac
    if grep -qxF "$name" "$removed"; then
        echo "ERROR: forbidden package in new manifest: $name" >&2
        fail=1
    fi
done < <(cat "$target" "$live" | sed -e 's/#.*//' -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' -e '/^$/d')

if [ "$fail" -eq 0 ]; then
    echo "PASS: new target/live manifests contain no removed product runtime packages"
fi
exit "$fail"
