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
