#pragma once

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include <wayland-server-core.h>

namespace Cutefish {

// 剪贴板选择项：保存 MIME 与字节载荷。主选择和普通剪贴板分别持有独立 store。
struct SelectionItem {
    QString mimeType;
    QByteArray data;
};

class SelectionStore {
public:
    void clear();
    void setData(const QString &mimeType, const QByteArray &data);
    QByteArray data(const QString &mimeType) const;
    QStringList mimeTypes() const;
    bool isEmpty() const;

private:
    QHash<QString, QByteArray> m_items;
};

// DataDeviceManager：wl_data_device_manager + wp_primary_selection_v1 的最小实现。
// 职责：source/device/offer 资源、selection 缓存、receive 回放。
// 安全边界：Lock 状态下由调用方禁止新建 selection/导出；普通应用只能通过
// 标准协议访问，不能访问 cutefish_core_v1。
class DataDeviceManager : public QObject {
    Q_OBJECT
public:
    explicit DataDeviceManager(QObject *parent = nullptr);

    bool registerDisplay(wl_display *display);

    SelectionStore *clipboardStore();
    SelectionStore *primaryStore();

    QList<int> pendingReadFds() const;
    void dispatchPendingRead(int fd);
    void clearSelectionForClient(wl_client *client);

    // 供 C 回调使用。字段定义为公开内部结构，仅 core 主线程访问。
    struct SourceData {
        wl_resource *resource = nullptr;
        DataDeviceManager *manager = nullptr;
        bool primary = false;
        QStringList mimeTypes;
        size_t nextMimeIndex = 0;
    };
    struct DeviceData {
        wl_resource *resource = nullptr;
        DataDeviceManager *manager = nullptr;
        bool primary = false;
        SourceData *currentSource = nullptr;
    };
    struct OfferData {
        wl_resource *resource = nullptr;
        DataDeviceManager *manager = nullptr;
        bool primary = false;
        QStringList mimeTypes;
    };

    SourceData *createSourceData(wl_resource *source, bool primary);
    DeviceData *createDeviceData(wl_resource *device, bool primary);
    OfferData *createOfferData(wl_resource *offer, bool primary);
    void setSelectionSource(DeviceData *device, SourceData *source);
    void startSourceRead(SourceData *source);
    void offerReceive(OfferData *offer, const QString &mimeType, int fd);

private:
    struct PendingRead {
        SourceData *source = nullptr;
        QString mimeType;
        int readFd = -1;
    };

    void finishPendingRead(PendingRead *pending, const QByteArray &data);
    void removePendingRead(int fd);
    void sendSelectionOffer(DeviceData *device, SourceData *source);

    SelectionStore m_clipboard;
    SelectionStore m_primary;
    QHash<int, PendingRead *> m_pendingReads;
};

} // namespace Cutefish
