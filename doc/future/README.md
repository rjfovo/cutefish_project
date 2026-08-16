# Future Direction

Long-term architecture direction for CutefishOS, separate from the current
X11-based product line.

## Documents

- `wayland-unified-compositor-roadmap.md`
  Unified compositor strategy for boot, login, desktop and shutdown.

- `kde-kwin-dependency-analysis.md`
  Current KDE/KWin dependencies and the path to removing them.

- `stage1-x11-to-kwin-wayland.md`
  Stage 1 implementation notes: X11 KWin -> Wayland KWin.

- `../../build_iso/script/sddm-switch-display-server.sh`
  Toggle the installed SDDM between X11 and Wayland greeters.

## Decision Summary

- Keep the current X11/Plymouth/SDDM stack as a stable fallback.
- Do not extend the X11 boot chain further.
- Prepare a Wayland session first, then evolve it into `cutefish-compositor`.
- KWin can be removed later, but is a useful first Wayland runtime.
