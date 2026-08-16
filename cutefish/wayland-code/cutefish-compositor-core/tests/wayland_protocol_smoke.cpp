// Isolated userspace smoke test. It uses the safe VirtualBackend only and
// creates a private XDG_RUNTIME_DIR under /tmp. It never opens DRM or input
// devices and never performs privileged operations.

#include "cutefish-core-v1-client-protocol.h"
#include "xdg-activation-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QThread>

#include <wayland-client.h>

#include <cstdlib>
#include <unistd.h>
#include <cstring>

namespace {

struct RegistryState {
    bool compositor = false;
    bool shm = false;
    bool seat = false;
    bool xdgShell = false;
    bool activation = false;
    bool dataManager = false;
    bool core = false;
    uint32_t coreName = 0;
    uint32_t compositorName = 0;
    uint32_t xdgName = 0;
    uint32_t seatName = 0;
    uint32_t activationName = 0;
    uint32_t dataManagerName = 0;
};

struct ToplevelState {
    int configureCount = 0;
    int activatedConfigures = 0;
    bool activated = false;
    bool maximized = false;
};

struct ActivationState {
    char token[128] = {};
};

struct DataState {
    wl_data_offer *offer = nullptr;
    bool sourceSent = false;
    QByteArray received;
};

void dataSourceSend(void *data, wl_data_source *source, const char *mimeType, int32_t fd)
{
    Q_UNUSED(source)
    auto *state = static_cast<DataState *>(data);
    if (qstrcmp(mimeType, "text/plain") != 0) {
        close(fd);
        return;
    }
    const QByteArray payload = QByteArrayLiteral("hello-cutefish-clipboard");
    size_t written = 0;
    while (written < static_cast<size_t>(payload.size())) {
        const ssize_t n = write(fd, payload.constData() + written, payload.size() - written);
        if (n < 0 && errno == EAGAIN)
            continue;
        if (n <= 0)
            break;
        written += static_cast<size_t>(n);
    }
    close(fd);
    state->sourceSent = written == payload.size();
}

void dataDeviceSelection(void *data, wl_data_device *device, wl_data_offer *offer)
{
    Q_UNUSED(device)
    auto *state = static_cast<DataState *>(data);
    state->offer = offer;
    wl_data_offer_accept(offer, 0, "text/plain");
}

struct CoreWindowState {
    int windowCount = 0;
    int stateChanges = 0;
    int destroyed = 0;
};

void coreLifecycle(void *data, cutefish_core_v1 *core, uint32_t state)
{
    Q_UNUSED(data)
    Q_UNUSED(core)
    Q_UNUSED(state)
}

void coreOutput(void *data, cutefish_core_v1 *core, const char *name,
                int32_t width, int32_t height, int32_t scale, int32_t transform, uint32_t connected)
{
    Q_UNUSED(data)
    Q_UNUSED(core)
    Q_UNUSED(name)
    Q_UNUSED(width)
    Q_UNUSED(height)
    Q_UNUSED(scale)
    Q_UNUSED(transform)
    Q_UNUSED(connected)
}

void coreFocusChanged(void *data, cutefish_core_v1 *core, const char *appId)
{
    Q_UNUSED(data)
    Q_UNUSED(core)
    Q_UNUSED(appId)
}

void coreLockChanged(void *data, cutefish_core_v1 *core, uint32_t locked)
{
    Q_UNUSED(data)
    Q_UNUSED(core)
    Q_UNUSED(locked)
}

void coreWindowEvent(void *data, cutefish_core_v1 *core, uint32_t id,
                     const char *appId, const char *title, uint32_t state, uint32_t activated)
{
    Q_UNUSED(core)
    Q_UNUSED(id)
    Q_UNUSED(appId)
    Q_UNUSED(title)
    Q_UNUSED(state)
    Q_UNUSED(activated)
    auto *tracker = static_cast<CoreWindowState *>(data);
    tracker->windowCount++;
}

void coreWindowDestroyed(void *data, cutefish_core_v1 *core, uint32_t id)
{
    Q_UNUSED(core)
    Q_UNUSED(id)
    static_cast<CoreWindowState *>(data)->destroyed++;
}

void coreWindowStateChanged(void *data, cutefish_core_v1 *core, uint32_t id,
                            uint32_t state, uint32_t activated)
{
    Q_UNUSED(core)
    Q_UNUSED(id)
    Q_UNUSED(state)
    Q_UNUSED(activated)
    static_cast<CoreWindowState *>(data)->stateChanges++;
}

struct InputState {
    bool keyboardEnter = false;
    bool pointerEnter = false;
    bool keymap = false;
    bool modifiers = false;
};

void keyboardKeymap(void *data, wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size)
{
    Q_UNUSED(keyboard)
    auto *state = static_cast<InputState *>(data);
    if (format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1 && size > 0)
        state->keymap = true;
    if (fd >= 0)
        close(fd);
}

void keyboardEnter(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
{
    Q_UNUSED(keyboard)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
    Q_UNUSED(keys)
    static_cast<InputState *>(data)->keyboardEnter = true;
}

void keyboardModifiers(void *data, wl_keyboard *keyboard, uint32_t serial,
                       uint32_t depressed, uint32_t latched, uint32_t locked, uint32_t group)
{
    Q_UNUSED(keyboard)
    Q_UNUSED(serial)
    Q_UNUSED(depressed)
    Q_UNUSED(latched)
    Q_UNUSED(locked)
    Q_UNUSED(group)
    static_cast<InputState *>(data)->modifiers = true;
}

void pointerEnter(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface,
                  wl_fixed_t x, wl_fixed_t y)
{
    Q_UNUSED(pointer)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
    Q_UNUSED(x)
    Q_UNUSED(y)
    static_cast<InputState *>(data)->pointerEnter = true;
}

struct PopupState {
    int configureCount = 0;
    int x = -1;
    int y = -1;
    int width = -1;
    int height = -1;
};

void popupConfigure(void *data, xdg_popup *popup, int32_t x, int32_t y, int32_t width, int32_t height)
{
    Q_UNUSED(popup)
    auto *state = static_cast<PopupState *>(data);
    state->configureCount++;
    state->x = x;
    state->y = y;
    state->width = width;
    state->height = height;
}

void popupDone(void *data, xdg_popup *popup)
{
    Q_UNUSED(data)
    Q_UNUSED(popup)
}

void toplevelConfigure(void *data, xdg_toplevel *toplevel, int32_t width, int32_t height, wl_array *states)
{
    Q_UNUSED(toplevel)
    Q_UNUSED(width)
    Q_UNUSED(height)
    auto *tracker = static_cast<ToplevelState *>(data);
    tracker->configureCount++;
    tracker->activated = false;
    tracker->maximized = false;
    for (size_t i = 0; i < states->size / sizeof(uint32_t); ++i) {
        uint32_t state = 0;
        std::memcpy(&state, static_cast<char *>(states->data) + i * sizeof(uint32_t), sizeof(state));
        if (state == XDG_TOPLEVEL_STATE_ACTIVATED) {
            tracker->activated = true;
            tracker->activatedConfigures++;
        }
        if (state == XDG_TOPLEVEL_STATE_MAXIMIZED)
            tracker->maximized = true;
    }
}

void toplevelClose(void *data, xdg_toplevel *toplevel)
{
    Q_UNUSED(data)
    Q_UNUSED(toplevel)
}

void activationDone(void *data, xdg_activation_token_v1 *token, const char *value)
{
    Q_UNUSED(token)
    auto *state = static_cast<ActivationState *>(data);
    std::strncpy(state->token, value ? value : "", sizeof(state->token) - 1);
}

void registryGlobal(void *data, wl_registry *registry, uint32_t name,
                    const char *interface, uint32_t version)
{
    Q_UNUSED(registry)
    Q_UNUSED(version)
    auto *state = static_cast<RegistryState *>(data);
    if (std::strcmp(interface, wl_compositor_interface.name) == 0)
        state->compositor = true;
    else if (std::strcmp(interface, wl_shm_interface.name) == 0)
        state->shm = true;
    else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
        state->seat = true;
        state->seatName = name;
    }
    else if (std::strcmp(interface, "xdg_wm_base") == 0) {
        state->xdgShell = true;
        state->xdgName = name;
    } else if (std::strcmp(interface, "xdg_activation_v1") == 0) {
        state->activation = true;
        state->activationName = name;
    } else if (std::strcmp(interface, wl_data_device_manager_interface.name) == 0) {
        state->dataManager = true;
        state->dataManagerName = name;
    } else if (std::strcmp(interface, cutefish_core_v1_interface.name) == 0) {
        state->core = true;
        state->coreName = name;
    }
}

void registryGlobalRemove(void *data, wl_registry *registry, uint32_t name)
{
    Q_UNUSED(data)
    Q_UNUSED(registry)
    Q_UNUSED(name)
}

const struct wl_registry_listener registryListener = {
    registryGlobal,
    registryGlobalRemove,
};

bool scanSocket(const QString &socketName, RegistryState *state)
{
    wl_display *display = wl_display_connect(socketName.toUtf8().constData());
    if (!display) {
        qCritical() << "connect failed for socket" << socketName;
        return false;
    }

    wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registryListener, state);
    if (wl_display_roundtrip(display) < 0) {
        qCritical() << "roundtrip failed for socket" << socketName;
        wl_registry_destroy(registry);
        wl_display_disconnect(display);
        return false;
    }

