#include "session/logind_session.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>

namespace Cutefish {

LogindSession::LogindSession()
    : m_sessionId(QString::fromLocal8Bit(qgetenv("XDG_SESSION_ID")))
{
}

QString LogindSession::sessionId() const
{
    return m_sessionId;
}

bool LogindSession::takeControl()
{
    // The product compositor registers its session through logind. Stage-1
    // keeps the D-Bus boundary explicit and does not take VT control here.
    QDBusInterface manager(QStringLiteral("org.freedesktop.login1"),
                           QStringLiteral("/org/freedesktop/login1"),
                           QStringLiteral("org.freedesktop.login1.Manager"),
                           QDBusConnection::systemBus());
    return manager.isValid();
}

bool LogindSession::releaseControl()
{
    return true;
}

void LogindSession::pause()
{
    QDBusInterface manager(QStringLiteral("org.freedesktop.login1"),
                           QStringLiteral("/org/freedesktop/login1"),
                           QStringLiteral("org.freedesktop.login1.Manager"),
                           QDBusConnection::systemBus());
    if (manager.isValid())
        manager.asyncCall(QStringLiteral("Suspend"), false);
}

void LogindSession::resume()
{
}

void LogindSession::switchToVT(int vt)
{
    QDBusInterface seat(QStringLiteral("org.freedesktop.login1"),
                        QStringLiteral("/org/freedesktop/login1/seat/seat0"),
                        QStringLiteral("org.freedesktop.login1.Seat"),
                        QDBusConnection::systemBus());
    if (seat.isValid())
        seat.asyncCall(QStringLiteral("SwitchTo"), vt);
}

void LogindSession::lock()
{
    if (m_sessionId.isEmpty())
        return;
    QDBusInterface session(QStringLiteral("org.freedesktop.login1"),
                           QStringLiteral("/org/freedesktop/login1/session/%1").arg(m_sessionId),
                           QStringLiteral("org.freedesktop.login1.Session"),
                           QDBusConnection::systemBus());
    if (session.isValid())
        session.asyncCall(QStringLiteral("Lock"));
}

void LogindSession::shutdown()
{
    QDBusInterface manager(QStringLiteral("org.freedesktop.login1"),
                           QStringLiteral("/org/freedesktop/login1"),
                           QStringLiteral("org.freedesktop.login1.Manager"),
                           QDBusConnection::systemBus());
    if (manager.isValid())
        manager.asyncCall(QStringLiteral("PowerOff"), false);
}

} // namespace Cutefish
