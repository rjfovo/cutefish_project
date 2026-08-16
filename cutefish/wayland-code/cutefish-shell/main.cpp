#include "shellclient.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include <cstdlib>

int main(int argc, char **argv)
{
    // Default to a safe headless-capable Qt QPA when tests do not provide one.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));

    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("cutefish-shell"));
    QGuiApplication::setOrganizationName(QStringLiteral("Cutefish"));
    QGuiApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("CutefishOS trusted Qt Quick shell (standard Wayland client)"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("mode"),
                                 QStringLiteral("boot|login|session|lock|shutdown"));

    QCommandLineOption bootOption(QStringLiteral("boot"), QStringLiteral("Show BootShell"));
    QCommandLineOption loginOption(QStringLiteral("greeter"), QStringLiteral("Show LoginShell"));
    QCommandLineOption sessionOption(QStringLiteral("session"), QStringLiteral("Show SessionShell"));
    QCommandLineOption lockOption(QStringLiteral("lock"), QStringLiteral("Show LockShell"));
    QCommandLineOption shutdownOption(QStringLiteral("shutdown"), QStringLiteral("Show ShutdownShell"));
    QCommandLineOption selfTestOption(QStringLiteral("self-test"),
                                      QStringLiteral("Exit automatically after the UI is loaded"));
    QCommandLineOption connectionTestOption(QStringLiteral("connection-test"),
                                            QStringLiteral("Initialize the selected QPA platform, verify the shell connection, and exit without creating a window"));
    parser.addOption(bootOption);
    parser.addOption(loginOption);
    parser.addOption(sessionOption);
    parser.addOption(lockOption);
    parser.addOption(shutdownOption);
    parser.addOption(selfTestOption);
    parser.addOption(connectionTestOption);
    parser.process(app);

    ShellClient::Mode mode = ShellClient::Mode::Boot;
    bool explicitMode = false;
    const QStringList positional = parser.positionalArguments();
    if (!positional.isEmpty()) {
        mode = ShellClient::modeFromString(positional.first(), &explicitMode);
        if (!explicitMode) {
            parser.showHelp(1);
        }
    }
    if (parser.isSet(bootOption))
        mode = ShellClient::Mode::Boot;
    else if (parser.isSet(loginOption))
        mode = ShellClient::Mode::Login;
    else if (parser.isSet(sessionOption))
        mode = ShellClient::Mode::Session;
    else if (parser.isSet(lockOption))
        mode = ShellClient::Mode::Lock;
    else if (parser.isSet(shutdownOption))
        mode = ShellClient::Mode::Shutdown;

    ShellClient shellClient(mode);

    if (parser.isSet(connectionTestOption)) {
        if (QGuiApplication::platformName() != QLatin1String("wayland")) {
            qCritical() << "connection-test requires the Wayland QPA; current platform is"
                        << QGuiApplication::platformName();
            return 1;
        }
        QTimer::singleShot(150, &app, [&app]() {
            app.exit(0);
        });
        return app.exec();
    }

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("ShellClient"), &shellClient);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;

    if (parser.isSet(selfTestOption)) {
        QTimer::singleShot(700, &app, [&app]() {
            app.exit(0);
        });
    }

    return app.exec();
}
