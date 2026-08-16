#!/bin/bash
# Report initramfs boot milestones to the persistent Plymouth daemon so the
# boot progress bar is meaningful before systemd takes over.
#
#   show-splash                      10
#   fsck starts                      40
#   fsck finishes                    65
#   root mounted                     85
#   just before switch-root          92
#
# Usage: patch_initramfs_progress.sh <rootfs>
set -e

ROOTFS="${1:?usage: patch_initramfs_progress.sh <rootfs>}"

python3 - "${ROOTFS}" <<'PY'
import sys
from pathlib import Path

root = Path(sys.argv[1])
changed = []

# init-premount/plymouth: after show-splash.
p = root / 'usr/share/initramfs-tools/scripts/init-premount/plymouth'
s = p.read_text() if p.exists() else ''
needle = '/usr/bin/plymouth --show-splash\n'
replacement = '/usr/bin/plymouth --show-splash\n/usr/bin/plymouth system-update --progress=10 || true\n'
if needle in s and 'system-update --progress=10' not in s:
    p.write_text(s.replace(needle, replacement, 1))
    changed.append(str(p))

# functions: fsck start/finish in quiet branch.
p = root / 'usr/share/initramfs-tools/scripts/functions'
s = p.read_text() if p.exists() else ''
if 'logsave -a -s $FSCK_LOGFILE fsck $spinner $force $fix -T -t "$TYPE" "$DEV"' in s:
    if 'system-update --progress=40' not in s:
        s = s.replace('logsave -a -s $FSCK_LOGFILE fsck $spinner $force $fix -T -t "$TYPE" "$DEV" >/dev/null',
                      '/usr/bin/plymouth system-update --progress=40 || true\n\t\tlogsave -a -s $FSCK_LOGFILE fsck $spinner $force $fix -T -t "$TYPE" "$DEV" >/dev/null', 1)
        s = s.replace('logsave -a -s $FSCK_LOGFILE fsck $spinner $force $fix -T -t "$TYPE" "$DEV" >/dev/null\n\t\tFSCKCODE=$?\n\t\tlog_end_msg',
                      'logsave -a -s $FSCK_LOGFILE fsck $spinner $force $fix -T -t "$TYPE" "$DEV" >/dev/null\n\t\tFSCKCODE=$?\n\t\t/usr/bin/plymouth system-update --progress=65 || true\n\t\tlog_end_msg', 1)
        p.write_text(s)
        changed.append(str(p))

# local: after root mount.
p = root / 'usr/share/initramfs-tools/scripts/local'
s = p.read_text() if p.exists() else ''
needle = 'if ! mount ${roflag} ${FSTYPE:+-t "${FSTYPE}"} ${ROOTFLAGS} "${ROOT}" "${rootmnt?}"; then\n\t\tpanic "Failed to mount ${ROOT} as root file system."\n\tfi'
replacement = needle + '\n\t/usr/bin/plymouth system-update --progress=85 || true'
if needle in s and 'system-update --progress=85' not in s:
    s = s.replace(needle, replacement, 1)
    p.write_text(s)
    changed.append(str(p))

# init-bottom/plymouth: before newroot.
p = root / 'usr/share/initramfs-tools/scripts/init-bottom/plymouth'
s = p.read_text() if p.exists() else ''
if '/usr/bin/plymouth --newroot=${rootmnt}' in s and 'system-update --progress=92' not in s:
    s = s.replace('/usr/bin/plymouth --newroot=${rootmnt}',
                  '/usr/bin/plymouth system-update --progress=92 || true\n/usr/bin/plymouth --newroot=${rootmnt}', 1)
    p.write_text(s)
    changed.append(str(p))

print('patch_initramfs_progress: ' + (', '.join(changed) if changed else 'already patched'))
PY
