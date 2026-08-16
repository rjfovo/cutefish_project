cmake_minimum_required(VERSION 3.16)

project(cutefish-core VERSION 0.9.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS
    Core Gui Widgets Quick QuickControls2 DBus Xml LinguistTools)
find_package(PkgConfig REQUIRED)
find_package(PolkitQt6-1 REQUIRED)
find_package(KF6IdleTime REQUIRED)

# Wayland-only build boundary. Legacy display-server components from the
# frozen audit source are intentionally absent from this migration copy.
# They may be re-introduced only as Wayland-native implementations or remain
# excluded per the migration plan.

add_subdirectory(clipboard)
add_subdirectory(notificationd)
add_subdirectory(powerman)
add_subdirectory(session)
add_subdirectory(settings-daemon)
add_subdirectory(cpufreq)
add_subdirectory(polkit-agent)
add_subdirectory(screen-brightness)
add_subdirectory(shutdown-ui)

install(FILES cutefish DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}")
