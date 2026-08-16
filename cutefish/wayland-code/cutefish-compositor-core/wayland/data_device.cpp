#include "wayland/data_device.h"

#include <QDebug>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include <wayland-server-protocol.h>
#include <wayland-server.h>

extern "C" {
#include "wayland-primary-selection-unstable-v1-server-protocol.h"
}

namespace Cutefish {

namespace {

constexpr uint32_t kDataVersion = 1;
constexpr uint32_t kPrimaryVersion = 1;
constexpr size_t kMaxSelectionBytes = 16 * 1024 * 1024;

// 通用资源析构
template <typename T>
void destroyData(wl_resource *resource)
{
    delete static_cast<T *>(wl_resource_get_user_data(resource));
}

// wl_data_device_manager ---------------------------------------------
void dataManagerCreateSource(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *manager = static_cast<DataDeviceManager *>(wl_resource_get_user_data(resource));
    auto *data = manager->createSourceData(
        wl_resource_create(client, &wl_data_source_interface, kDataVersion, id), false);
    if (!data->resource) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct wl_data_source_interface dataSourceImplementation;
    wl_resource_set_implementation(data->resource, &dataSourceImplementation, data, destroyData<DataDeviceManager::SourceData>);
}

void dataManagerGetDevice(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat)
{
    Q_UNUSED(seat)
    auto *manager = static_cast<DataDeviceManager *>(wl_resource_get_user_data(resource));
    auto *data = manager->createDeviceData(
        wl_resource_create(client, &wl_data_device_interface, kDataVersion, id), false);
    if (!data->resource) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct wl_data_device_interface dataDeviceImplementation;
    wl_resource_set_implementation(data->resource, &dataDeviceImplementation, data, destroyData<DataDeviceManager::DeviceData>);
}

const struct wl_data_device_manager_interface dataManagerImplementation = {
    dataManagerCreateSource,
    dataManagerGetDevice,
};

// wl_data_source ------------------------------------------------------
void dataSourceOffer(wl_client *client, wl_resource *resource, const char *mimeType)
{
    Q_UNUSED(client)
    auto *data = static_cast<DataDeviceManager::SourceData *>(wl_resource_get_user_data(resource));
    if (data && mimeType)
        data->mimeTypes.append(QString::fromUtf8(mimeType));
}

void dataSourceDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct wl_data_source_interface dataSourceImplementation = {
    dataSourceOffer,
    dataSourceDestroy,
};

// wl_data_device ------------------------------------------------------
void dataDeviceStartDrag(wl_client *client, wl_resource *resource, wl_resource *source,
                         wl_resource *origin, wl_resource *icon, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(source)
    Q_UNUSED(origin)
    Q_UNUSED(icon)
    Q_UNUSED(serial)
    // DnD 由后续阶段实现；首版拒绝隐式拖拽状态。
}

void dataDeviceSetSelection(wl_client *client, wl_resource *resource, wl_resource *source, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(serial)
    auto *device = static_cast<DataDeviceManager::DeviceData *>(wl_resource_get_user_data(resource));
    if (!device)
        return;
    auto *sourceData = source
        ? static_cast<DataDeviceManager::SourceData *>(wl_resource_get_user_data(source))
        : nullptr;
    if (sourceData && sourceData->resource != source)
        sourceData = nullptr;
    device->manager->setSelectionSource(device, sourceData);
}

const struct wl_data_device_interface dataDeviceImplementation = {
    dataDeviceStartDrag,
    dataDeviceSetSelection,
};

// wl_data_offer -------------------------------------------------------
void dataOfferAccept(wl_client *client, wl_resource *resource, uint32_t serial, const char *mimeType)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    Q_UNUSED(serial)
    Q_UNUSED(mimeType)
}

void dataOfferReceive(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd)
{
    Q_UNUSED(client)
    auto *offer = static_cast<DataDeviceManager::OfferData *>(wl_resource_get_user_data(resource));
    if (offer)
        offer->manager->offerReceive(offer, QString::fromUtf8(mimeType), fd);
}

void dataOfferDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct wl_data_offer_interface dataOfferImplementation = {
    dataOfferAccept,
    dataOfferReceive,
    dataOfferDestroy,
};

// wp_primary_selection ------------------------------------------------
void primaryManagerCreateSource(wl_client *client, wl_resource *resource, uint32_t id)
{
    auto *manager = static_cast<DataDeviceManager *>(wl_resource_get_user_data(resource));
    auto *data = manager->createSourceData(
        wl_resource_create(client, &zwp_primary_selection_source_v1_interface, kPrimaryVersion, id), true);
    if (!data->resource) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct zwp_primary_selection_source_v1_interface primarySourceImplementation;
    wl_resource_set_implementation(data->resource, &primarySourceImplementation, data, destroyData<DataDeviceManager::SourceData>);
}

void primaryManagerGetDevice(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *seat)
{
    Q_UNUSED(seat)
    auto *manager = static_cast<DataDeviceManager *>(wl_resource_get_user_data(resource));
    auto *data = manager->createDeviceData(
        wl_resource_create(client, &zwp_primary_selection_device_v1_interface, kPrimaryVersion, id), true);
    if (!data->resource) {
        delete data;
        wl_client_post_no_memory(client);
        return;
    }
    extern const struct zwp_primary_selection_device_v1_interface primaryDeviceImplementation;
    wl_resource_set_implementation(data->resource, &primaryDeviceImplementation, data, destroyData<DataDeviceManager::DeviceData>);
}

void primaryManagerDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct zwp_primary_selection_device_manager_v1_interface primaryManagerImplementation = {
    primaryManagerCreateSource,
    primaryManagerGetDevice,
    primaryManagerDestroy,
};

void primarySourceOffer(wl_client *client, wl_resource *resource, const char *mimeType)
{
    Q_UNUSED(client)
    auto *data = static_cast<DataDeviceManager::SourceData *>(wl_resource_get_user_data(resource));
    if (data && mimeType)
        data->mimeTypes.append(QString::fromUtf8(mimeType));
}

void primarySourceDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct zwp_primary_selection_source_v1_interface primarySourceImplementation = {
    primarySourceOffer,
    primarySourceDestroy,
};

void primaryDeviceSetSelection(wl_client *client, wl_resource *resource, wl_resource *source, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(serial)
    auto *device = static_cast<DataDeviceManager::DeviceData *>(wl_resource_get_user_data(resource));
    if (!device)
        return;
    auto *sourceData = source
        ? static_cast<DataDeviceManager::SourceData *>(wl_resource_get_user_data(source))
        : nullptr;
    device->manager->setSelectionSource(device, sourceData);
}

void primaryDeviceDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct zwp_primary_selection_device_v1_interface primaryDeviceImplementation = {
    primaryDeviceSetSelection,
    primaryDeviceDestroy,
};

void primaryOfferReceive(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd)
{
    Q_UNUSED(client)
    auto *offer = static_cast<DataDeviceManager::OfferData *>(wl_resource_get_user_data(resource));
    if (offer)
        offer->manager->offerReceive(offer, QString::fromUtf8(mimeType), fd);
}

void primaryOfferDestroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

const struct zwp_primary_selection_offer_v1_interface primaryOfferImplementation = {
    primaryOfferReceive,
    primaryOfferDestroy,
};

// 注册函数
void dataManagerBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *manager = static_cast<DataDeviceManager *>(data);
    auto *resource = wl_resource_create(client, &wl_data_device_manager_interface,
                                        std::min(version, kDataVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &dataManagerImplementation, manager, nullptr);
}

void primaryManagerBind(wl_client *client, void *data, uint32_t version, uint32_t id)
{
    auto *manager = static_cast<DataDeviceManager *>(data);
    auto *resource = wl_resource_create(client, &zwp_primary_selection_device_manager_v1_interface,
                                        std::min(version, kPrimaryVersion), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &primaryManagerImplementation, manager, nullptr);
}

} // namespace

