#include "shellclient.h"

ShellClient::ShellClient(Mode mode, QObject *parent)
    : QObject(parent)
    , m_mode(mode)
{
}

int ShellClient::mode() const
{
    return static_cast<int>(m_mode);
}

QString ShellClient::modeName() const
{
    switch (m_mode) {
    case Mode::Boot:
        return QStringLiteral("boot");
    case Mode::Login:
        return QStringLiteral("login");
    case Mode::Session:
        return QStringLiteral("session");
    case Mode::Lock:
        return QStringLiteral("lock");
    case Mode::Shutdown:
        return QStringLiteral("shutdown");
    }
    return QStringLiteral("boot");
}

ShellClient::Mode ShellClient::modeFromString(const QString &value, bool *ok)
{
    if (ok)
        *ok = true;
    if (value == QLatin1String("boot"))
        return Mode::Boot;
    if (value == QLatin1String("login") || value == QLatin1String("greeter"))
        return Mode::Login;
    if (value == QLatin1String("session"))
        return Mode::Session;
    if (value == QLatin1String("lock"))
        return Mode::Lock;
    if (value == QLatin1String("shutdown") || value == QLatin1String("poweroff"))
        return Mode::Shutdown;
    if (ok)
        *ok = false;
    return Mode::Boot;
}
