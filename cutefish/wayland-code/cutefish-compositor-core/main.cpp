#include "backend/virtual_backend.h"
#include "core_state.h"
#include "wayland/server.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QString>

#include <csignal>
#include <unistd.h>

namespace {

void terminateHandler(int)
{
    if (Cutefish::WaylandServer::instance())
        Cutefish::WaylandServer::instance()->terminate();
}

} // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("cutefish-compositor-core"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("CutefishOS dedicated Wayland compositor core (stage-0 virtual baseline)"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption virtualOption(QStringLiteral("virtual"),
                                     QStringLiteral("Use the safe VirtualBackend (default)"));
    QCommandLineOption kmsOption(QStringLiteral("kms"),
                                 QStringLiteral("Reserved for stage-1 KmsBackend; refuses to run in stage-0"));
    QCommandLineOption prefixOption(QStringLiteral("socket-prefix"),
                                    QStringLiteral("Socket name prefix"),
                                    QStringLiteral("prefix"),
                                    QStringLiteral("cutefish"));
    QCommandLineOption runtimeOption(QStringLiteral("runtime-dir"),
                                     QStringLiteral("Private runtime directory"),
                                     QStringLiteral("dir"));
    parser.addOption(virtualOption);
    parser.addOption(kmsOption);
    parser.addOption(prefixOption);
    parser.addOption(runtimeOption);
    parser.process(app);

    if (parser.isSet(kmsOption)) {
        qCritical() << "KmsBackend is not implemented in stage-0; refusing to touch DRM devices";
        return 2;
    }

    QString runtimeDir;
    if (parser.isSet(runtimeOption)) {
        runtimeDir = parser.value(runtimeOption);
    } else {
        const QByteArray xdgRuntime = qgetenv("XDG_RUNTIME_DIR");
        runtimeDir = xdgRuntime.isEmpty()
            ? QStringLiteral("/tmp/cutefish-runtime-%1").arg(::getuid())
            : QString::fromLocal8Bit(xdgRuntime);
    }
    const QString prefix = parser.value(prefixOption);
    const QString appsSocket = prefix + QStringLiteral("-apps");
    const QString shellSocket = prefix + QStringLiteral("-shell");

    std::signal(SIGINT, terminateHandler);
    std::signal(SIGTERM, terminateHandler);

    Cutefish::CoreState state;
    Cutefish::VirtualBackend backend;
    if (!backend.initialize()) {
        qCritical() << "VirtualBackend initialization failed";
        return 1;
    }
    state.setDisplayBackend(&backend);

    Cutefish::WaylandServer server(&state);
    if (!server.start(runtimeDir, appsSocket, shellSocket)) {
        qCritical() << "Wayland server failed to start";
        return 1;
    }

    const int rc = server.run();
    qInfo() << "compositor core stopped, rc" << rc;
    backend.shutdown();
    return rc == 0 ? 0 : 1;
}
