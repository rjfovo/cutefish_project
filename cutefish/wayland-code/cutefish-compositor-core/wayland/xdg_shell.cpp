#include "wayland/xdg_shell.h"

#include "xdg-shell-server-protocol.h"
#include "wm/window.h"
#include "wm/workspace.h"

#include <QDebug>
#include <QList>

#include <algorithm>

namespace Cutefish {
namespace {

constexpr uint32_t kXdgVersion = 1;

struct XdgResourceData {
    wl_resource *resource = nullptr;
    WaylandServer *server = nullptr;
    wl_resource *surface = nullptr;
    Window *window = nullptr;
};

struct XdgPositionerData : XdgResourceData {
    QSize size;
    QRect anchorRect;
    uint32_t anchor = XDG_POSITIONER_ANCHOR_NONE;
    uint32_t gravity = XDG_POSITIONER_GRAVITY_NONE;
    uint32_t constraintAdjustment = XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE;
    QPoint offset;
};

struct XdgPopupData : XdgResourceData {
    XdgPositionerData *positioner = nullptr;
    QRect geometry;
};

struct xdg_wm_base_interface xdgWmBaseImplementation;
struct xdg_positioner_interface xdgPositionerImplementation;
struct xdg_surface_interface xdgSurfaceImplementation;
struct xdg_toplevel_interface xdgToplevelImplementation;
struct xdg_popup_interface xdgPopupImplementation;

void xdgDestroyData(wl_resource *resource)
{
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (!data)
        return;
    if (data->window && data->server)
        data->server->workspace()->destroyWindow(data->window);
    delete data;
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
    auto *rd = new XdgPositionerData;
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
    rd->surface = surface;
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
    auto *data = static_cast<XdgPositionerData *>(wl_resource_get_user_data(resource));
    if (data)
        data->size = QSize(width, height);
}

void xdgPositionerSetAnchorRect(wl_client *client, wl_resource *resource,
                                int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    auto *data = static_cast<XdgPositionerData *>(wl_resource_get_user_data(resource));
    if (data)
        data->anchorRect = QRect(x, y, width, height);
}

void xdgPositionerSetAnchor(wl_client *client, wl_resource *resource, uint32_t anchor)
{
    Q_UNUSED(client)
    auto *data = static_cast<XdgPositionerData *>(wl_resource_get_user_data(resource));
    if (data)
        data->anchor = anchor;
}

void xdgPositionerSetGravity(wl_client *client, wl_resource *resource, uint32_t gravity)
{
    Q_UNUSED(client)
    auto *data = static_cast<XdgPositionerData *>(wl_resource_get_user_data(resource));
    if (data)
        data->gravity = gravity;
}

void xdgPositionerSetConstraintAdjustment(wl_client *client, wl_resource *resource, uint32_t adjustment)
{
    Q_UNUSED(client)
    auto *data = static_cast<XdgPositionerData *>(wl_resource_get_user_data(resource));
    if (data)
        data->constraintAdjustment = adjustment;
}

void xdgPositionerSetOffset(wl_client *client, wl_resource *resource, int32_t x, int32_t y)
{
    Q_UNUSED(client)
    auto *data = static_cast<XdgPositionerData *>(wl_resource_get_user_data(resource));
    if (data)
        data->offset = QPoint(x, y);
}


// xdg_surface -------------------------------------------------------------
void xdgSurfaceDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void sendToplevelConfigure(wl_client *client, wl_resource *xdgSurface,
                          wl_resource *toplevel, Window *window,
                          int32_t width, int32_t height,
                          const QList<uint32_t> &states)
{
    const uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    if (window)
        window->setLastConfigureSerial(serial);
    xdg_surface_send_configure(xdgSurface, serial);
    wl_array stateArray;
    wl_array_init(&stateArray);
    for (uint32_t state : states) {
        uint32_t *entry = static_cast<uint32_t *>(wl_array_add(&stateArray, sizeof(uint32_t)));
        if (entry)
            *entry = state;
    }
    xdg_toplevel_send_configure(toplevel, width, height, &stateArray);
    wl_array_release(&stateArray);
}

void sendInitialToplevelConfigure(wl_client *client, wl_resource *xdgSurface,
                                  wl_resource *toplevel, Window *window)
{
    sendToplevelConfigure(client, xdgSurface, toplevel, window, 0, 0,
                          {XDG_TOPLEVEL_STATE_ACTIVATED});
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
    if (rd->server && rd->server->workspace()) {
        // surface is passed to the xdg_surface request below; associate it
        // with the new Window model through the server workspace.
    }
    if (rd->server && data && data->surface)
        rd->window = rd->server->workspace()->createWindow(data->surface, resource, rd->resource);
    wl_resource_set_implementation(rd->resource, &xdgToplevelImplementation, rd, xdgDestroyData);
    sendInitialToplevelConfigure(client, resource, rd->resource, rd->window);
}

void xdgSurfaceGetPopup(wl_client *client, wl_resource *resource, uint32_t id,
                        wl_resource *parent, wl_resource *positioner)
{
    Q_UNUSED(parent)
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    auto *positionerData = positioner
        ? static_cast<XdgPositionerData *>(wl_resource_get_user_data(positioner))
        : nullptr;
    auto *rd = new XdgPopupData;
    rd->resource = wl_resource_create(client, &xdg_popup_interface, kXdgVersion, id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->server = data ? data->server : nullptr;
    rd->surface = data ? data->surface : nullptr;
    rd->positioner = positionerData;
    if (positionerData) {
        QPoint pos = positionerData->anchorRect.topLeft() + positionerData->offset;
        rd->geometry = QRect(pos, positionerData->size);
    }
    wl_resource_set_implementation(rd->resource, &xdgPopupImplementation, rd, xdgDestroyData);

    const uint32_t serial = wl_display_next_serial(wl_client_get_display(client));
    xdg_popup_send_configure(rd->resource, rd->geometry.x(), rd->geometry.y(),
                             rd->geometry.width(), rd->geometry.height());
    xdg_surface_send_configure(resource, serial);
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
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->server && data->window) {
        data->server->workspace()->destroyWindow(data->window);
        data->window = nullptr;
    }
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
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->window)
        data->window->setTitle(QString::fromUtf8(title ? title : ""));
    qInfo() << "xdg_toplevel title" << (title ? title : "");
}

void xdgToplevelSetAppId(wl_client *client, wl_resource *resource, const char *appId)
{
    Q_UNUSED(client)
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->window)
        data->window->setAppId(QString::fromUtf8(appId ? appId : ""));
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

QSize workspaceSize(WaylandServer *server)
{
    if (!server || !server->state() || !server->state()->displayBackend())
        return QSize(1920, 1080);
    const auto outputs = server->state()->displayBackend()->outputs();
    return outputs.value(0).currentMode;
}

void xdgToplevelSetMaxSize(wl_client *client, wl_resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->window)
        data->window->setRequestedSize(QSize(width, height));
}

