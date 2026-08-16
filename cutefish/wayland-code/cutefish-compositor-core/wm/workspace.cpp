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
    const int offset = static_cast<int>(m_windows.size()) * 24;
    window->setGeometry(QRect(32 + offset, 32 + offset, 640, 480));
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

Window *Workspace::windowAt(const QPointF &position) const
{
    const auto windows = windowsInStackingOrder();
    for (Window *window : windows) {
        if (window->geometry().contains(position.toPoint()))
            return window;
    }
    return activeWindow();
}

void Workspace::raiseWindow(Window *window)
{
    if (!window)
        return;
    m_windows.removeAll(window);
    m_windows.prepend(window);
}

void Workspace::beginMove(Window *window)
{
    if (!window)
        return;
    endInteraction();
    raiseWindow(window);
    setActiveWindow(window);
    window->setInteractiveMove(true);
}

void Workspace::beginResize(Window *window, uint32_t edges)
{
    Q_UNUSED(edges)
    if (!window)
        return;
    endInteraction();
    raiseWindow(window);
    setActiveWindow(window);
    window->setInteractiveResize(true);
}

void Workspace::updateInteraction(const QPointF &position)
{
    Window *window = activeWindow();
    if (!window)
        return;
    if (window->interactiveMove()) {
        QRect geometry = window->geometry();
        geometry.moveCenter(position.toPoint());
        window->setGeometry(geometry);
    } else if (window->interactiveResize()) {
        QRect geometry = window->geometry();
        geometry.setBottomRight(position.toPoint());
        if (geometry.width() < 160 || geometry.height() < 120)
            return;
        window->setGeometry(geometry);
        window->setRequestedSize(geometry.size());
    }
}

void Workspace::endInteraction()
{
    for (Window *window : std::as_const(m_windows)) {
        window->setInteractiveMove(false);
        window->setInteractiveResize(false);
    }
}

QList<Window *> Workspace::windowsInStackingOrder() const
{
    QList<Window *> result;
    for (Window *window : m_windows)
        result.append(window);
    return result;
}

} // namespace Cutefish
