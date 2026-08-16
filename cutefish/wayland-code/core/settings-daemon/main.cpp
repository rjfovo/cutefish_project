#include "application.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setQuitLockEnabled(false);
    Application app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    return app.exec();
}