void xdgToplevelSetMinSize(wl_client *client, wl_resource *resource, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->window && data->window->requestedSize().isEmpty())
        data->window->setRequestedSize(QSize(width, height));
}

void xdgToplevelSetMaximized(wl_client *client, wl_resource *resource)
{
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->window)
        return;
    data->window->setState(Window::State::Maximized);
    const QSize size = workspaceSize(data->server);
    sendToplevelConfigure(client, data->window->xdgSurface(), resource, data->window,
                          size.width(), size.height(),
                          {XDG_TOPLEVEL_STATE_MAXIMIZED, XDG_TOPLEVEL_STATE_ACTIVATED});
}

void xdgToplevelUnsetMaximized(wl_client *client, wl_resource *resource)
{
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->window)
        return;
    data->window->setState(Window::State::Normal);
    sendToplevelConfigure(client, data->window->xdgSurface(), resource, data->window, 0, 0,
                          {XDG_TOPLEVEL_STATE_ACTIVATED});
}

void xdgToplevelSetFullscreen(wl_client *client, wl_resource *resource, wl_resource *output)
{
    Q_UNUSED(output)
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->window)
        return;
    data->window->setState(Window::State::Fullscreen);
    const QSize size = workspaceSize(data->server);
    sendToplevelConfigure(client, data->window->xdgSurface(), resource, data->window,
                          size.width(), size.height(),
                          {XDG_TOPLEVEL_STATE_FULLSCREEN, XDG_TOPLEVEL_STATE_ACTIVATED});
}

void xdgToplevelUnsetFullscreen(wl_client *client, wl_resource *resource)
{
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->window)
        return;
    data->window->setState(Window::State::Normal);
    sendToplevelConfigure(client, data->window->xdgSurface(), resource, data->window, 0, 0,
                          {XDG_TOPLEVEL_STATE_ACTIVATED});
}

void xdgToplevelSetMinimized(wl_client *client, wl_resource *resource)
{
    auto *data = static_cast<XdgResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->window)
        return;
    data->window->setState(Window::State::Minimized);
    sendToplevelConfigure(client, data->window->xdgSurface(), resource, data->window, 0, 0, {});
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

void configureXdgToplevel(Window *window, const QSize &size, const QList<uint32_t> &states)
{
    if (!window || !window->xdgSurface() || !window->xdgToplevel())
        return;
    wl_client *client = wl_resource_get_client(window->xdgToplevel());
    sendToplevelConfigure(client, window->xdgSurface(), window->xdgToplevel(),
                          window, size.width(), size.height(), states);
}

} // namespace Cutefish
