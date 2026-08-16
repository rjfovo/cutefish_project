#include "dimdisplayaction.h"

#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>
#include <QGuiApplication>
#include <QProcess>
#include <QSettings>
#include <QTimer>

DimDisplayAction::DimDisplayAction(QObject *parent)
    : Action(parent)
    , m_iface(QStringLiteral("com.cutefish.Settings"),
              QStringLiteral("/Brightness"),
              QStringLiteral("com.cutefish.Brightness"),
              QDBusConnection::sessionBus())
{
    // Wayland-only path: DPMS is owned by the compositor core. This service
    // keeps the legacy brightness idle policy and requests session lock
    // through logind; it never opens a display connection.
}

void DimDisplayAction::onWakeupFromIdle()
{
    if (!m_dimmed)
        return;

    if (m_oldScreenBrightness < 0)
        m_oldScreenBrightness = 1;

    QTimer::singleShot(0, this, [this]() {
        m_iface.asyncCall(QStringLiteral("setValue"), QVariant::fromValue(m_oldScreenBrightness));
    });

    m_dimmed = false;
}

void DimDisplayAction::onIdleTimeout(int msec)
{
    const int sec = msec / 1000;

    if (m_iface.property("brightness").toInt() == 0)
        return;

    if (sec == m_dimOnIdleTime) {
        m_iface.asyncCall(QStringLiteral("setValue"), QVariant::fromValue(0));

        if (m_sleep) {
            QDBusInterface iface(QStringLiteral("com.cutefish.Session"),
                                 QStringLiteral("/Session"),
                                 QStringLiteral("com.cutefish.Session"),
                                 QDBusConnection::sessionBus());
            if (iface.isValid())
                iface.call(QStringLiteral("suspend"));
        }

        if (m_lock) {
            // Lock state is owned by the compositor core. Request the current
            // session lock through logind; stage-2 replaces this with the
            // core lifecycle request once the controlled IPC is available.
            QProcess::startDetached(QStringLiteral("loginctl"), {QStringLiteral("lock-session")});
        }
    } else if (sec == (m_dimOnIdleTime * 3 / 4)) {
        m_iface.asyncCall(QStringLiteral("setValue"), QVariant::fromValue(qRound(m_oldScreenBrightness / 8.0)));
    } else if (sec == (m_dimOnIdleTime * 1 / 2)) {
        m_oldScreenBrightness = m_iface.property("brightness").toInt();
        m_iface.asyncCall(QStringLiteral("setValue"), QVariant::fromValue(qRound(m_oldScreenBrightness / 2.0)));
    }

    m_dimmed = true;
}

void DimDisplayAction::setTimeout(int timeout)
{
    unregisterIdleTimeout();

    if (timeout < 0) {
        m_dimOnIdleTime = timeout;
        return;
    }

    m_dimOnIdleTime = timeout;
    registerIdleTimeout(m_dimOnIdleTime * 3 / 4);
    registerIdleTimeout(m_dimOnIdleTime / 2);
    registerIdleTimeout(m_dimOnIdleTime);
}

void DimDisplayAction::setSleep(bool sleep)
{
    m_sleep = sleep;
}

void DimDisplayAction::setLock(bool lock)
{
    m_lock = lock;
}
