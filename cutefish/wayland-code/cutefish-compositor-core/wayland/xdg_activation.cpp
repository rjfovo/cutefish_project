#include "wayland/xdg_activation.h"
#include "wayland/server.h"
#include "xdg-shell-server-protocol.h"

#include "wayland/xdg_shell.h"
#include "wm/window.h"
#include "wm/workspace.h"
#include "xdg-activation-v1-server-protocol.h"

#include <QDebug>
#include <QSize>

#include <algorithm>

namespace Cutefish {

namespace {

constexpr uint32_t kActivationVersion = 1;

struct TokenResourceData {
    XdgActivation *manager = nullptr;
    ActivationToken *token = nullptr;
};

struct xdg_activation_token_v1_interface tokenImplementation;

void tokenDestroyData(wl_resource *resource)
{
    delete static_cast<TokenResourceData *>(wl_resource_get_user_data(resource));
}

void activationDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void activationGetToken(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *manager = static_cast<XdgActivation *>(wl_resource_get_user_data(resource));
    auto *data = new TokenResourceData;
    data->manager = manager;
    data->token = new ActivationToken;
    auto *tokenResource = wl_resource_create(client, &xdg_activation_token_v1_interface,
                                             kActivationVersion, id);
    if (!tokenResource) {
        delete data->token;
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(tokenResource, &tokenImplementation, data, tokenDestroyData);
}

void activationActivate(wl_client *client, wl_resource *resource,
                        const char *tokenValue, wl_resource *surface)
{
    Q_UNUSED(client)
    auto *manager = static_cast<XdgActivation *>(wl_resource_get_user_data(resource));
    WaylandServer *server = manager ? manager->server() : nullptr;
    if (!manager || !server || !tokenValue)
        return;

    ActivationToken *token = manager->takeToken(QString::fromUtf8(tokenValue));
    if (token)
        token->used = true;

    Window *target = server->workspace()->windowForSurface(surface);
    if (!target && token) {
        const auto windows = server->workspace()->windows();
        for (Window *window : windows) {
            if (window->appId() == token->appId) {
                target = window;
                break;
            }
        }
    }
    if (!target)
        return;

    Window *previous = server->workspace()->activeWindow();
    server->workspace()->setActiveWindow(target);

    QSize size = target->requestedSize();
    if (!size.isValid()) {
        const auto outputs = server->state()->displayBackend()->outputs();
        size = outputs.value(0).currentMode;
    }
    configureXdgToplevel(target, size, QList<uint32_t>{XDG_TOPLEVEL_STATE_ACTIVATED});
    if (previous && previous != target)
        configureXdgToplevel(previous, previous->requestedSize(), {});
}

void tokenDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void tokenSetSerial(wl_client *client, wl_resource *resource, uint32_t serial, wl_resource *seat)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(serial)
    Q_UNUSED(seat)
}

void tokenSetAppId(wl_client *client, wl_resource *resource, const char *appId)
{
    Q_UNUSED(client)
    auto *data = static_cast<TokenResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->token)
        data->token->appId = QString::fromUtf8(appId ? appId : "");
}

void tokenSetSurface(wl_client *client, wl_resource *resource, wl_resource *surface)
{
    Q_UNUSED(client)
    auto *data = static_cast<TokenResourceData *>(wl_resource_get_user_data(resource));
    if (data && data->token)
        data->token->surface = surface;
}

void tokenCommit(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto *data = static_cast<TokenResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->manager || !data->token || data->token->committed)
        return;
    data->token->committed = true;
    data->token->value = data->manager->createToken(data->token);
    xdg_activation_token_v1_send_done(resource, data->token->value.toUtf8().constData());
}

struct xdg_activation_v1_interface activationImplementation;

void installImplementations()
{
    static bool installed = false;
    if (installed)
        return;
    installed = true;

    activationImplementation.destroy = activationDestroy;
    activationImplementation.get_activation_token = activationGetToken;
    activationImplementation.activate = activationActivate;

    tokenImplementation.set_serial = tokenSetSerial;
    tokenImplementation.set_app_id = tokenSetAppId;
    tokenImplementation.set_surface = tokenSetSurface;
    tokenImplementation.commit = tokenCommit;
    tokenImplementation.destroy = tokenDestroy;
}

void activationBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *manager = static_cast<XdgActivation *>(data);
    auto *resource = wl_resource_create(client, &xdg_activation_v1_interface,
                                        std::min(version, kActivationVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &activationImplementation, manager, nullptr);
}

} // namespace

XdgActivation::XdgActivation(WaylandServer *server, QObject *parent)
    : QObject(parent)
    , m_server(server)
{
    installImplementations();
}

WaylandServer *XdgActivation::server() const
{
    return m_server;
}

bool XdgActivation::registerDisplay(wl_display *display)
{
    void *global = wl_global_create(display, &xdg_activation_v1_interface, kActivationVersion,
                                    this, activationBind);
    qInfo() << "xdg_activation_v1 global" << (global ? "registered" : "FAILED");
    return global != nullptr;
}

QString XdgActivation::createToken(ActivationToken *token)
{
    const QString value = QStringLiteral("cutefish-activation-%1").arg(m_nextTokenId++);
    m_tokens.insert(value, token);
    return value;
}

ActivationToken *XdgActivation::takeToken(const QString &token)
{
    return m_tokens.take(token);
}

} // namespace Cutefish
