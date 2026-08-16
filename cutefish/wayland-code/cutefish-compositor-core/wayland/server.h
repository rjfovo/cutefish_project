/*
 * WaylandServer：双 socket Wayland 服务。
 * 安全边界：
 *   - apps socket 不注册 cutefish_core_v1；
 *   - shell socket 注册 cutefish_core_v1；
 *   - 同一 Seat/Workspace 跨两个 socket 共享状态。
 * 进程边界：Shell 是可重启客户端；core 崩溃时由 systemd 重启并回到 Boot/Login。
 */
#pragma once

#include "core_state.h"
#include "input/input_backend.h"
#include "wayland/data_device.h"
#include "wayland/seat.h"
#include "wayland/text_input.h"
#include "wayland/xdg_activation.h"
#include "wm/workspace.h"

#include <QObject>
#include <QString>

#include <wayland-server-core.h>

namespace Cutefish {

class WaylandServer : public QObject {
    Q_OBJECT
public:
    explicit WaylandServer(CoreState *state, QObject *parent = nullptr);
    ~WaylandServer() override;

    bool start(const QString &runtimeDir,
               const QString &appsSocketName,
               const QString &shellSocketName);
    int run();
    void terminate();

    wl_display *appsDisplay() const;
    wl_display *shellDisplay() const;

    static WaylandServer *instance();

    Workspace *workspace() const;
    Seat *seat() const;
    XdgActivation *activation() const;
    DataDeviceManager *dataDevices() const;
    TextInputManager *textInput() const;
    void setInputBackend(InputBackend *backend);
    InputBackend *inputBackend() const;

    // Internal protocol implementation surface used by server.cpp.
    CoreState *state() const;
    wl_resource *trustedShellResource() const;
    void setTrustedShellResource(wl_resource *resource);
    void handleCoreReady(wl_resource *resource, uint32_t version, uint32_t mode);
    void sendWindowToShell(wl_resource *resource, Window *window);
    void notifyWindowRemoved(Window *window);
    void notifyWindowStateChanged(Window *window);

private:
    bool registerGlobals(wl_display *display, bool trustedShellDisplay);
    void broadcastLifecycle() const;

    CoreState *m_state = nullptr;
    Workspace *m_workspace = nullptr;
    Seat *m_seat = nullptr;
    XdgActivation *m_activation = nullptr;
    DataDeviceManager *m_dataDevices = nullptr;
    TextInputManager *m_textInput = nullptr;
    InputBackend *m_inputBackend = nullptr;
    wl_display *m_appsDisplay = nullptr;
    wl_display *m_shellDisplay = nullptr;
    wl_resource *m_trustedShellResource = nullptr;
    int m_terminatePipe[2] = {-1, -1};
    bool m_terminated = false;

    static WaylandServer *s_instance;
};

} // namespace Cutefish
