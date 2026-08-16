#include "sessionapplication.h"

#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication::setQuitLockEnabled(false);
    SessionApplication app(argc, argv);
    return app.exec();
}
