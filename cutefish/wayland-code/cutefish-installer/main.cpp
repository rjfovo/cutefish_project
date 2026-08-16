#include "installerbackend.h"

#include <QCommandLineParser>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

#include <cstdlib>

int main(int argc, char **argv)
{
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));

    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("cutefish-installer"));
    QGuiApplication::setOrganizationName(QStringLiteral("Cutefish"));
    QGuiApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("CutefishOS dedicated installer (stage-0 safe UI skeleton)"));
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption selfTestOption(QStringLiteral("self-test"),
                                      QStringLiteral("Cycle through the fixed page flow and exit"));
    parser.addOption(selfTestOption);
    parser.process(app);

    InstallerBackend backend;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("InstallerBackend"), &backend);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;

    if (parser.isSet(selfTestOption)) {
        QTimer *timer = new QTimer(&app);
        QObject::connect(timer, &QTimer::timeout, &app, [&backend, &app, &timer]() {
            if (backend.currentStep() < InstallerBackend::FinishStep) {
                backend.next();
            } else {
                timer->stop();
                app.exit(0);
            }
        });
        timer->start(60);
    }

    return app.exec();
}
