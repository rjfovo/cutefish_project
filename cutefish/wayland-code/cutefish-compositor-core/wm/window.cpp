#include "wm/window.h"

namespace Cutefish {

Window::Window(wl_resource *surface, wl_resource *xdgSurface, wl_resource *xdgToplevel,
               QObject *parent)
    : QObject(parent)
    , m_surface(surface)
    , m_xdgSurface(xdgSurface)
    , m_xdgToplevel(xdgToplevel)
{
}

Window::~Window() = default;

wl_resource *Window::surface() const
{
    return m_surface;
}

wl_resource *Window::xdgSurface() const
{
    return m_xdgSurface;
}

wl_resource *Window::xdgToplevel() const
{
    return m_xdgToplevel;
}

QString Window::title() const
{
    return m_title;
}

void Window::setTitle(const QString &title)
{
    m_title = title;
}

QString Window::appId() const
{
    return m_appId;
}

void Window::setAppId(const QString &appId)
{
    m_appId = appId;
}

QSize Window::requestedSize() const
{
    return m_requestedSize;
}

void Window::setRequestedSize(const QSize &size)
{
    m_requestedSize = size;
}

Window::State Window::state() const
{
    return m_state;
}

void Window::setState(State state)
{
    if (m_state == state)
        return;
    m_state = state;
    emit stateChanged();
}

bool Window::activated() const
{
    return m_activated;
}

void Window::setActivated(bool activated)
{
    m_activated = activated;
}

uint32_t Window::lastConfigureSerial() const
{
    return m_lastConfigureSerial;
}

void Window::setLastConfigureSerial(uint32_t serial)
{
    m_lastConfigureSerial = serial;
}

uint32_t Window::id() const
{
    return m_id;
}

void Window::setId(uint32_t id)
{
    m_id = id;
}

} // namespace Cutefish
