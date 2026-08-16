#!/bin/bash
# Single visual line + persistent Plymouth daemon.
#
# Boot path:
#   initramfs starts Plymouth
#     -> systemd keeps the daemon alive
#     -> SDDM starts Xorg
#     -> Xsetup runs "plymouth deactivate" once X owns the display
#     -> desktop session runs normally while Plymouth idles
#
# Shutdown/reboot path:
#   Xstop reactivates the same daemon (no theme/DRM reload)
#     -> plymouth-reboot/poweroff/halt only change-mode + show-splash
#
# The automatic plymouth-quit/quit-wait units are neutralized so they cannot
# kill the daemon after boot. A dedicated fallback unit still quits Plymouth if
# SDDM fails.
#
# Usage: patch_plymouth_persistent.sh <rootfs>
set -e

ROOTFS="${1:?usage: patch_plymouth_persistent.sh <rootfs>}"
LIBEXEC="${ROOTFS}/usr/lib/cutefish"
UNIT_DIR="${ROOTFS}/etc/systemd/system"
SDDM_CONF_DIR="${ROOTFS}/etc/sddm.conf.d"

mkdir -p "${LIBEXEC}" "${SDDM_CONF_DIR}"

# ---------------------------------------------------------------- scripts ---
cat > "${LIBEXEC}/plymouth-start.sh" <<'EOF'
#!/bin/sh
if /usr/bin/plymouth --ping; then
    /usr/bin/plymouth show-splash || true
else
    /usr/sbin/plymouthd --mode=boot --pid-file=/run/plymouth/pid --attach-to-session
    /usr/bin/plymouth show-splash || true
fi
EOF

cat > "${LIBEXEC}/plymouth-shutdown.sh" <<'EOF'
#!/bin/sh
mode="${1:-shutdown}"

if /usr/bin/plymouth --ping; then
    case "${mode}" in
        reboot)
            /usr/bin/plymouth change-mode --reboot || true
            ;;
        *)
            /usr/bin/plymouth change-mode --shutdown || true
            ;;
    esac
    /usr/bin/plymouth show-splash || true
else
    case "${mode}" in
        reboot)
            /usr/sbin/plymouthd --mode=reboot --attach-to-session
            ;;
        *)
            /usr/sbin/plymouthd --mode=shutdown --attach-to-session
            ;;
    esac
    /usr/bin/plymouth show-splash || true
fi
EOF

cat > "${LIBEXEC}/sddm-locale-wrapper" <<'EOF'
#!/bin/sh
# Start SDDM with the locale selected by cutefish-settings.
if [ -r /var/tmp/cutefish-locale.conf ]; then
    set -a
    . /var/tmp/cutefish-locale.conf
    set +a
fi

exec /usr/bin/sddm "$@"
EOF

cat > "${LIBEXEC}/sddm-plymouth-setup" <<'EOF'
#!/bin/sh
# SDDM/Plymouth display hand-over helper.
#
# X11: keep the persistent Plymouth daemon alive until Xsetup deactivates it.
# Wayland: quit Plymouth before weston starts, otherwise Plymouth owns the DRM
# device and the boot screen never goes away.

display_server=""

# SDDM reads these files in order; the last matching value wins.
for conf in /usr/lib/sddm/sddm.conf.d/*.conf /etc/sddm.conf.d/*.conf /etc/sddm.conf; do
    [ -f "$conf" ] || continue
    value="$(sed -e 's/#.*//' "$conf" | sed -n 's/^[[:space:]]*DisplayServer[[:space:]]*=[[:space:]]*//p' | tail -1)"
    [ -n "$value" ] && display_server="$value"
fi

if [ "$display_server" = "wayland" ]; then
    if /usr/bin/plymouth --ping; then
        /usr/bin/plymouth quit || true
    fi
    /usr/bin/plymouth --wait || true
else
    if /usr/bin/plymouth --ping; then
        /usr/bin/plymouth show-splash || true
    fi
fi

exit 0
EOF

cat > "${LIBEXEC}/sddm-plymouth-xsetup" <<'EOF'
#!/bin/sh
# Let the vendor Xsetup run first, then release the display to Xorg while
# keeping the Plymouth daemon alive and theme loaded.
if [ -x /usr/share/sddm/scripts/Xsetup ]; then
    /usr/share/sddm/scripts/Xsetup
