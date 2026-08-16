#pragma once

#include "wayland/server.h"

#include <wayland-server-core.h>

namespace Cutefish {

// Registers the minimal stable xdg-shell global used by the stage-0
// VirtualBackend smoke baseline. No window-management policy is implemented
// here; toplevel move/resize and maximization are intentionally no-ops.
bool registerXdgShellGlobals(wl_display *display, WaylandServer *server);

} // namespace Cutefish
