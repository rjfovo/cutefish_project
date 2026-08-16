#pragma once

#include "input/input_backend.h"

#include <libinput.h>

#include <QString>

namespace Cutefish {

class LibinputBackend : public InputBackend {
public:
    explicit LibinputBackend();
    ~LibinputBackend() override;

    bool initialize() override;
    void shutdown() override;
    int dispatch();

    void setKeyboardLayout(const QString &layout) override;
    void keyboardKey(uint32_t key, bool pressed) override;
    void pointerMotion(double x, double y) override;
    void pointerButton(uint32_t button, bool pressed) override;
    void touchDown(int id, double x, double y) override;
    void touchMotion(int id, double x, double y) override;
    void touchUp(int id) override;

    int fd() const;
    libinput *context() const;

private:
    libinput *m_context = nullptr;
    udev *m_udev = nullptr;
    QString m_keyboardLayout;
};

} // namespace Cutefish
