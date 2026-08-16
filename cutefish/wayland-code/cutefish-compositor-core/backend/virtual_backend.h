#pragma once

#include "backend/display_backend.h"

namespace Cutefish {

class VirtualBuffer : public Buffer {
public:
    explicit VirtualBuffer(const QSize &size);
    QSize size() const override;

private:
    QSize m_size;
};

class VirtualBackend : public DisplayBackend {
public:
    bool initialize() override;
    void shutdown() override;
    QList<OutputInfo> outputs() const override;
    bool setMode(const QString &name, const QSize &mode) override;
    bool setDpms(const QString &name, bool on) override;
    void setCursor(const QImage &image, const QPoint &hotspot) override;
    Buffer *createBuffer(const QSize &size) override;
    void present(OutputInfo *output, Buffer *buffer) override;

private:
    QList<OutputInfo> m_outputs;
};

} // namespace Cutefish
