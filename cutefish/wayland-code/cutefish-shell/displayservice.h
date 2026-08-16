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
