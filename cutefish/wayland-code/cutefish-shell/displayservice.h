/*
 * DisplayService：用户会话 D-Bus com.cutefish.Display。
 * 职责：普通设置应用 -> Shell -> cutefish_core_v1 -> core 的输出管理通路。
 * 安全边界：普通应用不能直接绑定 cutefish_core_v1。
 */
#pragma once

#include <QDBusConnection>
#include <QObject>
#include <QVariantMap>

class ShellCoreClient;

class DisplayService : public QObject {
    Q_OBJECT
public:
    explicit DisplayService(ShellCoreClient *client, QObject *parent = nullptr);

    bool registerService();

public slots:
    QList<QVariantMap> GetOutputs();
    void SetMode(const QString &name, int width, int height);
    void SetScale(const QString &name, int scale);
    void SetTransform(const QString &name, int transform);

signals:
    void OutputsChanged();

private:
    ShellCoreClient *m_client = nullptr;
};
