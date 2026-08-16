#pragma once

#include "backend/display_backend.h"
#include "lifecycle/state_machine.h"

#include <QObject>
#include <QString>

namespace Cutefish {

class CoreState : public QObject {
    Q_OBJECT
public:
    explicit CoreState(QObject *parent = nullptr);

    DisplayBackend *displayBackend() const;
    void setDisplayBackend(DisplayBackend *backend);
    LifecycleStateMachine *lifecycle() const;

    QString focusedAppId() const;
    void setFocusedAppId(const QString &appId);

private:
    DisplayBackend *m_displayBackend = nullptr;
    LifecycleStateMachine *m_lifecycle = nullptr;
    QString m_focusedAppId;
};

} // namespace Cutefish
