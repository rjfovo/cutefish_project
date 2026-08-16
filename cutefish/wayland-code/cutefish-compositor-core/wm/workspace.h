#pragma once

#include <QList>
#include <QObject>

struct wl_resource;

namespace Cutefish {

class Window;

class Workspace : public QObject {
    Q_OBJECT
public:
    explicit Workspace(QObject *parent = nullptr);

    Window *createWindow(wl_resource *surface, wl_resource *xdgSurface, wl_resource *xdgToplevel);
    void destroyWindow(Window *window);

    Window *windowForSurface(wl_resource *surface) const;
    Window *windowForToplevel(wl_resource *toplevel) const;
    Window *activeWindow() const;
    void setActiveWindow(Window *window);

    QList<Window *> windows() const;
    QList<Window *> windowsInStackingOrder() const;

private:
    QList<Window *> m_windows;
    Window *m_activeWindow = nullptr;
};

} // namespace Cutefish
