#include <iostream>
#include <QCoreApplication>
#include <QDebug>
#include "cutefish/system/wallpaper.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    System::Wallpaper wallpaper;
    
    qDebug() << "Wallpaper path:" << wallpaper.path();
    qDebug() << "Wallpaper type:" << wallpaper.type();
    qDebug() << "Wallpaper color:" << wallpaper.color();
    
    if (wallpaper.path().isEmpty()) {
        std::cout << "ERROR: Wallpaper path is empty!" << std::endl;
        return 1;
    } else {
        std::cout << "SUCCESS: Wallpaper path is: " << wallpaper.path().toStdString() << std::endl;
        return 0;
    }
}