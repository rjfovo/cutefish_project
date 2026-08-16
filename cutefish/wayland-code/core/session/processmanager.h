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
