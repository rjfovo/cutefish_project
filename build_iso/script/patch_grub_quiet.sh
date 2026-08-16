#!/bin/bash
# Patch Debian's /etc/grub.d/10_linux in a chroot/rootfs so grub-mkconfig:
#   1. suppresses "Loading Linux ..." / "Loading initial ramdisk ..." echo lines
#      when the kernel command line contains "quiet";
#   2. enables the existing vt_handoff logic when "splash" is used, so GRUB
#      hands the framebuffer/VT over to Plymouth without a text-mode flash.
#
# Usage: patch_grub_quiet.sh <rootfs>
set -e

ROOTFS="${1:?usage: patch_grub_quiet.sh <rootfs>}"
GRUB_SCRIPT="${ROOTFS}/etc/grub.d/10_linux"

if [ ! -f "${GRUB_SCRIPT}" ]; then
    echo "patch_grub_quiet: ${GRUB_SCRIPT} not found, skipping" >&2
    exit 0
fi

python3 - "${GRUB_SCRIPT}" <<'PY'
import sys
from pathlib import Path

path = Path(sys.argv[1])
text = path.read_text()
needle = 'quiet_boot="0"\nquick_boot="0"\ngfxpayload_dynamic="0"\nvt_handoff="0"\n'
replacement = '''quiet_boot="0"
quick_boot="0"
gfxpayload_dynamic="0"
vt_handoff="0"

# CutefishOS: quiet boot must not print "Loading Linux ..." /
# "Loading initial ramdisk ..." messages from GRUB.
case " ${GRUB_CMDLINE_LINUX_DEFAULT} " in
    *" quiet "*) quiet_boot="1" ;;
esac

# CutefishOS: keep the framebuffer/VT handoff smooth when Plymouth splash is
# enabled (uses Debian's existing vt_handoff logic to add vt.handoff=7).
case " ${GRUB_CMDLINE_LINUX_DEFAULT} " in
    *" splash "*) vt_handoff="1" ;;
esac
'''
if 'CutefishOS: quiet boot' not in text:
    if needle in text:
        text = text.replace(needle, replacement, 1)
        print('patch_grub_quiet: variables patched')
    else:
        print('patch_grub_quiet: /etc/grub.d/10_linux layout has changed, no patch applied', file=sys.stderr)
        sys.exit(1)

# The stock Debian condition only suppresses the messages for the
# "simple" top-level entry. CutefishOS disables the submenu, so the real
# entry is "advanced" and would still print; drop the type check as well.
old_cond = 'if [ x"$quiet_boot" = x0 ] || [ x"$type" != xsimple ]; then'
new_cond = 'if [ x"$quiet_boot" = x0 ]; then'
count = text.count(old_cond)
if count:
    text = text.replace(old_cond, new_cond)
    print(f'patch_grub_quiet: quieted {count} echo conditions')
elif 'CutefishOS: quiet boot' in text:
    print('patch_grub_quiet: already fully patched')

path.write_text(text)
PY
