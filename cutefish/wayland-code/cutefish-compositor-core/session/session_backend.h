#pragma once

namespace Cutefish {

// Replaceable privileged session backend (logind in the product). Stage-0
// keeps the interface only; no VT/DRM-master operations are performed here.
class SessionBackend {
public:
    virtual ~SessionBackend() = default;

    virtual bool takeControl() = 0;
    virtual bool releaseControl() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
    virtual void switchToVT(int vt) = 0;
    virtual void lock() = 0;
    virtual void shutdown() = 0;
};

} // namespace Cutefish