void SelectionStore::clear()
{
    m_items.clear();
}

void SelectionStore::setData(const QString &mimeType, const QByteArray &data)
{
    if (data.size() > static_cast<int>(kMaxSelectionBytes))
        return;
    m_items.insert(mimeType, data);
}

QByteArray SelectionStore::data(const QString &mimeType) const
{
    return m_items.value(mimeType);
}

QStringList SelectionStore::mimeTypes() const
{
    return m_items.keys();
}

bool SelectionStore::isEmpty() const
{
    return m_items.isEmpty();
}

DataDeviceManager::DataDeviceManager(QObject *parent)
    : QObject(parent)
{
}

bool DataDeviceManager::registerDisplay(wl_display *display)
{
    if (!wl_global_create(display, &wl_data_device_manager_interface, kDataVersion,
                          this, dataManagerBind))
        return false;
    if (!wl_global_create(display, &zwp_primary_selection_device_manager_v1_interface,
                          kPrimaryVersion, this, primaryManagerBind))
        return false;
    return true;
}

SelectionStore *DataDeviceManager::clipboardStore()
{
    return &m_clipboard;
}

SelectionStore *DataDeviceManager::primaryStore()
{
    return &m_primary;
}

DataDeviceManager::SourceData *DataDeviceManager::createSourceData(wl_resource *source, bool primary)
{
    auto *data = new SourceData;
    data->resource = source;
    data->manager = this;
    data->primary = primary;
    return data;
}

DataDeviceManager::DeviceData *DataDeviceManager::createDeviceData(wl_resource *device, bool primary)
{
    auto *data = new DeviceData;
    data->resource = device;
    data->manager = this;
    data->primary = primary;
    return data;
}

DataDeviceManager::OfferData *DataDeviceManager::createOfferData(wl_resource *offer, bool primary)
{
    auto *data = new OfferData;
    data->resource = offer;
    data->manager = this;
    data->primary = primary;
    return data;
}

