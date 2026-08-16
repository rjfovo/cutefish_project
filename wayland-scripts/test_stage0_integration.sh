#!/bin/bash
# Stage-0 userspace integration test harness. Safe: VirtualBackend and
# offscreen/minimal QPA only. No DRM/input devices, no privileged services,
# no disk jobs. Temporary files are created under /tmp and removed.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${CUTEFISH_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
OUT="$ROOT/cutefish/output"
BUILD="$OUT/build"

core_bin=""
shell_bin=""
installer_bin=""
smoke_bin=""

for candidate in \
    "$BUILD/cutefish-compositor-core/cmake/cutefish-compositor-core" \
    "$BUILD/cutefish-compositor-core/source/obj-x86_64-linux-gnu/cutefish-compositor-core"; do
    [ -x "$candidate" ] && core_bin="$candidate" && break
done
for candidate in \
    "$BUILD/cutefish-shell/cmake/cutefish-shell" \
    "$BUILD/cutefish-shell/source/obj-x86_64-linux-gnu/cutefish-shell"; do
    [ -x "$candidate" ] && shell_bin="$candidate" && break
done
for candidate in \
    "$BUILD/cutefish-installer/cmake/cutefish-installer" \
    "$BUILD/cutefish-installer/source/obj-x86_64-linux-gnu/cutefish-installer"; do
    [ -x "$candidate" ] && installer_bin="$candidate" && break
done
for candidate in \
    "$BUILD/cutefish-compositor-core/cmake/wayland_protocol_smoke" \
    "$BUILD/cutefish-compositor-core/source/obj-x86_64-linux-gnu/wayland_protocol_smoke"; do
    [ -x "$candidate" ] && smoke_bin="$candidate" && break
done

[ -n "$core_bin" ] || { echo "ERROR: cutefish-compositor-core binary not built" >&2; exit 1; }
[ -n "$shell_bin" ] || { echo "ERROR: cutefish-shell binary not built" >&2; exit 1; }
[ -n "$installer_bin" ] || { echo "ERROR: cutefish-installer binary not built" >&2; exit 1; }

runtime="$(mktemp -d /tmp/cutefish-stage0-test-XXXXXX)"
chmod 700 "$runtime"
core_pid=""

cleanup() {
    if [ -n "$core_pid" ]; then
        kill -TERM "$core_pid" 2>/dev/null || true
        wait "$core_pid" 2>/dev/null || true
    fi
    rm -rf "$runtime"
}
trap cleanup EXIT

prefix="stage0-$PPID"
"$core_bin" --virtual --runtime-dir "$runtime" --socket-prefix "$prefix" >"$runtime/core.log" 2>&1 &
core_pid=$!

for _ in $(seq 1 150); do
    [ -S "$runtime/$prefix-apps" ] && [ -S "$runtime/$prefix-shell" ] && break
    sleep 0.02
done
[ -S "$runtime/$prefix-apps" ] || { echo "ERROR: apps socket not ready" >&2; exit 1; }
[ -S "$runtime/$prefix-shell" ] || { echo "ERROR: shell socket not ready" >&2; exit 1; }

# 1. Custom minimal libwayland client: dual-socket isolation and core protocol.
if [ -n "$smoke_bin" ]; then
    echo "== protocol smoke"
    XDG_RUNTIME_DIR="$runtime" "$smoke_bin" "$core_bin" >/tmp/cutefish-stage0-protocol.log 2>&1 || {
        cat /tmp/cutefish-stage0-protocol.log >&2
        exit 1
    }
fi

# 2. Real Qt Quick shell as a standard Wayland client on the apps socket.
echo "== Qt shell Wayland connection smoke"
XDG_RUNTIME_DIR="$runtime" \
WAYLAND_DISPLAY="$prefix-apps" \
QT_QPA_PLATFORM=wayland \
QT_LOGGING_RULES='qt.qpa.wayland*=true' \
"$shell_bin" --boot --connection-test >/tmp/cutefish-stage0-shell.log 2>&1 || {
    cat /tmp/cutefish-stage0-shell.log >&2
    exit 1
}

# 3. Installer fixed-flow UI smoke on offscreen.
echo "== installer offscreen smoke"
QT_QPA_PLATFORM=offscreen QT_QUICK_BACKEND=software \
    "$installer_bin" --self-test >/tmp/cutefish-stage0-installer.log 2>&1 || {
    cat /tmp/cutefish-stage0-installer.log >&2
    exit 1
}

# 4. KMS guard: on machines without DRM the backend must fail safely; on
# machines with DRM it must never acquire master unless explicitly authorized.
echo "== KMS guard"
set +e
"$core_bin" --kms >/tmp/cutefish-stage0-kms.log 2>&1
kms_rc=$?
set -e
if [ "$kms_rc" -eq 0 ]; then
    cat /tmp/cutefish-stage0-kms.log >&2
    echo "ERROR: KmsBackend unexpectedly succeeded in unauthorized mode" >&2
    exit 1
fi
cat /tmp/cutefish-stage0-kms.log
# 1 = DRM device unavailable; 2 = modeset path refused.
case "$kms_rc" in
    1|2) ;;
    *) echo "ERROR: unexpected KMS guard rc=$kms_rc" >&2; exit 1 ;;
esac

echo "PASS: stage-0 VirtualBackend dual-socket, Qt shell connection, installer UI and KMS guard checks"
