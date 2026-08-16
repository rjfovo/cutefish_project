#include "backend/kms_backend.h"

#include <QDebug>
#include <QFile>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <drm_fourcc.h>
#include <xf86drm.h>

namespace Cutefish {

KmsBuffer::KmsBuffer(gbm_bo *bo, const QSize &size)
    : bo(bo)
{
    if (bo) {
        dmabufFd = gbm_bo_get_fd(bo);
        stride = gbm_bo_get_stride(bo);
        handle = gbm_bo_get_handle(bo).u32;
    }
}

KmsBuffer::~KmsBuffer()
{
    if (dmabufFd >= 0)
        ::close(dmabufFd);
    if (bo)
        gbm_bo_destroy(bo);
}

QSize KmsBuffer::size() const
{
    if (!bo)
        return {};
    return QSize(static_cast<int>(gbm_bo_get_width(bo)),
                 static_cast<int>(gbm_bo_get_height(bo)));
}

KmsBackend::KmsBackend(Options options)
    : m_options(std::move(options))
{
}

KmsBackend::~KmsBackend()
{
    shutdown();
}

bool KmsBackend::initialize()
{
    m_fd = ::open(m_options.cardPath.toUtf8().constData(), O_RDWR | O_CLOEXEC);
    if (m_fd < 0) {
        qWarning() << "KmsBackend: cannot open" << m_options.cardPath << ":" << ::strerror(errno);
        return false;
    }

    m_renderFd = ::open(m_options.renderPath.toUtf8().constData(), O_RDWR | O_CLOEXEC);
    if (m_renderFd >= 0)
        m_gbm = gbm_create_device(m_renderFd);
    if (!m_gbm)
        qWarning() << "KmsBackend: GBM device unavailable, buffer creation disabled";

    uint64_t cap = 0;
    if (drmGetCap(m_fd, DRM_CAP_DUMB_BUFFER, &cap) == 0)
        qInfo() << "KmsBackend: dumb-buffer supported" << cap;

    drmModeRes *resources = drmModeGetResources(m_fd);
    if (!resources) {
        qWarning() << "KmsBackend: drmModeGetResources failed";
        return false;
    }

    for (int i = 0; i < resources->count_connectors; ++i) {
        drmModeConnector *connector = drmModeGetConnector(m_fd, resources->connectors[i]);
        if (!connector)
            continue;

        if (connector->connection != DRM_MODE_CONNECTED || connector->count_modes == 0) {
            drmModeFreeConnector(connector);
            continue;
        }

        KmsOutput output;
        output.info.name = QStringLiteral("CONNECTOR-%1").arg(connector->connector_id);
        output.info.connected = true;
        output.info.scale = 1;
        output.info.transform = 0;
        for (int m = 0; m < connector->count_modes && m < 32; ++m) {
            const drmModeModeInfo &mode = connector->modes[m];
            output.info.modes.append(QSize(mode.hdisplay, mode.vdisplay));
            if (m == 0)
                output.mode = mode;
        }
        output.info.currentMode = output.info.modes.value(0);
        output.connectorId = connector->connector_id;
        output.encoderId = connector->encoder_id;
        if (output.encoderId) {
            drmModeEncoder *encoder = drmModeGetEncoder(m_fd, output.encoderId);
            if (encoder) {
                output.crtcId = encoder->crtc_id;
                drmModeFreeEncoder(encoder);
            }
        }
        if (!output.crtcId && resources->count_crtcs > 0)
            output.crtcId = resources->crtcs[0];

        qInfo() << "KmsBackend output" << output.info.name
                << "connector" << output.connectorId
                << "mode" << output.info.currentMode
                << "modes" << output.info.modes.size();

        m_outputs.append(output);
        drmModeFreeConnector(connector);
    }
    drmModeFreeResources(resources);

    acquireMasterIfAllowed();
    return true;
}

void KmsBackend::shutdown()
{
    delete m_currentBuffer;
    m_currentBuffer = nullptr;
    m_index.clear();
    m_outputs.clear();
    if (m_gbm)
        gbm_device_destroy(m_gbm);
    m_gbm = nullptr;
    if (m_renderFd >= 0)
        ::close(m_renderFd);
    m_renderFd = -1;
    if (m_fd >= 0)
        ::close(m_fd);
    m_fd = -1;
    m_master = false;
}

bool KmsBackend::acquireMasterIfAllowed()
{
    if (!m_options.allowModeset || m_fd < 0)
        return false;
    if (drmSetMaster(m_fd) == 0) {
        m_master = true;
        qInfo() << "KmsBackend: DRM master acquired";
    } else {
        qWarning() << "KmsBackend: drmSetMaster failed" << ::strerror(errno);
    }
    return m_master;
}

QList<OutputInfo> KmsBackend::outputs() const
{
    QList<OutputInfo> result;
    result.reserve(m_outputs.size());
    for (const KmsOutput &output : m_outputs)
        result.append(output.info);
    return result;
}

KmsOutput *KmsBackend::findOutput(const QString &name)
{
    for (KmsOutput &output : m_outputs) {
        if (output.info.name == name)
            return &output;
    }
    return nullptr;
}

bool KmsBackend::setMode(const QString &name, const QSize &mode)
{
    KmsOutput *output = findOutput(name);
    if (!output) {
        qWarning() << "KmsBackend: unknown output" << name;
        return false;
    }
    const int index = output->info.modes.indexOf(mode);
    if (index < 0)
        return false;
    output->info.currentMode = mode;
    qInfo() << "KmsBackend: mode selected (modeset active:" << m_master << ")";
    return true;
}

bool KmsBackend::setDpms(const QString &name, bool on)
{
    KmsOutput *output = findOutput(name);
    if (!output || m_fd < 0)
        return false;
    drmModeConnector *connector = drmModeGetConnector(m_fd, output->connectorId);
    if (!connector)
        return false;
    bool ok = false;
    for (int i = 0; i < connector->count_props; ++i) {
        drmModePropertyRes *prop = drmModeGetProperty(m_fd, connector->props[i]);
        if (!prop)
            continue;
        if (qstrcmp(prop->name, "DPMS") == 0) {
            const int value = on ? DRM_MODE_DPMS_ON : DRM_MODE_DPMS_OFF;
            ok = drmModeConnectorSetProperty(m_fd, connector->connector_id,
                                             prop->prop_id, value) == 0;
        }
        drmModeFreeProperty(prop);
        if (ok)
            break;
    }
    drmModeFreeConnector(connector);
    return ok;
}

void KmsBackend::setCursor(const QImage &image, const QPoint &hotspot)
{
    Q_UNUSED(image)
    Q_UNUSED(hotspot)
    // Hardware cursor arrives with the stage-1 renderer completion.
}

Buffer *KmsBackend::createBuffer(const QSize &size)
{
    if (!m_gbm || !size.isValid())
        return nullptr;
    gbm_bo *bo = gbm_bo_create(m_gbm, size.width(), size.height(),
                               GBM_FORMAT_XRGB8888,
                               GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT);
    if (!bo) {
        qWarning() << "KmsBackend: gbm_bo_create failed for" << size;
        return nullptr;
    }
    return new KmsBuffer(bo, size);
}

bool KmsBackend::addFramebuffer(KmsBuffer *buffer)
{
    if (!buffer || !buffer->bo || m_fd < 0)
        return false;
    uint32_t fb = 0;
    const uint32_t handles[4] = {buffer->handle, 0, 0, 0};
    const uint32_t strides[4] = {buffer->stride, 0, 0, 0};
    const uint32_t offsets[4] = {0, 0, 0, 0};
    if (drmModeAddFB2(m_fd,
                      static_cast<uint32_t>(gbm_bo_get_width(buffer->bo)),
                      static_cast<uint32_t>(gbm_bo_get_height(buffer->bo)),
                      GBM_FORMAT_XRGB8888, handles, strides, offsets, &fb, 0) != 0) {
        qWarning() << "KmsBackend: drmModeAddFB2 failed" << ::strerror(errno);
        return false;
    }
    if (m_currentFb)
        drmModeRmFB(m_fd, m_currentFb);
    m_currentFb = fb;
    return true;
}

bool KmsBackend::presentWithModeset(KmsOutput *output, KmsBuffer *buffer)
{
    if (!m_master || !m_options.allowModeset || !output || !buffer)
        return false;
    if (!addFramebuffer(buffer))
        return false;
    if (drmModeSetCrtc(m_fd, output->crtcId, m_currentFb, 0, 0,
                       &output->connectorId, 1, &output->mode) != 0) {
        qWarning() << "KmsBackend: drmModeSetCrtc failed" << ::strerror(errno);
        return false;
    }
    return true;
}

void KmsBackend::present(OutputInfo *outputInfo, Buffer *buffer)
{
    if (!buffer || !outputInfo)
        return;
    auto *kmsBuffer = dynamic_cast<KmsBuffer *>(buffer);
    if (!kmsBuffer)
        return;
    KmsOutput *output = nullptr;
    for (KmsOutput &candidate : m_outputs) {
        if (&candidate.info == outputInfo) {
            output = &candidate;
            break;
        }
    }
    if (!output)
        return;

    if (m_master && m_options.allowModeset) {
        presentWithModeset(output, kmsBuffer);
        return;
    }

    // Safe probe mode: no master/modeset/page flip. Buffer creation and
    // output enumeration are the only privileged operations performed.
    qInfo() << "KmsBackend: present suppressed (modeset not authorized)";
}

bool KmsBackend::modesetAllowed() const
{
    return m_options.allowModeset;
}

bool KmsBackend::drmMaster() const
{
    return m_master;
}

} // namespace Cutefish
