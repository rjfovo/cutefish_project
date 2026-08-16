#include "core_state.h"

namespace Cutefish {

CoreState::CoreState(QObject *parent)
    : QObject(parent)
    , m_lifecycle(new LifecycleStateMachine(this))
{
}

DisplayBackend *CoreState::displayBackend() const
{
    return m_displayBackend;
}

void CoreState::setDisplayBackend(DisplayBackend *backend)
{
    m_displayBackend = backend;
}

LifecycleStateMachine *CoreState::lifecycle() const
{
    return m_lifecycle;
}

QString CoreState::focusedAppId() const
{
    return m_focusedAppId;
}

void CoreState::setFocusedAppId(const QString &appId)
{
    if (m_focusedAppId == appId)
        return;
    m_focusedAppId = appId;
}

} // namespace Cutefish
