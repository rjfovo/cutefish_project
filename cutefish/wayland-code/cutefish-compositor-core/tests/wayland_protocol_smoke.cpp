// Isolated userspace smoke test. It uses the safe VirtualBackend only and
// creates a private XDG_RUNTIME_DIR under /tmp. It never opens DRM or input
// devices and never performs privileged operations.

#include "cutefish-core-v1-client-protocol.h"

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
    bool core = false;
    uint32_t coreName = 0;
};

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
    else if (std::strcmp(interface, wl_seat_interface.name) == 0)
        state->seat = true;
    else if (std::strcmp(interface, "xdg_wm_base") == 0)
        state->xdgShell = true;
    else if (std::strcmp(interface, cutefish_core_v1_interface.name) == 0) {
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
            << "cutefish_core_v1" << state->core;

    if (state->core) {
        auto *core = static_cast<cutefish_core_v1 *>(
            wl_registry_bind(registry, state->coreName, &cutefish_core_v1_interface, 1));
        if (!core) {
            qCritical() << "bind cutefish_core_v1 failed";
            ok = false;
        } else {
            cutefish_core_v1_shell_ready(core, 1, 1);
            cutefish_core_v1_get_outputs(core);
            if (wl_display_roundtrip(display) < 0) {
                qCritical() << "cutefish_core_v1 request roundtrip failed";
                ok = false;
            }
            cutefish_core_v1_destroy(core);
        }
    }

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
    if (!shellState.compositor || !shellState.shm || !shellState.seat || !shellState.xdgShell || !shellState.core) {
        qCritical() << "shell socket missing expected globals";
        return 1;
    }
    if (!appsState.compositor || !appsState.shm || !appsState.seat || !appsState.xdgShell) {
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
