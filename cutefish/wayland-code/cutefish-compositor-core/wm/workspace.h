/*
 * Workspace：窗口集合、焦点和 z-order 基础模型。
 * 线程边界：仅 core 主线程访问；信号用于 Seat focus 和 cutefish_core_v1 广播。
 */
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
    void notifyWindowStateChanged();

public slots:
    void handleWindowStateChanged();

signals:
    void activeWindowChanged(Window *window);
    void windowAdded(Window *window);
    void windowRemoved(Window *window);
    void windowStateChanged(Window *window);

private:
    QList<Window *> m_windows;
    Window *m_activeWindow = nullptr;
    uint32_t m_nextWindowId = 1;
};

} // namespace Cutefish
