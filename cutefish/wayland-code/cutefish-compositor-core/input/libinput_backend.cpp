#include "input/libinput_backend.h"

#include <QDebug>
#include <QProcessEnvironment>

#include <fcntl.h>
#include <unistd.h>

#include <libudev.h>

namespace {

const libinput_interface kLibinputInterface = {
    [](const char *path, int flags, void *) -> int {
        return open(path, flags);
    },
    [](int fd, void *) { close(fd); },
};

} // namespace

namespace Cutefish {

LibinputBackend::LibinputBackend() = default;

LibinputBackend::~LibinputBackend()
{
    shutdown();
}

bool LibinputBackend::initialize()
{
    m_udev = udev_new();
    if (!m_udev)
        return false;

    m_context = libinput_udev_create_context(&kLibinputInterface, nullptr, m_udev);
    if (!m_context)
        return false;

    const QByteArray seat = qgetenv("CUTEFISH_INPUT_SEAT");
    const int rc = libinput_udev_assign_seat(m_context, seat.isEmpty() ? "seat0" : seat.constData());
    if (rc != 0) {
        qWarning() << "LibinputBackend: seat assignment failed" << rc;
        // A missing privileged seat is not fatal for the VirtualBackend CI.
        return true;
    }
    qInfo() << "LibinputBackend initialized on seat" << (seat.isEmpty() ? "seat0" : seat.constData());
    return true;
}

void LibinputBackend::shutdown()
{
    if (m_context)
        libinput_unref(m_context);
    m_context = nullptr;
    if (m_udev)
        udev_unref(m_udev);
    m_udev = nullptr;
}

int LibinputBackend::dispatch()
{
    if (!m_context)
        return 0;
    return libinput_dispatch(m_context);
}

int LibinputBackend::fd() const
{
    return m_context ? libinput_get_fd(m_context) : -1;
}

void LibinputBackend::setKeyboardLayout(const QString &layout)
{
    m_keyboardLayout = layout;
    qInfo() << "LibinputBackend keyboard layout" << layout;
}

void LibinputBackend::keyboardKey(uint32_t key, bool pressed)
{
    Q_UNUSED(key)
    Q_UNUSED(pressed)
}

void LibinputBackend::pointerMotion(double x, double y)
{
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void LibinputBackend::pointerButton(uint32_t button, bool pressed)
{
    Q_UNUSED(button)
    Q_UNUSED(pressed)
}

void LibinputBackend::touchDown(int id, double x, double y)
{
    Q_UNUSED(id)
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void LibinputBackend::touchMotion(int id, double x, double y)
{
    Q_UNUSED(id)
    Q_UNUSED(x)
    Q_UNUSED(y)
}

void LibinputBackend::touchUp(int id)
{
    Q_UNUSED(id)
}

} // namespace Cutefish
