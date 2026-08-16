/*
 * Power：logind/systemd 电源动作封装。
 * 安全边界：所有动作经 loginctl/systemctl，由 polkit 授权。
 */
#pragma once

#include <QObject>

class Power : public QObject
{
    Q_OBJECT
public:
    explicit Power(QObject *parent = nullptr);

    bool logout();
    bool reboot();
    bool shutdown();
    bool suspend();
};
