# KDE / KWin Dependency Analysis

## Current dependencies

| Package | KDE-related dependencies |
|---|---|
| `cutefish-core` | `libkf6coreaddons6`, `libkf6idletime6`, `libkf6windowsystem6` |
| `cutefish-dock` | `libkf6windowsystem6` |
| `cutefish-settings` | `libkf6configcore6`, `kscreen` |
| `cutefish-kwin-plugins` | `kwin-x11`, `libkdecorations3-6` |
| Runtime | `kwin_x11` |

Current relationship:

```text
Cutefish
  -> KWin
  -> KF6 / KDE Frameworks
  -> X11
```

## Dependency reduction path

### Stage 1: X11 KWin -> Wayland KWin

- Replace `kwin_x11` with `kwin_wayland`.
- Remove Xorg from the critical path.
- Still depends on KWin.

### Stage 2: KWin -> cutefish-compositor

- Implement the shell and window management around Qt Wayland Compositor and
  KWayland.
- Remove `kwin-wayland`, `kwin-common`, `libkdecorations`, and KWin plugins.
- Likely still use useful KF6 libraries:
  - KWayland
  - KWindowSystem
  - KIdleTime
  - KGlobalAccel
  - KConfig

```text
Cutefish
  -> cutefish-compositor
  -> QtWaylandCompositor + KWayland
  -> KF6 base libraries
```

### Stage 3: optional full KDE removal

Can replace KF6 with one of:

- Pure Qt Wayland Compositor, or
- wlroots + a Qt/C++ shell.

Cost:

- Window management policy
- XWayland integration
- Screenshot / screencast / clipboard / DnD
- Global shortcuts
- Output and HiDPI management
- Idle and power management
- Lock-screen security and input methods

## Recommendation

- Stage 1 first: validate the Cutefish desktop on `kwin_wayland`.
- Keep KWin until the Cutefish shell is Wayland-clean.
- Replace the compositor only after the shell is stable.
- Full KDE removal is optional and should be a separate product decision.
