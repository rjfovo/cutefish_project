#include "application.h"

int main(int argc, char **argv)
{
    QCoreApplication::setQuitLockEnabled(false);
    Application app(argc, argv);
    return app.run();
}
