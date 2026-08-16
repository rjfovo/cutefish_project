#pragma once

#include "core_state.h"

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

    // Internal protocol implementation surface used by server.cpp.
    CoreState *state() const;
    wl_resource *trustedShellResource() const;
    void setTrustedShellResource(wl_resource *resource);
    void handleCoreReady(wl_resource *resource, uint32_t version, uint32_t mode);

private:
    bool registerGlobals(wl_display *display, bool trustedShellDisplay);
    void broadcastLifecycle() const;

    CoreState *m_state = nullptr;
    wl_display *m_appsDisplay = nullptr;
    wl_display *m_shellDisplay = nullptr;
    wl_resource *m_trustedShellResource = nullptr;
    int m_terminatePipe[2] = {-1, -1};
    bool m_terminated = false;

    static WaylandServer *s_instance;
};

} // namespace Cutefish
