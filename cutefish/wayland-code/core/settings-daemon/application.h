/*
 * Wayland-only cutefish-settings-daemon。
 * 职责：Theme/Brightness/Battery/Language/Dock/DefaultApps 用户会话 D-Bus 服务。
 * 输入配置只写 Cutefish 配置文件；不打开旧显示设备属性。
 */
#pragma once

#include <QGuiApplication>
#include <QTimer>

#include "battery/upowermanager.h"
#include "brightness/brightnessmanager.h"
#include "defaultapplications.h"
#include "language/language.h"
#include "mouse/mousemanager.h"
#include "theme/thememanager.h"
#include "touchpad/touchpadmanager.h"

class Application : public QGuiApplication
{
    Q_OBJECT
public:
    explicit Application(int &argc, char **argv);

public slots:
    void invokeDesktopProcess();

private:
    void initTrash();
    void initTranslations();

    ThemeManager *m_themeManager = nullptr;
    BrightnessManager *m_brightnessManager = nullptr;
    UPowerManager *m_upowerManager = nullptr;
    Language *m_language = nullptr;
    Mouse *m_mouse = nullptr;
    TouchpadManager *m_touchpad = nullptr;
    DefaultApplications *m_defaultApps = nullptr;
};
