#include "shellcoreclient.h"
#include "outputlistmodel.h"
#include "windowlistmodel.h"

#include <QDebug>
#include <QSocketNotifier>

#include <cstring>

namespace {

void registryGlobal(void *data, wl_registry *registry, uint32_t name,
                    const char *interface, uint32_t version)
{
    Q_UNUSED(version)
    auto *client = static_cast<ShellCoreClient *>(data);
    if (std::strcmp(interface, cutefish_core_v1_interface.name) != 0)
        return;
    client->setCoreProxy(static_cast<cutefish_core_v1 *>(
        wl_registry_bind(registry, name, &cutefish_core_v1_interface, 2)));
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

void coreLifecycle(void *data, cutefish_core_v1 *core, uint32_t state)
{
    Q_UNUSED(core)
    auto *client = static_cast<ShellCoreClient *>(data);
    client->setMode(static_cast<int>(state));
}

void coreOutput(void *data, cutefish_core_v1 *core, const char *name,
                int32_t width, int32_t height, int32_t scale, int32_t transform, uint32_t connected)
{
    Q_UNUSED(core)
    auto *client = static_cast<ShellCoreClient *>(data);
    ShellCoreClient::Output output;
    output.name = QString::fromUtf8(name ? name : "");
    output.width = width;
    output.height = height;
    output.scale = scale;
    output.transform = transform;
    output.connected = connected != 0;
    client->updateOutput(output);
}

void coreWindowFocus(void *data, cutefish_core_v1 *core, const char *appId)
{
    Q_UNUSED(data)
    Q_UNUSED(core)
    Q_UNUSED(appId)
}

void coreWindow(void *data, cutefish_core_v1 *core, uint32_t id,
                const char *appId, const char *title, uint32_t state, uint32_t activated)
{
    Q_UNUSED(core)
    auto *client = static_cast<ShellCoreClient *>(data);
    ShellCoreClient::WindowInfo window;
    window.id = id;
    window.appId = QString::fromUtf8(appId ? appId : "");
    window.title = QString::fromUtf8(title ? title : "");
    window.state = state;
    window.activated = activated != 0;
    client->updateWindow(window);
}

void coreWindowDestroyed(void *data, cutefish_core_v1 *core, uint32_t id)
{
    Q_UNUSED(core)
    auto *client = static_cast<ShellCoreClient *>(data);
    client->removeWindow(id);
}

void coreWindowStateChanged(void *data, cutefish_core_v1 *core, uint32_t id,
                            uint32_t state, uint32_t activated)
{
    Q_UNUSED(core)
    auto *client = static_cast<ShellCoreClient *>(data);
    client->updateWindowState(id, state, activated != 0);
}

void coreLockChanged(void *data, cutefish_core_v1 *core, uint32_t locked)
{
    Q_UNUSED(data)
    Q_UNUSED(core)
    Q_UNUSED(locked)
}

const struct cutefish_core_v1_listener coreListener = {
    coreLifecycle,
    coreOutput,
    coreWindowFocus,
    coreLockChanged,
    coreWindow,
    coreWindowDestroyed,
    coreWindowStateChanged,
};

} // namespace

ShellCoreClient::ShellCoreClient(QObject *parent)
    : QObject(parent)
    , m_outputModel(new OutputListModel(this))
    , m_windowModel(new WindowListModel(this))
{
}

ShellCoreClient::~ShellCoreClient()
{
    disconnectFromCore();
}

bool ShellCoreClient::connectToCore(const QString &socketName)
{
    if (m_display)
        return true;
    m_display = wl_display_connect(socketName.isEmpty() ? nullptr : socketName.toUtf8().constData());
    if (!m_display) {
        qWarning() << "ShellCoreClient: cannot connect to core socket" << socketName;
        return false;
    }
    m_registry = wl_display_get_registry(m_display);
    wl_registry_add_listener(m_registry, &registryListener, this);
    if (wl_display_roundtrip(m_display) < 0) {
        disconnectFromCore();
        return false;
    }
    if (!m_core) {
        qWarning() << "ShellCoreClient: cutefish_core_v1 not available on" << socketName;
        disconnectFromCore();
        return false;
    }
    cutefish_core_v1_add_listener(m_core, &coreListener, this);
    cutefish_core_v1_shell_ready(m_core, 2, static_cast<uint32_t>(m_mode));
    cutefish_core_v1_get_outputs(m_core);
    cutefish_core_v1_get_windows(m_core);
    wl_display_roundtrip(m_display);

    m_notifier = new QSocketNotifier(wl_display_get_fd(m_display), QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &ShellCoreClient::dispatch);
    emit connectedChanged();
    qInfo() << "ShellCoreClient connected to core socket" << socketName;
    return true;
}

void ShellCoreClient::disconnectFromCore()
{
    delete m_notifier;
    m_notifier = nullptr;
    if (m_core)
        cutefish_core_v1_destroy(m_core);
    m_core = nullptr;
    if (m_registry)
        wl_registry_destroy(m_registry);
    m_registry = nullptr;
    if (m_display)
        wl_display_disconnect(m_display);
    m_display = nullptr;
    m_outputs.clear();
    m_windows.clear();
    emit connectedChanged();
}

bool ShellCoreClient::connected() const
{
    return m_display && m_core;
}

int ShellCoreClient::mode() const
{
    return m_mode;
}

void ShellCoreClient::setMode(int mode)
{
    if (m_mode == mode)
        return;
    m_mode = mode;
    if (m_core)
        cutefish_core_v1_shell_ready(m_core, 2, static_cast<uint32_t>(mode));
    emit modeChanged();
}

void ShellCoreClient::dispatch()
{
    if (m_display)
        wl_display_dispatch_pending(m_display);
}

void ShellCoreClient::syncRequestGetOutputs()
{
    if (!m_core || !m_display)
        return;
    m_outputs.clear();
    cutefish_core_v1_get_outputs(m_core);
    wl_display_flush(m_display);
    wl_display_roundtrip(m_display);
}

void ShellCoreClient::requestGetOutputs()
{
    if (m_core) {
        m_outputs.clear();
        cutefish_core_v1_get_outputs(m_core);
        wl_display_flush(m_display);
    }
}

void ShellCoreClient::requestGetWindows()
{
    if (m_core) {
        m_windows.clear();
        cutefish_core_v1_get_windows(m_core);
        wl_display_flush(m_display);
    }
}

void ShellCoreClient::setOutputConfig(const QString &name, int width, int height, int scale, int transform)
{
    if (m_core) {
        cutefish_core_v1_set_output_config(m_core, name.toUtf8().constData(),
                                           width, height, scale, transform);
        wl_display_flush(m_display);
    }
}

void ShellCoreClient::requestActivate(const QString &appId)
{
    if (m_core) {
        cutefish_core_v1_request_activate(m_core, appId.toUtf8().constData());
        wl_display_flush(m_display);
    }
}

void ShellCoreClient::requestClose(const QString &appId)
{
    if (m_core) {
        cutefish_core_v1_request_close(m_core, appId.toUtf8().constData());
        wl_display_flush(m_display);
    }
}

QVector<ShellCoreClient::Output> ShellCoreClient::outputs() const
{
    return m_outputs;
}

QVector<ShellCoreClient::WindowInfo> ShellCoreClient::windows() const
{
    return m_windows;
}

QObject *ShellCoreClient::outputModel() const
{
    return m_outputModel;
}

QObject *ShellCoreClient::windowModel() const
{
    return m_windowModel;
}

void ShellCoreClient::setCoreProxy(cutefish_core_v1 *core)
{
    m_core = core;
}

void ShellCoreClient::updateOutput(const Output &output)
{
    for (auto &existing : m_outputs) {
        if (existing.name == output.name) {
            existing = output;
            m_outputModel->setOutputs(m_outputs);
            emit outputsChanged();
            return;
        }
    }
    m_outputs.append(output);
    m_outputModel->setOutputs(m_outputs);
    emit outputsChanged();
}

void ShellCoreClient::updateWindow(const WindowInfo &window)
{
    for (auto &existing : m_windows) {
        if (existing.id == window.id) {
            existing = window;
            m_windowModel->setWindows(m_windows);
            emit windowsChanged();
            return;
        }
    }
    m_windows.append(window);
    m_windowModel->setWindows(m_windows);
    emit windowsChanged();
}

void ShellCoreClient::updateWindowState(uint32_t id, uint32_t state, bool activated)
{
    for (auto &window : m_windows) {
        if (window.id == id) {
            window.state = state;
            window.activated = activated;
            m_windowModel->setWindows(m_windows);
            emit windowsChanged();
            return;
        }
    }
}

void ShellCoreClient::removeWindow(uint32_t id)
{
    for (int i = 0; i < m_windows.size(); ++i) {
        if (m_windows.at(i).id == id) {
            m_windows.removeAt(i);
            m_windowModel->setWindows(m_windows);
            emit windowsChanged();
            return;
        }
    }
}
