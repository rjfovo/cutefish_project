#pragma once

#include <QtDBus>

#include "application.h"

class DBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.panda.settings")
public:
    explicit DBusAdaptor(Application *app)
        : QDBusAbstractAdaptor(app)
        , m_app(app)
    {
    }

private:
    Application *m_app = nullptr;
};
