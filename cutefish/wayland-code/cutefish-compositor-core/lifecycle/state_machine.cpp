#include "lifecycle/state_machine.h"

#include <QDebug>
#include <QMetaEnum>

namespace Cutefish {

LifecycleStateMachine::LifecycleStateMachine(QObject *parent)
    : QObject(parent)
{
}

LifecycleStateMachine::State LifecycleStateMachine::state() const
{
    return m_state;
}

QString LifecycleStateMachine::stateName() const
{
    return QString::fromLatin1(QMetaEnum::fromType<State>().valueToKey(static_cast<int>(m_state)));
}

bool LifecycleStateMachine::transitionTo(State target)
{
    if (target == m_state)
        return true;

    const bool valid =
        (m_state == State::Boot && target == State::Login) ||
        (m_state == State::Login && (target == State::Session || target == State::Shutdown)) ||
        (m_state == State::Session && (target == State::Lock || target == State::Login || target == State::Shutdown)) ||
        (m_state == State::Lock && (target == State::Session || target == State::Login || target == State::Shutdown)) ||
        (m_state == State::Shutdown && target == State::Shutdown);

    if (!valid) {
        qWarning() << "invalid lifecycle transition" << stateName()
                   << "->" << QMetaEnum::fromType<State>().valueToKey(static_cast<int>(target));
        return false;
    }

    qInfo() << "lifecycle" << stateName()
            << "->" << QMetaEnum::fromType<State>().valueToKey(static_cast<int>(target));
    m_state = target;
    emit stateChanged(m_state);
    return true;
}

} // namespace Cutefish
