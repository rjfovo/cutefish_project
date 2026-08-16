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
