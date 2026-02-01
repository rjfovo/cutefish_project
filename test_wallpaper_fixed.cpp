#include <iostream>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    // 测试壁纸路径
    QString wallpaperPath;
    
    // 首先检查用户自定义壁纸
    QString configLocation = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QString userWallpaper = configLocation + "/cutefishos/wallpaper.jpg";
    
    if (QFile::exists(userWallpaper)) {
        wallpaperPath = userWallpaper;
        qDebug() << "Using user wallpaper:" << wallpaperPath;
    } else {
        // 使用默认壁纸
        QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (const QString &path : dataPaths) {
            QString defaultWallpaper = path + "/backgrounds/cutefishos/default.jpg";
            if (QFile::exists(defaultWallpaper)) {
                wallpaperPath = defaultWallpaper;
                qDebug() << "Using default wallpaper:" << wallpaperPath;
                break;
            }
        }
    }
    
    if (wallpaperPath.isEmpty()) {
        qDebug() << "ERROR: Wallpaper path is empty!";
        qDebug() << "Config location:" << QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
        qDebug() << "Data locations:" << QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        
        // 尝试直接检查路径
        QString testPath = "/usr/share/backgrounds/cutefishos/default.jpg";
        if (QFile::exists(testPath)) {
            qDebug() << "Found wallpaper at:" << testPath;
            wallpaperPath = testPath;
        } else {
            qDebug() << "Wallpaper not found at:" << testPath;
        }
    } else {
        qDebug() << "SUCCESS: Wallpaper path found:" << wallpaperPath;
    }
    
    return 0;
}
