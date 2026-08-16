#include "wm/workspace.h"

#include "wm/window.h"

namespace Cutefish {

Workspace::Workspace(QObject *parent)
    : QObject(parent)
{
}

Window *Workspace::createWindow(wl_resource *surface, wl_resource *xdgSurface, wl_resource *xdgToplevel)
{
    auto *window = new Window(surface, xdgSurface, xdgToplevel, this);
    window->setId(++m_nextWindowId);
    connect(window, &Window::stateChanged, this, &Workspace::handleWindowStateChanged);
    m_windows.prepend(window);
    emit windowAdded(window);
    setActiveWindow(window);
    return window;
}

void Workspace::destroyWindow(Window *window)
{
    if (!window)
        return;
    if (m_activeWindow == window) {
        m_activeWindow = nullptr;
        emit activeWindowChanged(nullptr);
    }
    m_windows.removeAll(window);
    emit windowRemoved(window);
    window->deleteLater();
}

Window *Workspace::windowForSurface(wl_resource *surface) const
{
    for (Window *window : m_windows) {
        if (window->surface() == surface)
            return window;
    }
    return nullptr;
}

Window *Workspace::windowForToplevel(wl_resource *toplevel) const
{
    for (Window *window : m_windows) {
        if (window->xdgToplevel() == toplevel)
            return window;
    }
    return nullptr;
}

Window *Workspace::activeWindow() const
{
    return m_activeWindow;
}

void Workspace::setActiveWindow(Window *window)
{
    if (m_activeWindow == window)
        return;
    if (m_activeWindow)
        m_activeWindow->setActivated(false);
    m_activeWindow = window;
    if (m_activeWindow)
        m_activeWindow->setActivated(true);
    emit activeWindowChanged(m_activeWindow);
    emit windowStateChanged(window);
}

QList<Window *> Workspace::windows() const
{
    return m_windows;
}

void Workspace::notifyWindowStateChanged()
{
}

void Workspace::handleWindowStateChanged()
{
    auto *window = qobject_cast<Window *>(sender());
    if (window)
        emit windowStateChanged(window);
}

QList<Window *> Workspace::windowsInStackingOrder() const
{
    QList<Window *> result;
    for (Window *window : m_windows)
        result.append(window);
    return result;
}

} // namespace Cutefish
