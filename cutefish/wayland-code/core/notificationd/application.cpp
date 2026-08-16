#include "application.h"
#include "notificationadaptor.h"
#include "notificationserver.h"
#include "notificationsmodel.h"
#include "settings.h"

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QFile>
#include <QLocale>
#include <QTranslator>

Application::Application(int &argc, char **argv)
    : QCoreApplication(argc, argv)
    , m_notificationServer(NotificationServer::self())
    , m_model(NotificationsModel::self())
    , m_settings(Settings::self())
{
    if (QDBusConnection::sessionBus().registerService(QStringLiteral("com.cutefish.Notification"))) {
        setOrganizationName(QStringLiteral("cutefishos"));

        QLocale locale;
        const QString qmFilePath = QStringLiteral("/usr/share/cutefish-notificationd/translations/%1.qm").arg(locale.name());
        if (QFile::exists(qmFilePath)) {
            auto *translator = new QTranslator(this);
            if (translator->load(qmFilePath))
                installTranslator(translator);
            else
                translator->deleteLater();
        }

        new NotificationAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/Notification"), this);
        m_instance = true;
    }
}

void Application::showWindow()
{
    // Data-only service: notification popup placement and rendering moved to
    // cutefish-shell through the shell surface role. Keep the D-Bus API for
    // compatibility and log the request for diagnostics.
    qInfo() << "showWindow requested; notification UI is rendered by cutefish-shell";
}

void Application::setDoNotDisturb(bool enabled)
{
    m_settings->setDoNotDisturb(enabled);
}

bool Application::doNotDisturb() const
{
    return m_settings->doNotDisturb();
}

int Application::run()
{
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Cutefish notification data service"));
    parser.addHelpOption();
    QCommandLineOption showOption({QStringLiteral("s"), QStringLiteral("show")}, QStringLiteral("Request notification UI"));
    parser.addOption(showOption);
    parser.process(*this);

    if (m_instance) {
        return QCoreApplication::exec();
    }

    if (parser.isSet(showOption)) {
        QDBusInterface iface(QStringLiteral("com.cutefish.Notification"),
                             QStringLiteral("/Notification"),
                             QStringLiteral("com.cutefish.Notification"),
                             QDBusConnection::sessionBus());
        if (iface.isValid())
            iface.call(QStringLiteral("showWindow"));
    }
    return 0;
}