fi

if /usr/bin/plymouth --ping; then
    /usr/bin/plymouth deactivate || true
fi
EOF

cat > "${LIBEXEC}/sddm-plymouth-xstop" <<'EOF'
#!/bin/sh
# Bring the persistent splash back before the vendor Xstop runs. This is the
# first visible frame after X releases the VT, so shutdown never falls back to
# a black console.
if /usr/bin/plymouth --ping; then
    /usr/bin/plymouth reactivate || true
fi

if [ -x /usr/share/sddm/scripts/Xstop ]; then
    /usr/share/sddm/scripts/Xstop
fi
EOF

chmod 755 "${LIBEXEC}/plymouth-start.sh" "${LIBEXEC}/plymouth-shutdown.sh" \
          "${LIBEXEC}/sddm-plymouth-setup" "${LIBEXEC}/sddm-locale-wrapper" \
          "${LIBEXEC}/sddm-plymouth-xsetup" "${LIBEXEC}/sddm-plymouth-xstop"

# ----------------------------------------------------------- systemd units ---
mkdir -p "${UNIT_DIR}/plymouth-start.service.d" \
         "${UNIT_DIR}/plymouth-quit.service.d" \
         "${UNIT_DIR}/plymouth-quit-wait.service.d" \
         "${UNIT_DIR}/plymouth-reboot.service.d" \
         "${UNIT_DIR}/plymouth-poweroff.service.d" \
         "${UNIT_DIR}/plymouth-halt.service.d" \
         "${UNIT_DIR}/plymouth-kexec.service.d" \
         "${UNIT_DIR}/sddm.service.d"

cat > "${UNIT_DIR}/plymouth-start.service.d/cutefish-persistent.conf" <<'EOF'
[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=
ExecStart=/usr/lib/cutefish/plymouth-start.sh
ExecStartPost=
EOF

for unit in plymouth-quit plymouth-quit-wait; do
    cat > "${UNIT_DIR}/${unit}.service.d/cutefish-persistent.conf" <<'EOF'
[Service]
ExecStart=
ExecStart=/bin/true
EOF
done

for unit_mode in "plymouth-reboot:reboot" "plymouth-poweroff:shutdown" \
                 "plymouth-halt:shutdown" "plymouth-kexec:shutdown"; do
    unit="${unit_mode%%:*}"
    mode="${unit_mode##*:}"
    cat > "${UNIT_DIR}/${unit}.service.d/cutefish-persistent.conf" <<EOF
[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=
ExecStart=/usr/lib/cutefish/plymouth-shutdown.sh ${mode}
ExecStartPost=
EOF
done

cat > "${UNIT_DIR}/sddm.service.d/cutefish-plymouth.conf" <<'EOF'
[Unit]
# Keep Plymouth alive; SDDM takes over the display only after Xsetup.
Conflicts=plymouth-quit.service plymouth-quit-wait.service
After=plymouth-start.service systemd-user-sessions.service plymouth-quit.service plymouth-quit-wait.service
OnFailure=cutefish-plymouth-fallback.service

[Service]
# X11: keep the splash visible until Xsetup releases it.
# Wayland: quit Plymouth so weston can open the DRM device.
ExecStartPre=-/usr/lib/cutefish/sddm-plymouth-setup

# Use the user-selected locale (if any) instead of the static system locale.
ExecStart=
ExecStart=/usr/lib/cutefish/sddm-locale-wrapper
EOF

# Dedicated fallback that really quits Plymouth only when SDDM fails.
cat > "${UNIT_DIR}/cutefish-plymouth-fallback.service" <<'EOF'
[Unit]
Description=Quit Plymouth after display-manager failure
DefaultDependencies=no

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=-/usr/bin/plymouth quit
EOF

# ------------------------------------------------------------- SDDM config ---
cat > "${SDDM_CONF_DIR}/10-cutefish-plymouth.conf" <<'EOF'
[X11]
DisplayCommand=/usr/lib/cutefish/sddm-plymouth-xsetup
DisplayStopCommand=/usr/lib/cutefish/sddm-plymouth-xstop
EOF

echo "patch_plymouth_persistent: installed persistent daemon integration"
