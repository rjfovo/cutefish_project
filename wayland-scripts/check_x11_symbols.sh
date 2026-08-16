#!/bin/bash
# Stage-0 CI helper: scan Cutefish self-developed sources/binaries for forbidden
# X11/KWin symbols and strings. The old cutefish/code/ tree is audit-only and is
# not part of the new build target scan (unless CHECK_AUDIT_REFERENCE=1 is set).
#
# Exit code 0 means no forbidden symbols found in the new code tree.

set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${CUTEFISH_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
NEW_CODE="$ROOT/cutefish/wayland-code"
AUDIT_CODE="$ROOT/cutefish/code"

patterns=(
    'XOpenDisplay'
    'XCloseDisplay'
    'XCreateWindow'
    'XGrabKey'
    'XGrabPointer'
    'XGetXCBConnection'
    'xcb_connect'
    'xcb_ewmh'
    'Xcursor'
    'Xft'
    'KX11Extras'
    'KWindowSystem'
    'kwin_wayland'
    'kwin_x11'
    'kwin'
    'Qt6::WaylandCompositor'
    'QtWaylandCompositor'
    'eglfs_kms'
    'wlr-layer-shell'
    'wlr/'
    'libwlroots'
    'XWayland'
    'xwayland'
    '/usr/share/xsessions'
    '\bDISPLAY\b'
    '\bXAUTHORITY\b'
)

scan_file() {
    local file="$1"
    local pattern
    local line_no
    while IFS= read -r line_no || [ -n "$line_no" ]; do
        local text="${line_no#*:}"
        for pattern in "${patterns[@]}"; do
            if grep -qE -- "$pattern" <<<"$text"; then
                echo "$file:$line_no"
                break
            fi
        done
    done < <(grep -n -E "$(IFS='|'; echo "${patterns[*]}")" "$file" 2>/dev/null || true)
}

if [ ! -d "$NEW_CODE" ]; then
    echo "ERROR: new code tree not found: $NEW_CODE" >&2
    exit 1
fi

hits=0
while IFS= read -r file; do
    # Do not flag generated binary build products as source hits here; CI runs
    # this scan on the source tree. Keep binary audit as a separate job.
    case "${file##*/}" in
        *.cpp|*.h|*.c|*.cc|*.qml|*.cmake|CMakeLists.txt|control|rules|*.service|*.desktop|*.list|*.conf|*.sh)
            result="$(scan_file "$file")"
            if [ -n "$result" ]; then
                echo "$result"
                hits=$((hits+1))
            fi
            ;;
    esac
done < <(find "$NEW_CODE" -type f 2>/dev/null | sort)

echo "---"
echo "Forbidden source hits in cutefish/wayland-code: $hits"

if [ "$hits" -eq 0 ]; then
    echo "PASS: no forbidden X11/KWin/QtWaylandCompositor/wlroots symbols in new code."
    exit 0
fi

echo "FAIL: remove or justify every hit above." >&2
exit 1
