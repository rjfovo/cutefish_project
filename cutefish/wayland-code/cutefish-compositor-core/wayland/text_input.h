#pragma once

#include <QObject>

#include <wayland-server-core.h>

namespace Cutefish {

class WaylandServer;

class TextInputManager : public QObject {
    Q_OBJECT
public:
    explicit TextInputManager(WaylandServer *server, QObject *parent = nullptr);

    bool registerDisplay(wl_display *display);
    WaylandServer *server() const;

private:
    WaylandServer *m_server = nullptr;
};

} // namespace Cutefish
