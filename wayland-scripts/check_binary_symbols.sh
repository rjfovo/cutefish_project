#!/bin/bash
# CI helper: inspect newly built Cutefish debs and fail on direct forbidden
# dynamic dependencies or forbidden dynamic symbols. Qt-upstream transitive
# libraries (for example libQt6Gui's own dependencies) are not considered a
# Cutefish self-developed code violation; this script only reads the ELF
# NEEDED entries and undefined/defined dynamic symbols of our binaries.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${CUTEFISH_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
PACKAGE_ROOT="$ROOT/cutefish/output/packages"

forbidden_needed=(
    'libX11'
    'libxcb'
    'libXcursor'
    'libXft'
    'libXext'
    'libXtst'
    'libkwin'
    'libkdecorations'
    'libQt6WaylandCompositor'
    'libwlroots'
)

forbidden_symbols=(
    'XOpenDisplay'
    'XGrabKey'
    'xcb_connect'
    'Xcursor'
    'Xft'
    'KX11Extras'
    'KWindowSystem'
    'wlr_'
    'wl_display_add_socket_auto'
)

fail=0
tmp="$(mktemp -d /tmp/cutefish-binary-scan-XXXXXX)"
trap 'rm -rf "$tmp"' EXIT

if [ ! -d "$PACKAGE_ROOT" ]; then
    echo "No package output tree found: $PACKAGE_ROOT" >&2
    exit 0
fi

while IFS= read -r deb; do
    echo "== $deb"
    rm -rf "$tmp/extract"
    mkdir -p "$tmp/extract"
    dpkg-deb -x "$deb" "$tmp/extract"
    while IFS= read -r file; do
        [ -n "$file" ] || continue
        if ! file "$file" 2>/dev/null | grep -q 'ELF'; then
            continue
        fi
        while IFS= read -r needed; do
            for pattern in "${forbidden_needed[@]}"; do
                if [[ "$needed" == *"$pattern"* ]]; then
                    echo "FAIL: direct forbidden NEEDED in $file: $needed" >&2
                    fail=1
                fi
            done
        done < <(readelf -d "$file" 2>/dev/null | sed -n 's/.*NEEDED.*\[\(.*\)\]/\1/p' || true)
        while IFS= read -r symbol; do
            for pattern in "${forbidden_symbols[@]}"; do
                if [[ "$symbol" == *"$pattern"* ]]; then
                    echo "FAIL: forbidden dynamic symbol in $file: $symbol" >&2
                    fail=1
                fi
            done
        done < <(nm -D --defined-only "$file" 2>/dev/null | awk '{print $NF}' || true)
    done < <(find "$tmp/extract" -type f -perm /111 2>/dev/null)
done < <(find "$PACKAGE_ROOT" -name '*.deb' -type f | sort)

if [ "$fail" -eq 0 ]; then
    echo "PASS: no direct forbidden dynamic dependencies/symbols in new Cutefish packages"
fi
exit "$fail"