    bool ok = true;
    qInfo() << "socket" << socketName
            << "compositor" << state->compositor
            << "shm" << state->shm
            << "seat" << state->seat
            << "xdg_wm_base" << state->xdgShell
            << "xdg_activation" << state->activation
            << "cutefish_core_v1" << state->core;

    cutefish_core_v1 *core = nullptr;
    CoreWindowState coreWindowState;
    if (state->core) {
        core = static_cast<cutefish_core_v1 *>(
            wl_registry_bind(registry, state->coreName, &cutefish_core_v1_interface, 2));
        if (!core) {
            qCritical() << "bind cutefish_core_v1 failed";
            ok = false;
        } else {
            cutefish_core_v1_listener coreListener {};
            coreListener.lifecycle_state = coreLifecycle;
            coreListener.output = coreOutput;
            coreListener.window_focus_changed = coreFocusChanged;
            coreListener.lock_changed = coreLockChanged;
            coreListener.window = coreWindowEvent;
            coreListener.window_destroyed = coreWindowDestroyed;
            coreListener.window_state_changed = coreWindowStateChanged;
            cutefish_core_v1_add_listener(core, &coreListener, &coreWindowState);
            cutefish_core_v1_shell_ready(core, 2, 1);
            cutefish_core_v1_get_outputs(core);
            cutefish_core_v1_get_windows(core);
            if (wl_display_roundtrip(display) < 0) {
                qCritical() << "cutefish_core_v1 request roundtrip failed";
                ok = false;
            }
        }
    }