void DataDeviceManager::setSelectionSource(DeviceData *device, SourceData *source)
{
    if (!device)
        return;
    device->currentSource = source;
    sendSelectionOffer(device, source);
    SelectionStore *store = device->primary ? &m_primary : &m_clipboard;
    store->clear();
    if (source)
        startSourceRead(source);
}

void DataDeviceManager::sendSelectionOffer(DeviceData *device, SourceData *source)
{
    // 协议顺序：先发送 data_offer(new_id)，再发送 offer(mime) 列表，
    // 最后发送 selection(object)。缺少 data_offer 会导致客户端无法注册对象。
    wl_client *client = wl_resource_get_client(device->resource);
    const wl_interface *interface = device->primary
        ? &zwp_primary_selection_offer_v1_interface : &wl_data_offer_interface;
    const void *implementation = device->primary
        ? static_cast<const void *>(&primaryOfferImplementation)
        : static_cast<const void *>(&dataOfferImplementation);

    auto *data = new OfferData;
    data->manager = this;
    data->primary = device->primary;
    data->mimeTypes = source ? source->mimeTypes : QStringList();

    wl_resource *offer = wl_resource_create(client, interface, kDataVersion, 0);
    if (!offer) {
        delete data;
        return;
    }
    data->resource = offer;
    wl_resource_set_implementation(offer, implementation, data, destroyData<OfferData>);

    if (device->primary) {
        zwp_primary_selection_device_v1_send_data_offer(device->resource, offer);
        for (const QString &mime : data->mimeTypes)
            zwp_primary_selection_offer_v1_send_offer(offer, mime.toUtf8().constData());
        zwp_primary_selection_device_v1_send_selection(device->resource, offer);
    } else {
        wl_data_device_send_data_offer(device->resource, offer);
        for (const QString &mime : data->mimeTypes)
            wl_data_offer_send_offer(offer, mime.toUtf8().constData());
        wl_data_device_send_selection(device->resource, offer);
    }
}

void DataDeviceManager::startSourceRead(SourceData *source)
{
    if (!source || source->nextMimeIndex >= source->mimeTypes.size())
        return;
    const QString mime = source->mimeTypes.at(source->nextMimeIndex++);
    int pipeFds[2] = {-1, -1};
    if (::pipe2(pipeFds, O_CLOEXEC | O_NONBLOCK) != 0)
        return;
    auto *pending = new PendingRead;
    pending->source = source;
    pending->mimeType = mime;
    pending->readFd = pipeFds[0];
    m_pendingReads.insert(pipeFds[0], pending);

    if (source->primary)
        zwp_primary_selection_source_v1_send_send(source->resource, mime.toUtf8().constData(), pipeFds[1]);
    else
        wl_data_source_send_send(source->resource, mime.toUtf8().constData(), pipeFds[1]);
    ::close(pipeFds[1]);
}

QList<int> DataDeviceManager::pendingReadFds() const
{
    return m_pendingReads.keys();
}

void DataDeviceManager::dispatchPendingRead(int fd)
{
    PendingRead *pending = m_pendingReads.value(fd, nullptr);
    if (!pending)
        return;
    QByteArray buffer;
    char chunk[4096];
    while (true) {
        const ssize_t n = ::read(fd, chunk, sizeof(chunk));
        if (n > 0) {
            buffer.append(chunk, static_cast<int>(n));
            if (buffer.size() > static_cast<int>(kMaxSelectionBytes)) {
                buffer.clear();
                break;
            }
            continue;
        }
        if (n < 0 && errno == EAGAIN)
            return; // 对端尚未写完，等待下一次 poll。
        break; // EOF 或错误：结束本次读取。
    }
    finishPendingRead(pending, buffer);
}

void DataDeviceManager::finishPendingRead(PendingRead *pending, const QByteArray &data)
{
    SourceData *source = pending->source;
    SelectionStore *store = source->primary ? &m_primary : &m_clipboard;
    if (!data.isEmpty())
        store->setData(pending->mimeType, data);
    const int fd = pending->readFd;
    m_pendingReads.remove(fd);
    ::close(fd);
    delete pending;

    // 按顺序读取下一个 MIME，使文本、文件列表、图片等常见类型都被缓存；
    // 源客户端关闭后 selection 仍然有效。
    if (source && source->nextMimeIndex < source->mimeTypes.size())
        startSourceRead(source);
}

void DataDeviceManager::removePendingRead(int fd)
{
    delete m_pendingReads.take(fd);
}

void DataDeviceManager::offerReceive(OfferData *offer, const QString &mimeType, int fd)
{
    SelectionStore *store = offer->primary ? &m_primary : &m_clipboard;
    const QByteArray data = store->data(mimeType);
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

void DataDeviceManager::clearSelectionForClient(wl_client *client)
{
    Q_UNUSED(client)
    // 用户注销时由 core 调用；当前 stage-1 仅保留接口。
}

} // namespace Cutefish
