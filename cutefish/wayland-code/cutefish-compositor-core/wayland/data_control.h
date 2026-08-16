#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

#include <wayland-server-core.h>

namespace Cutefish {

class DataDeviceManager;

class DataControlManager : public QObject {
    Q_OBJECT
public:
    explicit DataControlManager(DataDeviceManager *stores, QObject *parent = nullptr);

    bool registerDisplay(wl_display *display);

    QList<int> pendingReadFds() const;
    void dispatchPendingRead(int fd);

    struct SourceData {
        wl_resource *resource = nullptr;
        DataControlManager *manager = nullptr;
        QStringList mimeTypes;
        size_t nextMime = 0;
    };
    struct DeviceData {
        wl_resource *resource = nullptr;
        DataControlManager *manager = nullptr;
        SourceData *currentSource = nullptr;
    };
    struct OfferData {
        wl_resource *resource = nullptr;
        DataControlManager *manager = nullptr;
        QStringList mimeTypes;
    };

    SourceData *createSource(wl_resource *source);
    DeviceData *createDevice(wl_resource *device);
    OfferData *createOffer(wl_resource *offer);
    void setSelection(DeviceData *device, SourceData *source);
    void receive(OfferData *offer, const QString &mimeType, int fd);

private:
    struct PendingRead {
        SourceData *source = nullptr;
        QString mimeType;
        int readFd = -1;
    };
    void finishRead(PendingRead *pending, const QByteArray &data);
    void startRead(SourceData *source);

    DataDeviceManager *m_stores = nullptr;
    QHash<int, PendingRead *> m_pending;
};

} // namespace Cutefish
