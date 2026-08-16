#include "wayland/server.h"
#include "input/libinput_backend.h"
#include "wayland/xdg_shell.h"
#include "wm/window.h"

#include "cutefish-core-v1-server-protocol.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMetaEnum>

#include <algorithm>
#include <array>

#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>

namespace Cutefish {

namespace {

constexpr uint32_t kCompositorVersion = 4;
constexpr uint32_t kOutputVersion = 3;
constexpr uint32_t kCoreProtocolVersion = 1;

struct ServerGlobals {
    WaylandServer *server = nullptr;
    bool trusted = false;
};

struct ResourceData {
    wl_resource *resource = nullptr;
    CoreState *state = nullptr;
    WaylandServer *server = nullptr;
    bool trusted = false;
};

struct SurfaceData : ResourceData {
    wl_resource *frameCallback = nullptr;
};

extern const struct wl_compositor_interface compositorImplementation;

void destroyResourceData(wl_resource *resource)
{
    delete static_cast<ResourceData *>(wl_resource_get_user_data(resource));
}

void compositorBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *globals = static_cast<ServerGlobals *>(data);
    auto *rd = new ResourceData;
    rd->resource = wl_resource_create(client, &wl_compositor_interface,
                                      std::min(version, kCompositorVersion), id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->state = globals->server->state();
    rd->server = globals->server;
    wl_resource_set_implementation(rd->resource, &compositorImplementation, rd, destroyResourceData);
}

void regionDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void regionAdd(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void regionSubtract(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

const struct wl_region_interface regionImplementation = {
    regionDestroy,
    regionAdd,
    regionSubtract,
};

void surfaceDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void surfaceAttach(wl_client *client, wl_resource *resource, wl_resource *buffer, int32_t x, int32_t y)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(buffer)
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void surfaceDamage(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

void surfaceFrame(wl_client *client, wl_resource *resource, uint32_t callbackId)
{
    auto *data = static_cast<SurfaceData *>(wl_resource_get_user_data(resource));
    if (!data)
        return;

    if (data->frameCallback)
        wl_resource_destroy(data->frameCallback);

    data->frameCallback = wl_resource_create(client, &wl_callback_interface, 1, callbackId);
    if (!data->frameCallback)
        wl_client_post_no_memory(client);
    else
        wl_resource_set_implementation(data->frameCallback, nullptr, data, nullptr);
}

void surfaceSetOpaqueRegion(wl_client *client, wl_resource *resource, wl_resource *region)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(region)
}

void surfaceSetInputRegion(wl_client *client, wl_resource *resource, wl_resource *region)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(region)
}

void surfaceCommit(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto *data = static_cast<SurfaceData *>(wl_resource_get_user_data(resource));
    if (!data)
        return;

    if (data->frameCallback) {
        wl_callback_send_done(data->frameCallback, 0);
        wl_resource_destroy(data->frameCallback);
        data->frameCallback = nullptr;
    }
}

void surfaceSetBufferTransform(wl_client *client, wl_resource *resource, int32_t transform)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(transform)
}

void surfaceSetBufferScale(wl_client *client, wl_resource *resource, int32_t scale)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(scale)
}

void surfaceDamageBuffer(wl_client *client, wl_resource *resource, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(width)
    Q_UNUSED(height)
}

const struct wl_surface_interface surfaceImplementation = {
    surfaceDestroy,
    surfaceAttach,
    surfaceDamage,
    surfaceFrame,
    surfaceSetOpaqueRegion,
    surfaceSetInputRegion,
    surfaceCommit,
    surfaceSetBufferTransform,
    surfaceSetBufferScale,
    surfaceDamageBuffer,
};

void compositorCreateSurface(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *compositor = static_cast<ResourceData *>(wl_resource_get_user_data(resource));
    auto *data = new SurfaceData;
    data->resource = wl_resource_create(client, &wl_surface_interface,
                                        std::min<uint32_t>(kCompositorVersion, wl_resource_get_version(resource)), id);
    if (!data->resource) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    data->state = compositor ? compositor->state : nullptr;
    data->server = compositor ? compositor->server : nullptr;
    wl_resource_set_implementation(data->resource, &surfaceImplementation, data, destroyResourceData);
}

void compositorCreateRegion(wl_client *client, wl_resource *resource, uint32_t id)
{
    Q_UNUSED(resource)
    auto *data = new ResourceData;
    data->resource = wl_resource_create(client, &wl_region_interface, 1, id);
    if (!data->resource) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(data->resource, &regionImplementation, data, destroyResourceData);
}

const struct wl_compositor_interface compositorImplementation = {
    compositorCreateSurface,
    compositorCreateRegion,
};

void outputRelease(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct wl_output_interface outputImplementation = {
    outputRelease,
};

void outputBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    Q_UNUSED(version)
    auto *globals = static_cast<ServerGlobals *>(data);
    auto *rd = new ResourceData;
    rd->resource = wl_resource_create(client, &wl_output_interface, kOutputVersion, id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->state = globals->server->state();
    rd->server = globals->server;
    wl_resource_set_implementation(rd->resource, &outputImplementation, rd, destroyResourceData);

    const auto outputs = rd->state->displayBackend()->outputs();
    const OutputInfo &out = outputs.value(0);
    const QByteArray make = QByteArrayLiteral("Cutefish");
    const QByteArray model = QByteArrayLiteral("VIRTUAL-1");
    wl_output_send_geometry(rd->resource, 0, 0, 0, 0, 0,
                            make.constData(), model.constData(), 0);
    wl_output_send_mode(rd->resource,
                        WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED,
                        out.currentMode.width(),
                        out.currentMode.height(),
                        60000);
    if (wl_resource_get_version(rd->resource) >= WL_OUTPUT_SCALE_SINCE_VERSION)
        wl_output_send_scale(rd->resource, out.scale);
    if (wl_resource_get_version(rd->resource) >= WL_OUTPUT_DONE_SINCE_VERSION)
        wl_output_send_done(rd->resource);
}

void coreDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void coreShellReady(wl_client *client, wl_resource *resource, uint32_t version, uint32_t mode)
{
    Q_UNUSED(client)
    Q_UNUSED(version)
    auto *data = static_cast<ResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->state)
        return;
    auto *server = data->server;
    if (!server)
        return;
    if (server->trustedShellResource() && server->trustedShellResource() != resource) {
        wl_resource_post_error(resource, 0, "cutefish_core_v1 already has a trusted shell client");
        return;
    }
    server->setTrustedShellResource(resource);
    server->handleCoreReady(resource, version, mode);
}

void coreBindShellSurface(wl_client *client, wl_resource *resource, wl_resource *surface,
                          uint32_t role, const char *output, int32_t zOrder)
{
    Q_UNUSED(client)
    if (!surface) {
        wl_resource_post_error(resource, 0, "bind_shell_surface requires a wl_surface");
        return;
    }
    qInfo() << "bind_shell_surface role" << role
            << "output" << (output ? output : QStringLiteral("")) << "z" << zOrder;
}

void coreGetOutputs(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto *data = static_cast<ResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->state || !data->state->displayBackend())
        return;
    const auto outputs = data->state->displayBackend()->outputs();
    for (const auto &out : outputs) {
        cutefish_core_v1_send_output(resource,
                                     out.name.toUtf8().constData(),
                                     out.currentMode.width(),
                                     out.currentMode.height(),
                                     out.scale,
                                     out.transform,
                                     out.connected ? 1u : 0u);
    }
}

