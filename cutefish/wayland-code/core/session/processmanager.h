/*
 * ProcessManager：用户会话 daemon/autostart 管理。
 * 职责：启动 settings-daemon/powerman/notificationd/polkit-agent/clipboard；
 *       不再包含旧显示服务/窗口管理器启动逻辑。
 */
#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>

class SessionApplication;

class ProcessManager : public QObject
{
    Q_OBJECT
public:
    explicit ProcessManager(SessionApplication *app, QObject *parent = nullptr);
    ~ProcessManager() override;

    void start();
    void startDesktopProcess();
    void startDaemonProcess();
    void loadAutoStartProcess();

private:
    SessionApplication *m_app = nullptr;
    QList<QProcess *> m_systemProcesses;
};
