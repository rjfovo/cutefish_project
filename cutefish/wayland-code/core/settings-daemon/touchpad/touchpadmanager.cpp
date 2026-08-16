#include "touchpadmanager.h"
#include "touchpadadaptor.h"

#include <QDBusConnection>

TouchpadManager::TouchpadManager(QObject *parent)
    : QObject(parent)
    , m_settings(QStringLiteral("cutefishos"), QStringLiteral("touchpad"))
{
    new TouchpadAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Touchpad"), this);
}

bool TouchpadManager::available() const
{
    // Hardware discovery belongs to the compositor core. The user-session
    // settings service always exposes the API; stage-1 forwards the actual
    // availability flag from libinput through com.cutefish.Input.
    return true;
}

bool TouchpadManager::enabled() const
{
    return m_settings.value(QStringLiteral("Enabled"), true).toBool();
}

void TouchpadManager::setEnabled(bool enabled)
{
    if (this->enabled() == enabled)
        return;
    m_settings.setValue(QStringLiteral("Enabled"), enabled);
    m_settings.sync();
}

bool TouchpadManager::tapToClick() const
{
    return m_settings.value(QStringLiteral("TapToClick"), true).toBool();
}

void TouchpadManager::setTapToClick(bool value)
{
    if (tapToClick() == value)
        return;
    m_settings.setValue(QStringLiteral("TapToClick"), value);
    m_settings.sync();
}

bool TouchpadManager::naturalScroll() const
{
    return m_settings.value(QStringLiteral("NaturalScroll"), true).toBool();
}

void TouchpadManager::setNaturalScroll(bool naturalScroll)
{
    if (this->naturalScroll() == naturalScroll)
        return;
    m_settings.setValue(QStringLiteral("NaturalScroll"), naturalScroll);
    m_settings.sync();
}

qreal TouchpadManager::pointerAcceleration() const
{
    return m_settings.value(QStringLiteral("PointerAcceleration"), 0.0).toReal();
}

void TouchpadManager::setPointerAcceleration(qreal value)
{
    if (qFuzzyCompare(pointerAcceleration(), value))
        return;
    m_settings.setValue(QStringLiteral("PointerAcceleration"), value);
    m_settings.sync();
}
