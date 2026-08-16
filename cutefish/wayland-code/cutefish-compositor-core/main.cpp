#include "backend/kms_backend.h"
#include "backend/virtual_backend.h"
#include "core_state.h"
#include "input/libinput_backend.h"
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
                                 QStringLiteral("Run the real KmsBackend. Safe by default: no DRM master/modeset unless explicitly authorized"));
    QCommandLineOption kmsModesetOption(QStringLiteral("kms-modeset"),
                                        QStringLiteral("Authorize DRM master and modeset path. Requires CUTEFISH_KMS_ALLOW_MODESET=1"));
    QCommandLineOption prefixOption(QStringLiteral("socket-prefix"),
                                    QStringLiteral("Socket name prefix"),
                                    QStringLiteral("prefix"),
                                    QStringLiteral("cutefish"));
    QCommandLineOption runtimeOption(QStringLiteral("runtime-dir"),
                                     QStringLiteral("Private runtime directory"),
                                     QStringLiteral("dir"));
    parser.addOption(virtualOption);
    parser.addOption(kmsOption);
    parser.addOption(kmsModesetOption);
    parser.addOption(prefixOption);
    parser.addOption(runtimeOption);
    parser.process(app);

    if (parser.isSet(kmsModesetOption) && !qEnvironmentVariableIsSet("CUTEFISH_KMS_ALLOW_MODESET")) {
        qCritical() << "KmsBackend modeset path requires CUTEFISH_KMS_ALLOW_MODESET=1";
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
    Cutefish::VirtualBackend virtualBackend;
    Cutefish::KmsBackend kmsBackend({QStringLiteral("/dev/dri/card0"),
                                     QStringLiteral("/dev/dri/renderD128"),
                                     parser.isSet(kmsModesetOption)});
    Cutefish::DisplayBackend *backend = &virtualBackend;
    if (parser.isSet(kmsOption)) {
        backend = &kmsBackend;
        if (!kmsBackend.initialize()) {
            qCritical() << "KmsBackend initialization failed";
            return 1;
        }
        if (!kmsBackend.drmMaster() && kmsBackend.modesetAllowed()) {
            qCritical() << "KmsBackend requested modeset but did not acquire DRM master";
            return 1;
        }
    } else if (!virtualBackend.initialize()) {
        qCritical() << "VirtualBackend initialization failed";
        return 1;
    }
    state.setDisplayBackend(backend);

    Cutefish::WaylandServer server(&state);
    Cutefish::LibinputBackend inputBackend;
    inputBackend.initialize();
    server.setInputBackend(&inputBackend);
    if (!server.start(runtimeDir, appsSocket, shellSocket)) {
        qCritical() << "Wayland server failed to start";
        return 1;
    }

    const int rc = server.run();
    qInfo() << "compositor core stopped, rc" << rc;
    inputBackend.shutdown();
    backend->shutdown();
    return rc == 0 ? 0 : 1;
}
