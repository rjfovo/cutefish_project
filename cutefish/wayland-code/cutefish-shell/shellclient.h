#pragma once

#include <QObject>
#include <QString>

class ShellClient : public QObject {
    Q_OBJECT
    Q_PROPERTY(int mode READ mode CONSTANT)
    Q_PROPERTY(QString modeName READ modeName CONSTANT)
public:
    enum class Mode {
        Boot = 0,
        Login = 1,
        Session = 2,
        Lock = 3,
        Shutdown = 4,
    };
    Q_ENUM(Mode)

    explicit ShellClient(Mode mode, QObject *parent = nullptr);

    int mode() const;
    QString modeName() const;

    static Mode modeFromString(const QString &value, bool *ok = nullptr);

private:
    Mode m_mode;
};
