/*
 * xdg-shell 最小实现入口。
 * 职责：xdg_wm_base/xdg_surface/xdg_toplevel/xdg_popup/xdg_positioner。
 * 状态机：toplevel 状态变化必须经 configure 事件通知客户端。
 */
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
