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
    m_windows.prepend(window);
    setActiveWindow(window);
    return window;
}

void Workspace::destroyWindow(Window *window)
{
    if (!window)
        return;
    if (m_activeWindow == window)
        m_activeWindow = nullptr;
    m_windows.removeAll(window);
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
}

QList<Window *> Workspace::windows() const
{
    return m_windows;
}

QList<Window *> Workspace::windowsInStackingOrder() const
{
    QList<Window *> result;
    for (Window *window : m_windows)
        result.append(window);
    return result;
}

} // namespace Cutefish
