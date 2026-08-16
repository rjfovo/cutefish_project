#include "wayland/xdg_shell.h"

#include "xdg-shell-server-protocol.h"

#include <QDebug>

#include <algorithm>

namespace Cutefish {
namespace {

constexpr uint32_t kXdgVersion = 1;

struct XdgResourceData {
    wl_resource *resource = nullptr;
    WaylandServer *server = nullptr;
};

struct xdg_wm_base_interface xdgWmBaseImplementation;
struct xdg_positioner_interface xdgPositionerImplementation;
struct xdg_surface_interface xdgSurfaceImplementation;
struct xdg_toplevel_interface xdgToplevelImplementation;
struct xdg_popup_interface xdgPopupImplementation;

void xdgDestroyData(wl_resource *resource)
{
    delete static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
}

// xdg_wm_base -------------------------------------------------------------
void xdgWmBaseDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void xdgWmBaseCreatePositioner(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    auto *rd = new XdgResourceData;
    rd->resource = wl_resource_create(client, &xdg_positioner_interface, kXdgVersion, id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->server = data ? data->server : nullptr;
    wl_resource_set_implementation(rd->resource, &xdgPositionerImplementation, rd, xdgDestroyData);
}

void xdgWmBaseGetXdgSurface(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface)
{
    Q_UNUSED(surface)
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    auto *rd = new XdgResourceData;
    rd->resource = wl_resource_create(client, &xdg_surface_interface, kXdgVersion, id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->server = data ? data->server : nullptr;
    wl_resource_set_implementation(rd->resource, &xdgSurfaceImplementation, rd, xdgDestroyData);
}

void xdgWmBasePong(wl_client *client, wl_resource *resource, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(serial)
}


// xdg_positioner ----------------------------------------------------------
void xdgPositionerDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void xdgPositionerSetSize(wl_client *client, wl_resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void xdgPositionerSetAnchorRect(wl_client *client, wl_resource *resource,
                                int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void xdgPositionerSetAnchor(wl_client *client, wl_resource *resource, uint32_t anchor)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(anchor)
}

void xdgPositionerSetGravity(wl_client *client, wl_resource *resource, uint32_t gravity)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(gravity)
}

void xdgPositionerSetConstraintAdjustment(wl_client *client, wl_resource *resource, uint32_t adjustment)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(adjustment)
}

void xdgPositionerSetOffset(wl_client *client, wl_resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
}


// xdg_surface -------------------------------------------------------------
void xdgSurfaceDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void sendInitialToplevelConfigure(wl_client *client, wl_resource *xdgSurface, wl_resource *toplevel)
{
    const uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    xdg_surface_send_configure(xdgSurface, serial);
    wl_array states;
    wl_array_init(&states);
    xdg_toplevel_send_configure(toplevel, 0, 0, &states);
    wl_array_release(&states);
}

void xdgSurfaceGetToplevel(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    auto *rd = new XdgResourceData;
    rd->resource = wl_resource_create(client, &xdg_toplevel_interface, kXdgVersion, id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->server = data ? data->server : nullptr;
    wl_resource_set_implementation(rd->resource, &xdgToplevelImplementation, rd, xdgDestroyData);
    sendInitialToplevelConfigure(client, resource, rd->resource);
}

void xdgSurfaceGetPopup(wl_client *client, wl_resource *resource, uint32_t id,
                        wl_resource *parent, wl_resource *positioner)
{
    Q_UNUSED(parent)
    Q_UNUSED(positioner)
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    auto *rd = new XdgResourceData;
    rd->resource = wl_resource_create(client, &xdg_popup_interface, kXdgVersion, id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->server = data ? data->server : nullptr;
    wl_resource_set_implementation(rd->resource, &xdgPopupImplementation, rd, xdgDestroyData);
    xdg_popup_send_configure(rd->resource, 0, 0, 0, 0);
}

void xdgSurfaceSetWindowGeometry(wl_client *client, wl_resource *resource,
                                 int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void xdgSurfaceAckConfigure(wl_client *client, wl_resource *resource, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(serial)
}


// xdg_toplevel ------------------------------------------------------------
void xdgToplevelDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void xdgToplevelSetParent(wl_client *client, wl_resource *resource, wl_resource *parent)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(parent)
}

void xdgToplevelSetTitle(wl_client *client, wl_resource *resource, const char *title)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    qInfo() << "xdg_toplevel title" << (title ? title : "");
}

void xdgToplevelSetAppId(wl_client *client, wl_resource *resource, const char *appId)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->server && data->server->state())
        data->server->state()->setFocusedAppId(QString::fromUtf8(appId ? appId : ""));
}

void xdgToplevelShowWindowMenu(wl_client *client, wl_resource *resource, wl_resource *seat,
                               uint32_t serial, int32_t x, int32_t y)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void xdgToplevelMove(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(seat)
    Q_UNUSED(serial)
}

void xdgToplevelResize(wl_client *client, wl_resource *resource, wl_resource *seat,
                       uint32_t serial, uint32_t edges)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(seat)
    Q_UNUSED(serial)
    Q_UNUSED(edges)
}

void xdgToplevelSetMaxSize(wl_client *client, wl_resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void xdgToplevelSetMinSize(wl_client *client, wl_resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void xdgToplevelSetMaximized(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
}

void xdgToplevelUnsetMaximized(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
}

void xdgToplevelSetFullscreen(wl_client *client, wl_resource *resource, wl_resource *output)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(output)
}

void xdgToplevelUnsetFullscreen(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
}

void xdgToplevelSetMinimized(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
}


// xdg_popup ---------------------------------------------------------------
void xdgPopupDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void xdgPopupGrab(wl_client *client, wl_resource *resource, wl_resource *seat, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(seat)
    Q_UNUSED(serial)
}

void xdgWmBaseBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *server = static_cast<WaylandServer *>(data);
    auto *rd = new XdgResourceData;
    rd->resource = wl_resource_create(client, &xdg_wm_base_interface,
                                      std::min(version, kXdgVersion), id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->server = server;
    wl_resource_set_implementation(rd->resource, &xdgWmBaseImplementation, rd, xdgDestroyData);
}

void installXdgImplementations()
{
    static bool installed = false;
    if (installed)
        return;
    installed = true;

    xdgWmBaseImplementation.destroy = xdgWmBaseDestroy;
    xdgWmBaseImplementation.create_positioner = xdgWmBaseCreatePositioner;
    xdgWmBaseImplementation.get_xdg_surface = xdgWmBaseGetXdgSurface;
    xdgWmBaseImplementation.pong = xdgWmBasePong;

    xdgPositionerImplementation.destroy = xdgPositionerDestroy;
    xdgPositionerImplementation.set_size = xdgPositionerSetSize;
    xdgPositionerImplementation.set_anchor_rect = xdgPositionerSetAnchorRect;
    xdgPositionerImplementation.set_anchor = xdgPositionerSetAnchor;
    xdgPositionerImplementation.set_gravity = xdgPositionerSetGravity;
    xdgPositionerImplementation.set_constraint_adjustment = xdgPositionerSetConstraintAdjustment;
    xdgPositionerImplementation.set_offset = xdgPositionerSetOffset;

    xdgSurfaceImplementation.destroy = xdgSurfaceDestroy;
    xdgSurfaceImplementation.get_toplevel = xdgSurfaceGetToplevel;
    xdgSurfaceImplementation.get_popup = xdgSurfaceGetPopup;
    xdgSurfaceImplementation.set_window_geometry = xdgSurfaceSetWindowGeometry;
    xdgSurfaceImplementation.ack_configure = xdgSurfaceAckConfigure;

    xdgToplevelImplementation.destroy = xdgToplevelDestroy;
    xdgToplevelImplementation.set_parent = xdgToplevelSetParent;
    xdgToplevelImplementation.set_title = xdgToplevelSetTitle;
    xdgToplevelImplementation.set_app_id = xdgToplevelSetAppId;
    xdgToplevelImplementation.show_window_menu = xdgToplevelShowWindowMenu;
    xdgToplevelImplementation.move = xdgToplevelMove;
    xdgToplevelImplementation.resize = xdgToplevelResize;
    xdgToplevelImplementation.set_max_size = xdgToplevelSetMaxSize;
    xdgToplevelImplementation.set_min_size = xdgToplevelSetMinSize;
    xdgToplevelImplementation.set_maximized = xdgToplevelSetMaximized;
    xdgToplevelImplementation.unset_maximized = xdgToplevelUnsetMaximized;
    xdgToplevelImplementation.set_fullscreen = xdgToplevelSetFullscreen;
    xdgToplevelImplementation.unset_fullscreen = xdgToplevelUnsetFullscreen;
    xdgToplevelImplementation.set_minimized = xdgToplevelSetMinimized;

    xdgPopupImplementation.destroy = xdgPopupDestroy;
    xdgPopupImplementation.grab = xdgPopupGrab;
}

} // namespace

bool registerXdgShellGlobals(wl_display *display, WaylandServer *server)
{
    installXdgImplementations();
    return wl_global_create(display, &xdg_wm_base_interface, kXdgVersion,
                            server, xdgWmBaseBind) != nullptr;
}

} // namespace Cutefish
