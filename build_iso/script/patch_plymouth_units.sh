#!/bin/bash
# Prevent Plymouth from re-enabling systemd console status output.
#
# Plymouth normally sends SIGRTMIN+20 / SIGRTMIN+21 to PID 1 when its splash
# attaches to/detaches from the session. Those signals explicitly toggle the
# visible "[ OK ] ..." console status output, overriding systemd.show_status=0
# and ShowStatus=no.
#
# We keep --attach-to-session (so VT hand-over still works correctly), but load
# a tiny LD_PRELOAD interposer into plymouthd that drops exactly those two
# signals addressed to PID 1. The same interposer is copied into the initramfs.
#
# Usage: patch_plymouth_units.sh <rootfs>
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOTFS="${1:?usage: patch_plymouth_units.sh <rootfs>}"
UNIT_DIR="${ROOTFS}/etc/systemd/system"
SHIM_DIR="${ROOTFS}/usr/local/lib"
SHIM="${SHIM_DIR}/libplymouth-no-status.so"

mkdir -p "${SHIM_DIR}"
cc -shared -fPIC -O2 -o "${SHIM}" "${SCRIPT_DIR}/plymouth-no-status.c" -ldl
chmod 755 "${SHIM}"

write_drop_in() {
    unit="$1"

    mkdir -p "${UNIT_DIR}/${unit}.service.d"
    cat > "${UNIT_DIR}/${unit}.service.d/cutefish-no-status.conf" <<EOF
[Service]
# Keep boot/shutdown progress in the journal. Plymouth keeps its normal
# --attach-to-session behaviour, but the LD_PRELOAD interposer stops it from
# sending SIGRTMIN+20/21 to PID 1 and re-enabling console status output.
Environment=LD_PRELOAD=/usr/local/lib/libplymouth-no-status.so
EOF
}

write_drop_in plymouth-start
write_drop_in plymouth-reboot
write_drop_in plymouth-poweroff
write_drop_in plymouth-halt
write_drop_in plymouth-kexec

# initramfs: keep plymouthd --attach-to-session, but preload the interposer.
INITRAMFS_PLYMOUTH="${ROOTFS}/usr/share/initramfs-tools/scripts/init-premount/plymouth"
if [ -f "${INITRAMFS_PLYMOUTH}" ]; then
    sed -i 's#^\([[:space:]]*\)/usr/sbin/plymouthd --mode=boot#\1LD_PRELOAD=/usr/local/lib/libplymouth-no-status.so /usr/sbin/plymouthd --mode=boot#' "${INITRAMFS_PLYMOUTH}"
    echo "patch_plymouth_units: patched initramfs plymouth launcher"
fi

# Make update-initramfs copy the interposer into future initramfs images.
mkdir -p "${ROOTFS}/etc/initramfs-tools/hooks"
cp -f "${SCRIPT_DIR}/plymouth-no-status.hook" "${ROOTFS}/etc/initramfs-tools/hooks/cutefish-plymouth-no-status"
chmod 755 "${ROOTFS}/etc/initramfs-tools/hooks/cutefish-plymouth-no-status"

echo "patch_plymouth_units: installed LD_PRELOAD console-quiet integration"
