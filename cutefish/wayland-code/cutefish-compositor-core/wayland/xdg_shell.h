#pragma once

#include "wayland/server.h"
#include "wm/window.h"

#include <QList>
#include <QSize>

#include <wayland-server-core.h>

namespace Cutefish {

bool registerXdgShellGlobals(wl_display *display, WaylandServer *server);
void configureXdgToplevel(Window *window, const QSize &size, const QList<uint32_t> &states);

} // namespace Cutefish