void coreSetOutputConfig(wl_client *client, wl_resource *resource, const char *name,
                         int32_t modeWidth, int32_t modeHeight,
                         int32_t scale, int32_t transform)
{
    Q_UNUSED(client)
    auto *data = static_cast<ResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->state || !data->state->displayBackend())
        return;
    Q_UNUSED(scale)
    Q_UNUSED(transform)
    data->state->displayBackend()->setMode(QString::fromUtf8(name), QSize(modeWidth, modeHeight));
}

void coreSetDockGeometry(wl_client *client, wl_resource *resource,
                         int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    qInfo() << "dock geometry" << x << y << width << height;
}

void coreSetPanelGeometry(wl_client *client, wl_resource *resource,
                          int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    qInfo() << "panel geometry" << x << y << width << height;
}

void coreLock(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto *data = static_cast<ResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->state)
        return;
    data->state->lifecycle()->transitionTo(LifecycleStateMachine::State::Lock);
}

void coreUnlock(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto *data = static_cast<ResourceData *>(wl_resource_get_user_data(resource));
    if (!data || !data->state)
        return;
    data->state->lifecycle()->transitionTo(LifecycleStateMachine::State::Session);
}

void coreRequestActivate(wl_client *client, wl_resource *resource, const char *appId)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    qInfo() << "request_activate" << (appId ? appId : QStringLiteral(""));
}

