/*
 * Wayland-only cutefish-notificationd 数据服务。
 * 职责：org.freedesktop.Notifications D-Bus 数据、历史、免打扰。
 * 弹窗 UI 由 cutefish-shell 通知 role 渲染；本服务不再创建窗口。
 */
#pragma once

#include <QCoreApplication>

class NotificationServer;
class NotificationsModel;
class Settings;

class Application : public QCoreApplication
{
    Q_OBJECT
    Q_PROPERTY(bool doNotDisturb READ doNotDisturb WRITE setDoNotDisturb NOTIFY doNotDisturbChanged)
public:
    explicit Application(int &argc, char **argv);

    void showWindow();
    void setDoNotDisturb(bool enabled);
    bool doNotDisturb() const;
    int run();

signals:
    void doNotDisturbChanged();

private:
    NotificationServer *m_notificationServer = nullptr;
    NotificationsModel *m_model = nullptr;
    Settings *m_settings = nullptr;
    bool m_instance = false;
};
