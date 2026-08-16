#pragma once

#include <QImage>
#include <QList>
#include <QPoint>
#include <QSize>
#include <QString>

namespace Cutefish {

struct OutputInfo {
    QString name;
    QList<QSize> modes;
    QSize currentMode;
    int scale = 1;
    int transform = 0;
    bool connected = false;
};

class Buffer;

// DisplayBackend is the replaceable output interface. Stage-0 ships the safe
// VirtualBackend only; the KMS backend is intentionally not executed here.
class DisplayBackend {
public:
    virtual ~DisplayBackend() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual QList<OutputInfo> outputs() const = 0;
    virtual bool setMode(const QString &name, const QSize &mode) = 0;
    virtual bool setDpms(const QString &name, bool on) = 0;
    virtual void setCursor(const QImage &image, const QPoint &hotspot) = 0;
    virtual Buffer *createBuffer(const QSize &size) = 0;
    virtual void present(OutputInfo *output, Buffer *buffer) = 0;
};

class Buffer {
public:
    virtual ~Buffer() = default;
    virtual QSize size() const = 0;
};

} // namespace Cutefish
