/*
 * Mouse：Wayland-only 鼠标配置数据服务。
 * 职责：持久化 leftHanded/acceleration/naturalScroll/pointerAcceleration；
 *       实际输入配置由 compositor core 读取并应用。
 */
#pragma once

#include <QObject>
#include <QSettings>

class Mouse : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool leftHanded READ leftHanded WRITE setLeftHanded NOTIFY leftHandedChanged)
    Q_PROPERTY(bool acceleration READ acceleration WRITE setAcceleration NOTIFY accelerationChanged)
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration NOTIFY pointerAccelerationChanged)

public:
    explicit Mouse(QObject *parent = nullptr);

    bool leftHanded() const;
    void setLeftHanded(bool enabled);

    bool acceleration() const;
    void setAcceleration(bool enabled);

    bool naturalScroll() const;
    void setNaturalScroll(bool enabled);

    qreal pointerAcceleration() const;
    void setPointerAcceleration(qreal value);

signals:
    void leftHandedChanged();
    void accelerationChanged();
    void naturalScrollChanged();
    void pointerAccelerationChanged();

private:
    QSettings m_settings;
};
