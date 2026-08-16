#!/bin/bash
# Switch the installed SDDM between the X11 greeter and the Wayland greeter.
#
# Usage: sddm-switch-display-server.sh [x11|wayland]
set -e

MODE="${1:?usage: sddm-switch-display-server.sh [x11|wayland]}"

# The persistent Plymouth daemon owns the DRM device. It must fully exit
# before Weston or another Wayland compositor can open the display.
/usr/bin/plymouth quit 2>/dev/null || true
/usr/bin/plymouth --wait 2>/dev/null || true

if [ "${MODE}" = "wayland" ]; then
    cat > /etc/sddm.conf <<'EOF'
[General]
DisplayServer=wayland

[Theme]
Current=cutefish

[Wayland]
CompositorCommand=weston --shell=kiosk
SessionDir=/usr/share/wayland-sessions
EOF
elif [ "${MODE}" = "x11" ]; then
    cat > /etc/sddm.conf <<'EOF'
[Theme]
Current=cutefish
EOF
else
    echo "unknown mode: ${MODE}" >&2
    exit 1
fi

systemctl restart sddm
echo "SDDM switched to ${MODE}"
