#pragma once

#include "session/session_backend.h"

#include <QString>

namespace Cutefish {

class LogindSession : public SessionBackend {
public:
    explicit LogindSession();

    bool takeControl() override;
    bool releaseControl() override;
    void pause() override;
    void resume() override;
    void switchToVT(int vt) override;
    void lock() override;
    void shutdown() override;

    QString sessionId() const;

private:
    QString m_sessionId;
};

} // namespace Cutefish
