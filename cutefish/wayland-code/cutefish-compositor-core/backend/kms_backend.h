#pragma once

#include "backend/display_backend.h"

#include <gbm.h>
#include <xf86drmMode.h>

#include <QHash>
#include <QString>

namespace Cutefish {

struct KmsBuffer final : Buffer {
    KmsBuffer(gbm_bo *bo, const QSize &size);
    ~KmsBuffer() override;

    QSize size() const override;

    gbm_bo *bo = nullptr;
    int dmabufFd = -1;
    uint32_t stride = 0;
    uint32_t handle = 0;
};

struct KmsOutput {
    OutputInfo info;
    uint32_t connectorId = 0;
    uint32_t encoderId = 0;
    uint32_t crtcId = 0;
    drmModeModeInfo mode {};
};

class KmsBackend : public DisplayBackend {
public:
    struct Options {
        Options() = default;
        QString cardPath = QStringLiteral("/dev/dri/card0");
        QString renderPath = QStringLiteral("/dev/dri/renderD128");
        bool allowModeset = false;
    };

    explicit KmsBackend(Options options);
    ~KmsBackend() override;

    bool initialize() override;
    void shutdown() override;
    QList<OutputInfo> outputs() const override;
    bool setMode(const QString &name, const QSize &mode) override;
    bool setDpms(const QString &name, bool on) override;
    void setCursor(const QImage &image, const QPoint &hotspot) override;
    Buffer *createBuffer(const QSize &size) override;
    void present(OutputInfo *output, Buffer *buffer) override;

    bool modesetAllowed() const;
    bool drmMaster() const;

private:
    bool acquireMasterIfAllowed();
    KmsOutput *findOutput(const QString &name);
    bool addFramebuffer(KmsBuffer *buffer);
    bool presentWithModeset(KmsOutput *output, KmsBuffer *buffer);

    Options m_options;
    int m_fd = -1;
    int m_renderFd = -1;
    gbm_device *m_gbm = nullptr;
    bool m_master = false;
    QList<KmsOutput> m_outputs;
    QHash<const OutputInfo *, KmsOutput *> m_index;
    uint32_t m_currentFb = 0;
    KmsBuffer *m_currentBuffer = nullptr;
};

} // namespace Cutefish
