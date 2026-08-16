#include "backend/virtual_backend.h"

#include <QDebug>

namespace Cutefish {

VirtualBuffer::VirtualBuffer(const QSize &size)
    : m_size(size)
{
}

QSize VirtualBuffer::size() const
{
    return m_size;
}

bool VirtualBackend::initialize()
{
    OutputInfo out;
    out.name = QStringLiteral("VIRTUAL-1");
    out.modes = {QSize(1920, 1080), QSize(1280, 720), QSize(1024, 768)};
    out.currentMode = out.modes.first();
    out.scale = 1;
    out.transform = 0;
    out.connected = true;
    m_outputs = {out};
    qInfo() << "VirtualBackend initialized" << out.name << out.currentMode;
    return true;
}

void VirtualBackend::shutdown()
{
    m_outputs.clear();
    qInfo() << "VirtualBackend shut down";
}

QList<OutputInfo> VirtualBackend::outputs() const
{
    return m_outputs;
}

bool VirtualBackend::setMode(const QString &name, const QSize &mode)
{
    for (OutputInfo &out : m_outputs) {
        if (out.name == name && out.modes.contains(mode)) {
            out.currentMode = mode;
            qInfo() << "VirtualBackend mode" << name << mode;
            return true;
        }
    }
    return false;
}

bool VirtualBackend::setDpms(const QString &name, bool on)
{
    Q_UNUSED(name)
    qInfo() << "VirtualBackend dpms" << on;
    return true;
}

void VirtualBackend::setCursor(const QImage &image, const QPoint &hotspot)
{
    qInfo() << "VirtualBackend cursor" << image.size() << hotspot;
}

void VirtualBackend::present(OutputInfo *output, Buffer *buffer)
{
    Q_UNUSED(output)
    Q_UNUSED(buffer)
    // Final presentation is intentionally a no-op in the virtual backend.
}

} // namespace Cutefish
