/*
 * Wayland-only cutefish-session 会话服务。
 * 职责：用户环境、语言、网络代理、用户 daemon 启动和 logind 电源动作。
 * 不再启动窗口管理器；桌面/Dock/顶栏/启动器由 cutefish-shell 承载。
 */
#pragma once

#include <QCoreApplication>

class ProcessManager;
class NetworkProxyManager;
class Power;

class SessionApplication : public QCoreApplication
{
    Q_OBJECT
public:
    explicit SessionApplication(int &argc, char **argv);

public slots:
    void logout();
    void reboot();
    void powerOff();
    void suspend();
    void startDesktopProcess();
    void updateNetworkProxy();
    void launch(const QString &exec, const QStringList &args);
    void launch(const QString &exec, const QString &workingDirectory, const QStringList &args);

private:
    void initEnvironments();
    void initLanguage();
    void createConfigDirectory();
    void updateUserDirs();

    ProcessManager *m_processManager = nullptr;
    NetworkProxyManager *m_networkProxyManager = nullptr;
    Power *m_power = nullptr;
};
