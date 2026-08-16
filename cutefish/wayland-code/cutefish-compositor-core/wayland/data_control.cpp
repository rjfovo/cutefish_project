#include "wayland/data_control.h"

#include "wayland/data_device.h"
#include "wlr-data-control-unstable-v1-server-protocol.h"

#include <QDebug>

#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

namespace Cutefish {

namespace {

constexpr uint32_t kDataControlVersion = 1;

template <typename T>
void destroyData(wl_resource *resource)
{
    delete static_cast<T *>(wl_resource_get_user_data(resource));
}

void managerCreateSource(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *manager = static_cast<DataControlManager *>(wl_resource_get_user_data(resource));
    auto *data = manager->createSource(
        wl_resource_create(client, &zwlr_data_control_source_v1_interface, kDataControlVersion, id));
    if (!data->resource) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct zwlr_data_control_source_v1_interface dataControlSourceImpl;
    wl_resource_set_implementation(data->resource, &dataControlSourceImpl, data, destroyData<DataControlManager::SourceData>);
}

void managerGetDevice(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat)
{
    Q_UNUSED(seat)
    auto *manager = static_cast<DataControlManager *>(wl_resource_get_user_data(resource));
    auto *data = manager->createDevice(
        wl_resource_create(client, &zwlr_data_control_device_v1_interface, kDataControlVersion, id));
    if (!data->resource) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct zwlr_data_control_device_v1_interface dataControlDeviceImpl;
    wl_resource_set_implementation(data->resource, &dataControlDeviceImpl, data, destroyData<DataControlManager::DeviceData>);
}

const struct zwlr_data_control_manager_v1_interface managerImpl = {
    managerCreateSource,
    managerGetDevice,
};

void sourceOffer(wl_client *client, wl_resource *resource, const char *mime)
{
    Q_UNUSED(client)
    auto *data = static_cast<DataControlManager::SourceData *>(wl_resource_get_user_data(resource));
    if (data && mime)
        data->mimeTypes.append(QString::fromUtf8(mime));
}

void sourceDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct zwlr_data_control_source_v1_interface dataControlSourceImpl = {
    sourceOffer,
    sourceDestroy,
};

void deviceSetSelection(wl_client *client, wl_resource *resource, wl_resource *source)
{
    Q_UNUSED(client)
    auto *device = static_cast<DataControlManager::DeviceData *>(wl_resource_get_user_data(resource));
    if (!device)
        return;
    auto *sourceData = source
        ? static_cast<DataControlManager::SourceData *>(wl_resource_get_user_data(source))
        : nullptr;
    device->manager->setSelection(device, sourceData);
}

void deviceDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct zwlr_data_control_device_v1_interface dataControlDeviceImpl = {
    deviceSetSelection,
    deviceDestroy,
};

void offerReceive(wl_client *client, wl_resource *resource, const char *mime, int32_t fd)
{
    Q_UNUSED(client)
    auto *offer = static_cast<DataControlManager::OfferData *>(wl_resource_get_user_data(resource));
    if (offer)
        offer->manager->receive(offer, QString::fromUtf8(mime), fd);
}

void offerDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct zwlr_data_control_offer_v1_interface dataControlOfferImpl = {
    offerReceive,
    offerDestroy,
};

void managerBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *manager = static_cast<DataControlManager *>(data);
    auto *resource = wl_resource_create(client, &zwlr_data_control_manager_v1_interface,
                                        std::min(version, kDataControlVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &managerImpl, manager, nullptr);
}

} // namespace

DataControlManager::DataControlManager(DataDeviceManager *stores, QObject *parent)
    : QObject(parent)
    , m_stores(stores)
{
}

bool DataControlManager::registerDisplay(wl_display *display)
{
    return wl_global_create(display, &zwlr_data_control_manager_v1_interface,
                            kDataControlVersion, this, managerBind) != nullptr;
}

DataControlManager::SourceData *DataControlManager::createSource(wl_resource *source)
{
    auto *data = new SourceData;
    data->resource = source;
    data->manager = this;
    return data;
}

DataControlManager::DeviceData *DataControlManager::createDevice(wl_resource *device)
{
    auto *data = new DeviceData;
    data->resource = device;
    data->manager = this;
    return data;
}

DataControlManager::OfferData *DataControlManager::createOffer(wl_resource *offer)
{
    auto *data = new OfferData;
    data->resource = offer;
    data->manager = this;
    return data;
}

void DataControlManager::setSelection(DeviceData *device, SourceData *source)
{
    device->currentSource = source;
    wl_client *client = wl_resource_get_client(device->resource);
    auto *offerData = new OfferData;
    offerData->manager = this;
    offerData->mimeTypes = source ? source->mimeTypes : QStringList();
    wl_resource *offer = wl_resource_create(client, &zwlr_data_control_offer_v1_interface,
                                            kDataControlVersion, 0);
    if (!offer) {
        delete offerData;
        return;
    }
    offerData->resource = offer;
    wl_resource_set_implementation(offer, &dataControlOfferImpl, offerData,
                                   destroyData<DataControlManager::OfferData>);
    zwlr_data_control_device_v1_send_data_offer(device->resource, offer);
    for (const QString &mime : offerData->mimeTypes)
        zwlr_data_control_offer_v1_send_offer(offer, mime.toUtf8().constData());
    zwlr_data_control_device_v1_send_selection(device->resource, offer);

    m_stores->clipboardStore()->clear();
    if (source)
        startRead(source);
}

void DataControlManager::startRead(SourceData *source)
{
    if (!source || source->nextMime >= source->mimeTypes.size())
        return;
    const QString mime = source->mimeTypes.at(source->nextMime++);
    int fds[2] = {-1, -1};
    if (::pipe2(fds, O_CLOEXEC | O_NONBLOCK) != 0)
        return;
    auto *pending = new PendingRead;
    pending->source = source;
    pending->mimeType = mime;
    pending->readFd = fds[0];
    m_pending.insert(fds[0], pending);
    zwlr_data_control_source_v1_send_send(source->resource, mime.toUtf8().constData(), fds[1]);
    ::close(fds[1]);
}

QList<int> DataControlManager::pendingReadFds() const
{
    return m_pending.keys();
}

void DataControlManager::dispatchPendingRead(int fd)
{
    PendingRead *pending = m_pending.value(fd, nullptr);
    if (!pending)
        return;
    QByteArray buffer;
    char chunk[4096];
    while (true) {
        const ssize_t n = ::read(fd, chunk, sizeof(chunk));
        if (n > 0) {
            buffer.append(chunk, static_cast<int>(n));
            if (buffer.size() > 16 * 1024 * 1024) {
                buffer.clear();
                break;
            }
            continue;
        }
        if (n < 0 && errno == EAGAIN)
            return;
        break;
    }
    finishRead(pending, buffer);
}

void DataControlManager::finishRead(PendingRead *pending, const QByteArray &data)
{
    SourceData *source = pending->source;
    if (!data.isEmpty())
        m_stores->clipboardStore()->setData(pending->mimeType, data);
    const int fd = pending->readFd;
    m_pending.remove(fd);
    ::close(fd);
    delete pending;
    if (source && source->nextMime < source->mimeTypes.size())
        startRead(source);
}

void DataControlManager::receive(OfferData *offer, const QString &mimeType, int fd)
{
    Q_UNUSED(offer)
    const QByteArray data = m_stores->clipboardStore()->data(mimeType);
    size_t written = 0;
    while (written < static_cast<size_t>(data.size())) {
        const ssize_t n = ::write(fd, data.constData() + written, data.size() - written);
        if (n < 0 && errno == EAGAIN)
            continue;
        if (n <= 0)
            break;
        written += static_cast<size_t>(n);
    }
    ::close(fd);
}

} // namespace Cutefish
