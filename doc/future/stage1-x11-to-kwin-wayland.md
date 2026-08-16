# Stage 1: X11 KWin -> Wayland KWin

## Status

Code implemented, package built. Full physical-device validation is still
required before making this the default session.

## Changes

- `cutefish-wayland-session.desktop`
  Installed to `/usr/share/wayland-sessions/`.

- `cutefish-session`
  - `--wayland` / `-w` runs the session manager on the offscreen QPA.
  - Starts `kwin_wayland --xwayland --socket wayland-0`.
  - Publishes `WAYLAND_DISPLAY=wayland-0` for child processes.
  - Skips the X11-specific WM-check event loop.
  - Skips `cutefish-xembedsniproxy` (X11 tray bridge).
  - Skips `picom` (X11 compositor) in Wayland sessions.

- `cutefish-core`
  - Runtime dependencies now include:
    - `kwin-wayland`
    - `qt6-wayland`
    - `xwayland`

## Session files

```text
/usr/share/xsessions/cutefish-xsession.desktop
  -> X11 fallback

/usr/share/wayland-sessions/cutefish-wayland-session.desktop
  -> Wayland session
```

## Validation checklist

On physical hardware:

0. Switch SDDM display server:
   `build_iso/script/sddm-switch-display-server.sh wayland`
   (return with `... x11`).

1. Boot with SDDM configured to use the Wayland session.
2. Verify desktop, dock, statusbar, launcher.
3. Verify filemanager, settings, terminal.
4. Verify XWayland fallback applications.
5. Verify logout/lock/suspend/reboot.
6. Verify multi-monitor and HiDPI.

## Known risks

- SDDM Wayland greeter support is still experimental.
- Several Cutefish components may contain hidden X11-only paths.
- `kwin_wayland` needs KDE configuration and plugins that are normally loaded
  by a Plasma session; a minimal Cutefish session must provide equivalents.