void coreRequestClose(wl_client *client, wl_resource *resource, const char *appId)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    qInfo() << "request_close" << (appId ? appId : QStringLiteral(""));
}

const struct cutefish_core_v1_interface coreImplementation = {
    coreDestroy,
    coreShellReady,
    coreBindShellSurface,
    coreGetOutputs,
    coreSetOutputConfig,
    coreSetDockGeometry,
    coreSetPanelGeometry,
    coreLock,
    coreUnlock,
    coreRequestActivate,
    coreRequestClose,
};

void coreBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *globals = static_cast<ServerGlobals *>(data);
    auto *rd = new ResourceData;
    rd->resource = wl_resource_create(client, &cutefish_core_v1_interface,
                                      std::min(version, kCoreProtocolVersion), id);
    if (!rd->resource) {
        delete rd;
        wl_client_post_no_memory(client);
        return;
    }
    rd->state = globals->server->state();
    rd->server = globals->server;
    rd->trusted = true;
    wl_resource_set_implementation(rd->resource, &coreImplementation, rd, destroyResourceData);

    pid_t pid = 0;
    uid_t uid = 0;
    gid_t gid = 0;
    wl_client_get_credentials(client, &pid, &uid, &gid);
    qInfo() << "cutefish_core_v1 bound" << "pid" << pid << "uid" << uid << "gid" << gid;
}

void processLibinputEvents(LibinputBackend *backend, Seat *seat)
{
    if (!backend || !seat || !backend->context())
        return;
    while (libinput_event *event = libinput_get_event(backend->context())) {
        const libinput_event_type type = libinput_event_get_type(event);
        switch (type) {
        case LIBINPUT_EVENT_KEYBOARD_KEY: {
            auto *keyEvent = libinput_event_get_keyboard_event(event);
            const uint32_t state = libinput_event_keyboard_get_key_state(keyEvent) == LIBINPUT_KEY_STATE_PRESSED
                ? WL_KEYBOARD_KEY_STATE_PRESSED : WL_KEYBOARD_KEY_STATE_RELEASED;
            seat->keyboardKey(libinput_event_keyboard_get_key(keyEvent), state);
            break;
        }
        case LIBINPUT_EVENT_POINTER_MOTION: {
            auto *pointerEvent = libinput_event_get_pointer_event(event);
            seat->pointerMotion(libinput_event_pointer_get_dx(pointerEvent),
                                libinput_event_pointer_get_dy(pointerEvent));
            break;
        }
        case LIBINPUT_EVENT_POINTER_BUTTON: {
            auto *pointerEvent = libinput_event_get_pointer_event(event);
            const uint32_t state = libinput_event_pointer_get_button_state(pointerEvent) == LIBINPUT_BUTTON_STATE_PRESSED
                ? WL_POINTER_BUTTON_STATE_PRESSED : WL_POINTER_BUTTON_STATE_RELEASED;
            seat->pointerButton(libinput_event_pointer_get_button(pointerEvent), state);
            break;
        }
        default:
            break;
        }
        libinput_event_destroy(event);
    }
}

} // namespace

