/*
 * LifecycleStateMachine：core 生命周期状态机。
 * 状态：Boot -> Login -> Session <-> Lock -> Shutdown。
 * 状态由 core 持有；Shell 只能请求迁移，不能私自改写状态。
 */
#pragma once

#include <QObject>
#include <QString>

namespace Cutefish {

class LifecycleStateMachine : public QObject {
    Q_OBJECT
public:
    enum class State {
        Boot = 0,
        Login = 1,
        Session = 2,
        Lock = 3,
        Shutdown = 4,
    };
    Q_ENUM(State)

    explicit LifecycleStateMachine(QObject *parent = nullptr);

    State state() const;
    QString stateName() const;
    bool transitionTo(State target);

signals:
    void stateChanged(State state);

private:
    State m_state = State::Boot;
};

} // namespace Cutefish
