/*
 * ShellCoreClient：Shell 到 core 的 cutefish_core_v1 v2 客户端。
 * 安全边界：只连接 shell 专用 socket；普通应用不得使用本类。
 * 线程边界：仅在 Qt 主线程使用；Wayland fd 由 QSocketNotifier 驱动。
 */
#pragma once

#include "cutefish-core-v1-client-protocol.h"

#include <QObject>
#include <QSocketNotifier>
#include <QString>
#include <QVector>

#include <wayland-client.h>

struct wl_registry;

class ShellCoreClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)
public:
    explicit ShellCoreClient(QObject *parent = nullptr);
    ~ShellCoreClient() override;

    bool connectToCore(const QString &socketName);
    void disconnectFromCore();

    bool connected() const;
    int mode() const;
    void setMode(int mode);

    Q_INVOKABLE void requestGetOutputs();
    Q_INVOKABLE void requestGetWindows();
    void syncRequestGetOutputs();
    Q_INVOKABLE void setOutputConfig(const QString &name, int width, int height, int scale, int transform);
    Q_INVOKABLE void requestActivate(const QString &appId);
    Q_INVOKABLE void requestClose(const QString &appId);

    struct Output {
        QString name;
        int width = 0;
        int height = 0;
        int scale = 1;
        int transform = 0;
        bool connected = false;
    };
    struct WindowInfo {
        uint32_t id = 0;
        QString appId;
        QString title;
        uint32_t state = 0;
        bool activated = false;
    };

    QVector<Output> outputs() const;
    QVector<WindowInfo> windows() const;
    QObject *outputModel() const;
    QObject *windowModel() const;

    void setCoreProxy(cutefish_core_v1 *core);
    void updateOutput(const Output &output);
    void updateWindow(const WindowInfo &window);
    void updateWindowState(uint32_t id, uint32_t state, bool activated);
    void removeWindow(uint32_t id);

signals:
    void connectedChanged();
    void modeChanged();
    void outputsChanged();
    void windowsChanged();

private:
    void setupRegistry();
    void dispatch();
    void handleCoreEvent();

    wl_display *m_display = nullptr;
    wl_registry *m_registry = nullptr;
    cutefish_core_v1 *m_core = nullptr;
    QSocketNotifier *m_notifier = nullptr;
    int m_mode = 1;
    QVector<Output> m_outputs;
    QVector<WindowInfo> m_windows;
    class OutputListModel *m_outputModel = nullptr;
    class WindowListModel *m_windowModel = nullptr;
};
