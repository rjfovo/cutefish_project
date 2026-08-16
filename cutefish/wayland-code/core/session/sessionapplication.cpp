#include "sessionapplication.h"
#include "sessionadaptor.h"

#include "networkproxymanager.h"
#include "powermanager/power.h"
#include "processmanager.h"

#include <QDBusConnection>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>

SessionApplication::SessionApplication(int &argc, char **argv)
    : QCoreApplication(argc, argv)
    , m_processManager(new ProcessManager(this))
    , m_networkProxyManager(new NetworkProxyManager(this))
    , m_power(new Power(this))
{
    setOrganizationName(QStringLiteral("cutefishos"));

    new SessionAdaptor(this);
    QDBusConnection::sessionBus().registerService(QStringLiteral("com.cutefish.Session"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Session"), this);

    createConfigDirectory();
    initEnvironments();
    initLanguage();

    m_networkProxyManager->update();
    QTimer::singleShot(100, m_processManager, &ProcessManager::start);
}

void SessionApplication::logout()
{
    if (m_power)
        m_power->logout();
    QCoreApplication::exit(0);
}

void SessionApplication::reboot()
{
    if (m_power)
        m_power->reboot();
    QCoreApplication::exit(0);
}

void SessionApplication::powerOff()
{
    if (m_power)
        m_power->shutdown();
    QCoreApplication::exit(0);
}

void SessionApplication::suspend()
{
    if (m_power)
        m_power->suspend();
}

void SessionApplication::startDesktopProcess()
{
    if (m_processManager)
        m_processManager->startDesktopProcess();
}

void SessionApplication::updateNetworkProxy()
{
    if (m_networkProxyManager)
        m_networkProxyManager->update();
}

void SessionApplication::launch(const QString &exec, const QStringList &args)
{
    QProcess::startDetached(exec, args);
}

void SessionApplication::launch(const QString &exec, const QString &workingDirectory, const QStringList &args)
{
    QProcess::startDetached(exec, args, workingDirectory);
}

void SessionApplication::initEnvironments()
{
    const QString home = QDir::homePath();
    if (qEnvironmentVariableIsEmpty("XDG_DATA_HOME"))
        qputenv("XDG_DATA_HOME", QDir(home).absoluteFilePath(QStringLiteral(".local/share")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_DESKTOP_DIR"))
        qputenv("XDG_DESKTOP_DIR", QDir(home).absoluteFilePath(QStringLiteral("Desktop")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_CONFIG_HOME"))
        qputenv("XDG_CONFIG_HOME", QDir(home).absoluteFilePath(QStringLiteral(".config")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_CACHE_HOME"))
        qputenv("XDG_CACHE_HOME", QDir(home).absoluteFilePath(QStringLiteral(".cache")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_DATA_DIRS"))
        qputenv("XDG_DATA_DIRS", "/usr/local/share/:/usr/share/");
    if (qEnvironmentVariableIsEmpty("XDG_CONFIG_DIRS"))
        qputenv("XDG_CONFIG_DIRS", "/etc/xdg");

    qputenv("DESKTOP_SESSION", "Cutefish");
    qputenv("XDG_CURRENT_DESKTOP", "Cutefish");
    qputenv("XDG_SESSION_DESKTOP", "Cutefish");

    qputenv("QT_QPA_PLATFORMTHEME", "cutefish");
    qputenv("QT_STYLE_OVERRIDE", "cutefish");
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "wayland");
    qputenv("QT_QPA_UPDATE_IDLE_TIME", "10");
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");
}

void SessionApplication::initLanguage()
{
    QSettings settings(QSettings::UserScope, QStringLiteral("cutefishos"), QStringLiteral("language"));
    const QString value = settings.value(QStringLiteral("language")).toString();
    if (value.isEmpty())
        return;

    const QString langFile = QStringLiteral("/etc/default/cutefish-locale");
    QFile file(langFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (line.startsWith(value + QLatin1Char('='))) {
            const QString pair = line.mid(value.size() + 1);
            const int idx = pair.indexOf(QLatin1Char('='));
            if (idx > 0) {
                const QByteArray key = pair.left(idx).toUtf8();
                const QByteArray val = pair.mid(idx + 1).toUtf8();
                if (!key.isEmpty() && !val.isEmpty())
                    qputenv(key, val);
            }
        }
    }
}

void SessionApplication::createConfigDirectory()
{
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(configDir);
    QDir().mkpath(configDir + QStringLiteral("/autostart"));
}

void SessionApplication::updateUserDirs()
{
    const QString home = QDir::homePath();
    for (const QString &name : {QStringLiteral("Desktop"), QStringLiteral("Documents"),
                                QStringLiteral("Downloads"), QStringLiteral("Music"),
                                QStringLiteral("Pictures"), QStringLiteral("Videos")}) {
        QDir().mkpath(home + QLatin1Char('/') + name);
    }
}
