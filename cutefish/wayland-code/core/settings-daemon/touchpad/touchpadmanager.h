#pragma once

#include <QObject>
#include <QSettings>

class TouchpadManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled)
    Q_PROPERTY(bool tapToClick READ tapToClick WRITE setTapToClick)
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration)

public:
    explicit TouchpadManager(QObject *parent = nullptr);

    bool available() const;
    bool enabled() const;
    void setEnabled(bool enabled);
    bool tapToClick() const;
    void setTapToClick(bool value);
    bool naturalScroll() const;
    void setNaturalScroll(bool naturalScroll);
    qreal pointerAcceleration() const;
    void setPointerAcceleration(qreal value);

private:
    QSettings m_settings;
};