    InputState inputState;
    wl_seat *seat = nullptr;
    wl_pointer *pointer = nullptr;
    wl_keyboard *keyboard = nullptr;

    if (state->seat && state->seatName) {
        seat = static_cast<wl_seat *>(
            wl_registry_bind(registry, state->seatName, &wl_seat_interface, 5));
        pointer = wl_seat_get_pointer(seat);
        keyboard = wl_seat_get_keyboard(seat);
        wl_pointer_listener pointerListener {};
        pointerListener.enter = pointerEnter;
        wl_pointer_add_listener(pointer, &pointerListener, &inputState);
        wl_keyboard_listener keyboardListener {};
        keyboardListener.keymap = keyboardKeymap;
        keyboardListener.enter = keyboardEnter;
        keyboardListener.modifiers = keyboardModifiers;
        wl_keyboard_add_listener(keyboard, &keyboardListener, &inputState);
    }

    if (state->xdgShell && state->compositor && state->xdgName && state->compositorName) {
        auto *compositor = static_cast<wl_compositor *>(
            wl_registry_bind(registry, state->compositorName, &wl_compositor_interface, 4));
        auto *wm = static_cast<xdg_wm_base *>(
            wl_registry_bind(registry, state->xdgName, &xdg_wm_base_interface, 1));
        wl_surface *surface = wl_compositor_create_surface(compositor);
        xdg_surface *xdgSurface = xdg_wm_base_get_xdg_surface(wm, surface);
        xdg_toplevel *toplevel = xdg_surface_get_toplevel(xdgSurface);
        if (!surface || !xdgSurface || !toplevel) {
            qCritical() << "xdg toplevel creation failed";
            ok = false;
        } else {
            ToplevelState tracker;
            xdg_toplevel_listener listener {};
            listener.configure = toplevelConfigure;
            listener.close = toplevelClose;
            xdg_toplevel_add_listener(toplevel, &listener, &tracker);
            xdg_toplevel_set_title(toplevel, "wayland-protocol-smoke");
            xdg_toplevel_set_app_id(toplevel, "com.cutefish.smoke");
            xdg_toplevel_set_min_size(toplevel, 100, 80);
            wl_surface_commit(surface);
            if (wl_display_roundtrip(display) < 0) {
                qCritical() << "xdg initial roundtrip failed";
                ok = false;
            }
            if (tracker.configureCount < 1 || !tracker.activated) {
                qCritical() << "xdg initial configure missing activated state";
                ok = false;
            }
            if (!inputState.keymap) {
                qCritical() << "seat did not send xkb keymap";
                ok = false;
            }
            if (!inputState.pointerEnter || !inputState.keyboardEnter || !inputState.modifiers) {
                qCritical() << "seat focus enter/modifiers missing"
                            << inputState.pointerEnter << inputState.keyboardEnter << inputState.modifiers;
                ok = false;
            }
            xdg_toplevel_move(toplevel, seat, 1);
            xdg_toplevel_resize(toplevel, seat, 1, XDG_TOPLEVEL_RESIZE_EDGE_BOTTOM_RIGHT);
            if (wl_display_roundtrip(display) < 0) {
                qCritical() << "xdg move/resize request roundtrip failed";
                ok = false;
            }
            xdg_toplevel_set_maximized(toplevel);
            if (wl_display_roundtrip(display) < 0 || !tracker.maximized) {
                qCritical() << "xdg maximized configure missing";
                ok = false;
            }
            if (core && coreWindowState.windowCount < 1) {
                qCritical() << "cutefish_core_v1 window event missing";
                ok = false;
            }
            if (core && coreWindowState.stateChanges < 1) {
                qCritical() << "cutefish_core_v1 window state-change event missing";
                ok = false;
            }

            if (state->activation && state->activationName) {
                auto *activation = static_cast<xdg_activation_v1 *>(
                    wl_registry_bind(registry, state->activationName, &xdg_activation_v1_interface, 1));
                xdg_activation_token_v1 *token = xdg_activation_v1_get_activation_token(activation);
                ActivationState activationState;
                xdg_activation_token_v1_listener activationListener {};
                activationListener.done = activationDone;
                xdg_activation_token_v1_add_listener(token, &activationListener, &activationState);
                xdg_activation_token_v1_set_app_id(token, "com.cutefish.smoke");
                xdg_activation_token_v1_set_surface(token, surface);
                xdg_activation_token_v1_commit(token);
                if (wl_display_roundtrip(display) < 0 || activationState.token[0] == '\0') {
                    qCritical() << "xdg_activation token done missing";
                    ok = false;
                } else {
                    xdg_activation_v1_activate(activation, activationState.token, surface);
                    if (wl_display_roundtrip(display) < 0 || tracker.activatedConfigures < 2) {
                        qCritical() << "xdg_activation did not produce activated configure"
                                    << tracker.activatedConfigures;
                        ok = false;
                    }
                }
                xdg_activation_token_v1_destroy(token);
                xdg_activation_v1_destroy(activation);
            }

            xdg_positioner *positioner = xdg_wm_base_create_positioner(wm);
            xdg_positioner_set_size(positioner, 200, 100);
            xdg_positioner_set_anchor_rect(positioner, 0, 0, 200, 100);
            xdg_positioner_set_anchor(positioner, XDG_POSITIONER_ANCHOR_TOP_LEFT);
            xdg_positioner_set_gravity(positioner, XDG_POSITIONER_GRAVITY_BOTTOM_RIGHT);
            xdg_positioner_set_offset(positioner, 10, 20);
            xdg_popup *popup = xdg_surface_get_popup(xdgSurface, xdgSurface, positioner);
            PopupState popupState;
            xdg_popup_listener popupListener {};
            popupListener.configure = popupConfigure;
            popupListener.popup_done = popupDone;
            xdg_popup_add_listener(popup, &popupListener, &popupState);
            if (wl_display_roundtrip(display) < 0) {
                qCritical() << "xdg popup roundtrip failed";
                ok = false;
            }
            if (popupState.configureCount < 1 || popupState.x != 10 || popupState.y != 20 ||
                popupState.width != 200 || popupState.height != 100) {
                qCritical() << "xdg popup configure mismatch"
                            << popupState.configureCount << popupState.x << popupState.y
                            << popupState.width << popupState.height;
                ok = false;
            }
            xdg_popup_destroy(popup);
            xdg_positioner_destroy(positioner);
            xdg_toplevel_destroy(toplevel);
            xdg_surface_destroy(xdgSurface);
            wl_surface_destroy(surface);
        }
        wl_compositor_destroy(compositor);
        xdg_wm_base_destroy(wm);
    }

