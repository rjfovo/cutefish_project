#include "processmanager.h"
#include "sessionapplication.h"

#include <QDebug>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

ProcessManager::ProcessManager(SessionApplication *app, QObject *parent)
    : QObject(parent)
    , m_app(app)
{
}

ProcessManager::~ProcessManager()
{
    for (QProcess *process : std::as_const(m_systemProcesses)) {
        if (process) {
            process->terminate();
            if (!process->waitForFinished(1500))
                process->kill();
            delete process;
        }
    }
}

void ProcessManager::start()
{
    // The display service is owned by cutefish-compositor-core. This session
    // service only starts user-session daemons and desktop autostart items.
    startDaemonProcess();
    startDesktopProcess();
    loadAutoStartProcess();
}

void ProcessManager::startDesktopProcess()
{
    // Desktop/Dock/StatusBar/Launcher are QML components inside
    // cutefish-shell; the compositor core ShellManager owns their lifecycle.
    qInfo() << "session desktop process request: handled by cutefish-shell";
}

void ProcessManager::startDaemonProcess()
{
    if (qEnvironmentVariableIsSet("CUTEFISH_SESSION_TEST")) {
        qInfo() << "session test mode: user daemons are not started";
        return;
    }
    const QStringList daemons = {
        QStringLiteral("cutefish-settings-daemon"),
        QStringLiteral("cutefish-powerman"),
        QStringLiteral("cutefish-notificationd"),
        QStringLiteral("cutefish-polkit-agent"),
        QStringLiteral("cutefish-clipboard"),
    };

    for (const QString &daemon : daemons) {
        if (QProcess::startDetached(daemon, {}, QDir::homePath())) {
            qInfo() << "started user daemon" << daemon;
        } else {
            qWarning() << "failed to start user daemon" << daemon;
        }
    }
}

void ProcessManager::loadAutoStartProcess()
{
    const QString autostart = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
        + QStringLiteral("/autostart");
    const QDir dir(autostart);
    const QStringList entries = dir.entryList({QStringLiteral("*.desktop")}, QDir::Files, QDir::Name);
    for (const QString &entry : entries) {
        QSettings desktop(dir.absoluteFilePath(entry), QSettings::IniFormat);
        desktop.beginGroup(QStringLiteral("Desktop Entry"));
        if (desktop.value(QStringLiteral("Hidden"), false).toBool())
            continue;
        const QString exec = desktop.value(QStringLiteral("Exec")).toString();
        desktop.endGroup();
        if (exec.isEmpty())
            continue;
        // Do not execute arbitrary shell from .desktop files; only accept a
        // simple program name for the stage-0 baseline.
        const QStringList parts = exec.simplified().split(QLatin1Char(' '));
        if (!parts.isEmpty())
            QProcess::startDetached(parts.first(), parts.mid(1));
    }
}