WaylandServer *WaylandServer::s_instance = nullptr;

WaylandServer::WaylandServer(CoreState *state, QObject *parent)
    : QObject(parent)
    , m_state(state)
    , m_workspace(new Workspace(this))
    , m_seat(new Seat(this))
{
    s_instance = this;
    connect(m_workspace, &Workspace::activeWindowChanged, this, [this](Window *window) {
        m_seat->setFocusSurface(window ? window->surface() : nullptr);
    });
    if (::pipe2(m_terminatePipe, O_CLOEXEC | O_NONBLOCK) != 0)
        qWarning() << "failed to create termination pipe";
}

WaylandServer::~WaylandServer()
{
    terminate();
    if (m_appsDisplay)
        wl_display_destroy(m_appsDisplay);
    if (m_shellDisplay)
        wl_display_destroy(m_shellDisplay);
    if (m_terminatePipe[0] >= 0)
        ::close(m_terminatePipe[0]);
    if (m_terminatePipe[1] >= 0)
        ::close(m_terminatePipe[1]);
    s_instance = nullptr;
}

bool WaylandServer::registerGlobals(wl_display *display, bool trustedShellDisplay)
{
    auto *globals = new ServerGlobals;
    globals->server = this;
    globals->trusted = trustedShellDisplay;

    if (!wl_global_create(display, &wl_compositor_interface, kCompositorVersion,
                          globals, compositorBind))
        return false;
    if (!wl_global_create(display, &wl_output_interface, kOutputVersion,
                          globals, outputBind))
        return false;
    if (wl_display_init_shm(display) != 0)
        return false;
    if (!m_seat->registerDisplay(display))
        return false;
    if (!registerXdgShellGlobals(display, this))
        return false;
    if (trustedShellDisplay) {
        if (!wl_global_create(display, &cutefish_core_v1_interface, kCoreProtocolVersion,
                              globals, coreBind))
            return false;
    }
    return true;
}

