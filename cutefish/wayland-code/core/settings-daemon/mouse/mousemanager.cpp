#include "mousemanager.h"
#include "mouseadaptor.h"

#include <QDBusConnection>

Mouse::Mouse(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("cutefishos"), QStringLiteral("mouse"))
{
    new MouseAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Mouse"), this);
}

bool Mouse::leftHanded() const
{
    return m_settings.value(QStringLiteral("LeftHanded"), false).toBool();
}

void Mouse::setLeftHanded(bool enabled)
{
    if (leftHanded() == enabled)
        return;
    m_settings.setValue(QStringLiteral("LeftHanded"), enabled);
    m_settings.sync();
    emit leftHandedChanged();
}

bool Mouse::acceleration() const
{
    return m_settings.value(QStringLiteral("AccelerationProfileFlat"), false).toBool();
}

void Mouse::setAcceleration(bool enabled)
{
    if (acceleration() == enabled)
        return;
    m_settings.setValue(QStringLiteral("AccelerationProfileFlat"), enabled);
    m_settings.sync();
    emit accelerationChanged();
}

bool Mouse::naturalScroll() const
{
    return m_settings.value(QStringLiteral("NaturalScroll"), false).toBool();
}

void Mouse::setNaturalScroll(bool enabled)
{
    if (naturalScroll() == enabled)
        return;
    m_settings.setValue(QStringLiteral("NaturalScroll"), enabled);
    m_settings.sync();
    emit naturalScrollChanged();
}

qreal Mouse::pointerAcceleration() const
{
    return m_settings.value(QStringLiteral("PointerAcceleration"), 0.0).toReal();
}

void Mouse::setPointerAcceleration(qreal value)
{
    if (qFuzzyCompare(pointerAcceleration(), value))
        return;
    m_settings.setValue(QStringLiteral("PointerAcceleration"), value);
    m_settings.sync();
    emit pointerAccelerationChanged();
}
