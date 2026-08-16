#include "displayservice.h"
#include "displayadaptor.h"

#include "shellcoreclient.h"

#include <QDBusMetaType>
#include <QDebug>
#include <QMetaObject>

DisplayService::DisplayService(ShellCoreClient *client, QObject *parent)
    : QObject(parent)
    , m_client(client)
{
    qDBusRegisterMetaType<QList<QVariantMap>>();
    connect(client, &ShellCoreClient::outputsChanged, this, &DisplayService::OutputsChanged);
}

bool DisplayService::registerService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(QStringLiteral("com.cutefish.Display"))) {
        qWarning() << "DisplayService: cannot register com.cutefish.Display";
        return false;
    }
    new DisplayAdaptor(this);
    if (!bus.registerObject(QStringLiteral("/Display"), this)) {
        bus.unregisterService(QStringLiteral("com.cutefish.Display"));
        return false;
    }
    // The adaptor is parented to the client and is exposed through the
    // registered object's child adaptors automatically.
    return true;
}

QList<QVariantMap> DisplayService::GetOutputs()
{
    QList<QVariantMap> result;
    if (!m_client)
        return result;
    m_client->syncRequestGetOutputs();
    const auto outputs = m_client->outputs();
    for (const auto &output : outputs) {
        QVariantMap map;
        map.insert(QStringLiteral("name"), output.name);
        map.insert(QStringLiteral("width"), output.width);
        map.insert(QStringLiteral("height"), output.height);
        map.insert(QStringLiteral("scale"), output.scale);
        map.insert(QStringLiteral("transform"), output.transform);
        map.insert(QStringLiteral("connected"), output.connected);
        result.append(map);
    }
    return result;
}

void DisplayService::SetMode(const QString &name, int width, int height)
{
    if (m_client)
        m_client->setOutputConfig(name, width, height, -1, -1);
}

void DisplayService::SetScale(const QString &name, int scale)
{
    if (!m_client)
        return;
    const auto outputs = m_client->outputs();
    for (const auto &output : outputs) {
        if (output.name == name) {
            m_client->setOutputConfig(name, output.width, output.height, scale, output.transform);
            return;
        }
    }
}

void DisplayService::SetTransform(const QString &name, int transform)
{
    if (!m_client)
        return;
    const auto outputs = m_client->outputs();
    for (const auto &output : outputs) {
        if (output.name == name) {
            m_client->setOutputConfig(name, output.width, output.height, output.scale, transform);
            return;
        }
    }
}