bool WaylandServer::start(const QString &runtimeDir,
                          const QString &appsSocketName,
                          const QString &shellSocketName)
{
    if (!m_state || !m_state->displayBackend())
        return false;

    QDir dir;
    if (!dir.mkpath(runtimeDir)) {
        qWarning() << "failed to create runtime dir" << runtimeDir;
        return false;
    }
    QFile::setPermissions(runtimeDir,
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    qputenv("XDG_RUNTIME_DIR", runtimeDir.toUtf8());

    QFile::remove(runtimeDir + QLatin1Char('/') + appsSocketName);
    QFile::remove(runtimeDir + QLatin1Char('/') + shellSocketName);

    m_appsDisplay = wl_display_create();
    m_shellDisplay = wl_display_create();
    if (!m_appsDisplay || !m_shellDisplay)
        return false;

    if (!registerGlobals(m_appsDisplay, false)) {
        qWarning() << "failed to register apps-socket globals";
        return false;
    }
    if (!registerGlobals(m_shellDisplay, true)) {
        qWarning() << "failed to register shell-socket globals";
        return false;
    }

    if (wl_display_add_socket(m_appsDisplay, appsSocketName.toUtf8().constData()) != 0) {
        qWarning() << "failed to add apps socket" << appsSocketName;
        return false;
    }
    if (wl_display_add_socket(m_shellDisplay, shellSocketName.toUtf8().constData()) != 0) {
        qWarning() << "failed to add shell socket" << shellSocketName;
        return false;
    }

    qInfo() << "Wayland server started"
            << "runtime" << runtimeDir
            << "apps-socket" << appsSocketName
            << "shell-socket" << shellSocketName
            << "cutefish_core_v1_on_shell_socket_only" << true;
    return true;
}

int WaylandServer::run()
{
    if (!m_shellDisplay || !m_appsDisplay)
        return -1;

    wl_event_loop *shellLoop = wl_display_get_event_loop(m_shellDisplay);
    wl_event_loop *appsLoop = wl_display_get_event_loop(m_appsDisplay);
    if (!shellLoop || !appsLoop)
        return -1;

    const int shellFd = wl_event_loop_get_fd(shellLoop);
    const int appsFd = wl_event_loop_get_fd(appsLoop);
    const int terminateFd = m_terminatePipe[0];
    auto *libinput = dynamic_cast<LibinputBackend *>(m_inputBackend);
    const int inputFd = libinput ? libinput->fd() : -1;
    if (shellFd < 0 || appsFd < 0 || terminateFd < 0)
        return -1;

    while (!m_terminated) {
        wl_display_flush_clients(m_shellDisplay);
        wl_display_flush_clients(m_appsDisplay);

        const int shellEvents = wl_event_loop_dispatch(shellLoop, 0);
        const int appsEvents = wl_event_loop_dispatch(appsLoop, 0);
        if (shellEvents < 0 || appsEvents < 0) {
            qInfo() << "Wayland event loop termination";
            break;
        }
        if (shellEvents == 0 && appsEvents == 0) {
            pollfd fds[4] = {
                { shellFd, POLLIN, 0 },
                { appsFd, POLLIN, 0 },
                { terminateFd, POLLIN, 0 },
                { inputFd, POLLIN, 0 },
            };
            const nfds_t count = inputFd >= 0 ? 4 : 3;
            const int prc = poll(fds, count, 100);
            if (prc < 0 && errno != EINTR)
                break;
            if (prc > 0 && (fds[2].revents & POLLIN))
                break;
            if (prc > 0 && inputFd >= 0 && (fds[3].revents & POLLIN) && libinput) {
                libinput->dispatch();
                processLibinputEvents(libinput, m_seat);
            }
        }
    }
    return 0;
}

void WaylandServer::terminate()
{
    if (m_terminated)
        return;
    m_terminated = true;
    const char token = 'q';
    if (m_terminatePipe[1] >= 0)
        ::write(m_terminatePipe[1], &token, 1);
}

wl_display *WaylandServer::appsDisplay() const
{
    return m_appsDisplay;
}

wl_display *WaylandServer::shellDisplay() const
{
    return m_shellDisplay;
}

WaylandServer *WaylandServer::instance()
{
    return s_instance;
}

Workspace *WaylandServer::workspace() const
{
    return m_workspace;
}

Seat *WaylandServer::seat() const
{
    return m_seat;
}

void WaylandServer::setInputBackend(InputBackend *backend)
{
    m_inputBackend = backend;
}

InputBackend *WaylandServer::inputBackend() const
{
    return m_inputBackend;
}

CoreState *WaylandServer::state() const
{
    return m_state;
}

wl_resource *WaylandServer::trustedShellResource() const
{
    return m_trustedShellResource;
}

void WaylandServer::setTrustedShellResource(wl_resource *resource)
{
    m_trustedShellResource = resource;
}

void WaylandServer::broadcastLifecycle() const
{
    if (!m_trustedShellResource)
        return;
    cutefish_core_v1_send_lifecycle_state(m_trustedShellResource,
                                          static_cast<uint32_t>(m_state->lifecycle()->state()));
}

void WaylandServer::handleCoreReady(wl_resource *resource, uint32_t version, uint32_t mode)
{
    Q_UNUSED(version)
    qInfo() << "trusted shell ready" << "protocol-version" << version << "mode" << mode;
    if (mode <= static_cast<uint32_t>(LifecycleStateMachine::State::Shutdown)) {
        m_state->lifecycle()->transitionTo(static_cast<LifecycleStateMachine::State>(mode));
    }
    cutefish_core_v1_send_lifecycle_state(resource,
                                          static_cast<uint32_t>(m_state->lifecycle()->state()));
}

} // namespace Cutefish