    if (core)
        cutefish_core_v1_destroy(core);
    if (pointer)
        wl_pointer_destroy(pointer);
    if (keyboard)
        wl_keyboard_destroy(keyboard);
    if (seat)
        wl_seat_destroy(seat);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);
    return ok;
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    if (argc < 2) {
        qCritical() << "usage: wayland_protocol_smoke <cutefish-compositor-core>";
        return 1;
    }
    const QString coreBinary = QString::fromLocal8Bit(argv[1]);

    QTemporaryDir runtimeDir(QStringLiteral("/tmp/cutefish-smoke-XXXXXX"));
    runtimeDir.setAutoRemove(true);
    if (!runtimeDir.isValid()) {
        qCritical() << "failed to create private runtime dir";
        return 1;
    }
    QFile::setPermissions(runtimeDir.path(),
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);

    const QString prefix = QStringLiteral("smoke-%1").arg(::getpid());
    const QString appsSocket = prefix + QStringLiteral("-apps");
    const QString shellSocket = prefix + QStringLiteral("-shell");

    QProcess core;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("XDG_RUNTIME_DIR"), runtimeDir.path());
    core.setProcessEnvironment(env);
    core.setProcessChannelMode(QProcess::ForwardedChannels);
    core.start(coreBinary, {QStringLiteral("--virtual"),
                            QStringLiteral("--runtime-dir"), runtimeDir.path(),
                            QStringLiteral("--socket-prefix"), prefix});
    if (!core.waitForStarted(5000)) {
        qCritical() << "core did not start";
        return 1;
    }

    QElapsedTimer timer;
    timer.start();
    bool socketsReady = false;
    while (timer.elapsed() < 7000) {
        if (QFileInfo::exists(runtimeDir.path() + QLatin1Char('/') + appsSocket) &&
            QFileInfo::exists(runtimeDir.path() + QLatin1Char('/') + shellSocket)) {
            socketsReady = true;
            break;
        }
        QThread::msleep(20);
    }
    if (!socketsReady) {
        qCritical() << "sockets not ready in time";
        core.kill();
        core.waitForFinished();
        return 1;
    }

    ::setenv("XDG_RUNTIME_DIR", runtimeDir.path().toUtf8().constData(), 1);

    RegistryState shellState;
    const bool shellOk = scanSocket(shellSocket, &shellState);
    RegistryState appsState;
    const bool appsOk = scanSocket(appsSocket, &appsState);

    core.terminate();
    if (!core.waitForFinished(5000)) {
        core.kill();
        core.waitForFinished(2000);
    }
    qInfo() << "core exited with code" << core.exitCode();

    if (!shellOk || !appsOk)
        return 1;
    if (!shellState.compositor || !shellState.shm || !shellState.seat || !shellState.xdgShell || !shellState.activation || !shellState.core) {
        qCritical() << "shell socket missing expected globals";
        return 1;
    }
    if (!appsState.compositor || !appsState.shm || !appsState.seat || !appsState.xdgShell || !appsState.activation) {
        qCritical() << "apps socket missing expected base globals";
        return 1;
    }
    if (appsState.core) {
        qCritical() << "apps socket must not expose cutefish_core_v1";
        return 1;
    }

    qInfo() << "PASS: dual-socket isolation verified; VirtualBackend server and cutefish_core_v1 smoke OK";
    return 0;
}
