/*
 * Window：单个 xdg_toplevel 窗口模型。
 * 生命周期：xdg_surface.get_toplevel 创建；client 断开或 destroy 时销毁。
 * 线程边界：仅 core 主线程访问。
 */
#pragma once

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QString>

#include <wayland-server-core.h>

namespace Cutefish {

class Window : public QObject {
    Q_OBJECT
public:
    enum class State {
        Normal = 0,
        Maximized,
        Fullscreen,
        Minimized,
    };

    Window(wl_resource *surface, wl_resource *xdgSurface, wl_resource *xdgToplevel,
           QObject *parent = nullptr);
    ~Window() override;

    wl_resource *surface() const;
    wl_resource *xdgSurface() const;
    wl_resource *xdgToplevel() const;

    QString title() const;
    void setTitle(const QString &title);

    QString appId() const;
    void setAppId(const QString &appId);

    QSize requestedSize() const;
    void setRequestedSize(const QSize &size);

    State state() const;
    void setState(State state);

    bool activated() const;
    void setActivated(bool activated);

    uint32_t lastConfigureSerial() const;
    void setLastConfigureSerial(uint32_t serial);

    uint32_t id() const;
    void setId(uint32_t id);

signals:
    void stateChanged();



private:
    wl_resource *m_surface = nullptr;
    wl_resource *m_xdgSurface = nullptr;
    wl_resource *m_xdgToplevel = nullptr;
    QString m_title;
    QString m_appId;
    QSize m_requestedSize;
    State m_state = State::Normal;
    bool m_activated = false;
    uint32_t m_lastConfigureSerial = 0;
    uint32_t m_id = 0;
};

} // namespace Cutefish
