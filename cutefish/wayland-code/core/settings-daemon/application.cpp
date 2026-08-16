#include "application.h"
#include "dbusadaptor.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QLocale>
#include <QStandardPaths>
#include <QTimer>
#include <QTranslator>

#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

Application::Application(int &argc, char **argv)
    : QGuiApplication(argc, argv)
    , m_themeManager(ThemeManager::self())
    , m_brightnessManager(new BrightnessManager(this))
    , m_upowerManager(new UPowerManager(this))
    , m_language(Language::self())
    , m_mouse(new Mouse(this))
    , m_touchpad(new TouchpadManager(this))
    , m_defaultApps(new DefaultApplications)
{
    initTrash();
    new DBusAdaptor(this);
    QDBusConnection::sessionBus().registerService(QStringLiteral("com.cutefish.Settings"));
    initTranslations();
    QTimer::singleShot(10, this, &Application::invokeDesktopProcess);
}

void Application::invokeDesktopProcess()
{
    // Desktop/Dock/StatusBar/Launcher are owned by cutefish-shell and managed
    // by the compositor core. The session service exposes the compatibility
    // entry point and forwards it through the session D-Bus service.
    QDBusInterface sessionInterface(QStringLiteral("com.cutefish.Session"),
                                    QStringLiteral("/Session"),
                                    QStringLiteral("com.cutefish.Session"),
                                    QDBusConnection::sessionBus());
    if (sessionInterface.isValid())
        sessionInterface.call(QStringLiteral("startDesktopProcess"));
}

void Application::initTranslations()
{
    QLocale locale;
    const QString qmFilePath = QStringLiteral("/usr/share/cutefish-settings-daemon/translations/%1.qm").arg(locale.name());
    if (!QFile::exists(qmFilePath))
        return;
    auto *translator = new QTranslator(this);
    if (translator->load(qmFilePath))
        installTranslator(translator);
    else
        translator->deleteLater();
}

void Application::initTrash()
{
    const QByteArray trashDir = QString(QDir::homePath() + QStringLiteral("/.local/share/Trash")).toLatin1();
    if (::mkdir(trashDir.constData(), 0700) != 0)
        return;

    struct stat buff;
    const uid_t uid = ::getuid();
    if (::lstat(trashDir.constData(), &buff) != 0)
        return;

    if (buff.st_uid == uid && ((buff.st_mode & 0777) == 0700)) {
        QDir().mkpath(QString::fromLatin1(trashDir) + QStringLiteral("/info"));
        QDir().mkpath(QString::fromLatin1(trashDir) + QStringLiteral("/files"));
    } else {
        ::rmdir(trashDir.constData());
    }
}
