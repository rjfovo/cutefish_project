#include "power.h"

#include <QDebug>
#include <QProcess>

Power::Power(QObject *parent)
    : QObject(parent)
{
}

bool Power::logout()
{
    const QByteArray sessionId = qgetenv("XDG_SESSION_ID");
    QStringList args;
    if (!sessionId.isEmpty()) {
        args << QStringLiteral("terminate-session") << QString::fromLatin1(sessionId);
    } else {
        args << QStringLiteral("terminate-user") << qEnvironmentVariable("USER");
    }
    qInfo() << "power logout via loginctl";
    return QProcess::startDetached(QStringLiteral("loginctl"), args);
}

bool Power::reboot()
{
    qInfo() << "power reboot via systemctl";
    return QProcess::startDetached(QStringLiteral("systemctl"), {QStringLiteral("reboot")});
}

bool Power::shutdown()
{
    qInfo() << "power shutdown via systemctl";
    return QProcess::startDetached(QStringLiteral("systemctl"), {QStringLiteral("poweroff")});
}

bool Power::suspend()
{
    qInfo() << "power suspend via systemctl";
    return QProcess::startDetached(QStringLiteral("systemctl"), {QStringLiteral("suspend")});
}
