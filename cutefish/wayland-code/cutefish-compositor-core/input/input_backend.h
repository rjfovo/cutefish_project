/*
 * InputBackend：可替换输入接口。
 * 线程/进程边界：输入设备只允许 core 进程打开；Shell/应用不得访问。
 */
#pragma once

#include <QString>

namespace Cutefish {

// Replaceable input interface. Stage-0 defines the boundary only; the
// privileged libinput implementation belongs to stage-1 and is never opened
// by the stage-0 test programs.
class InputBackend {
public:
    virtual ~InputBackend() = default;

    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void setKeyboardLayout(const QString &layout) = 0;
    virtual void keyboardKey(uint32_t key, bool pressed) = 0;
    virtual void pointerMotion(double x, double y) = 0;
    virtual void pointerButton(uint32_t button, bool pressed) = 0;
    virtual void touchDown(int id, double x, double y) = 0;
    virtual void touchMotion(int id, double x, double y) = 0;
    virtual void touchUp(int id) = 0;
};

} // namespace Cutefish
