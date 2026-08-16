#!/bin/bash
# Keep initramfs filesystem-check output in /run/initramfs/fsck.log instead of
# printing it on the console.
#
# initramfs-tools' _checkfs_once() runs:
#   logsave -a -s "$FSCK_LOGFILE" fsck ... "$DEV"
# logsave always writes a copy of the command output to stdout, which is the
# visible tty. That is the source of "/dev/sda2: clean, ..." before the splash
# takes over. The file log is still written; only the stdout copy is dropped.
#
# Usage: patch_initramfs_quiet.sh <rootfs>
set -e

ROOTFS="${1:?usage: patch_initramfs_quiet.sh <rootfs>}"
FUNCTIONS="${ROOTFS}/usr/share/initramfs-tools/scripts/functions"

if [ ! -f "${FUNCTIONS}" ]; then
    echo "patch_initramfs_quiet: ${FUNCTIONS} not found, skipping" >&2
    exit 0
fi

if grep -q 'logsave -a -s $FSCK_LOGFILE fsck $spinner $force $fix -T -t "$TYPE" "$DEV"' "${FUNCTIONS}"; then
    sed -i 's#\(logsave -a -s \$FSCK_LOGFILE fsck \$spinner \$force \$fix -T -t "\$TYPE" "\$DEV"\)#\1 >/dev/null#' "${FUNCTIONS}"
    echo "patch_initramfs_quiet: fsck console output redirected to log"
else
    echo "patch_initramfs_quiet: already patched or layout changed" >&2
fi
