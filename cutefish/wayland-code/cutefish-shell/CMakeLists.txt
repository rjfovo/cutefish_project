cmake_minimum_required(VERSION 3.16)

project(cutefish-shell VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

set(SHELL_QML
    qml/Main.qml
    qml/BootShell.qml
    qml/LoginShell.qml
    qml/SessionShell.qml
    qml/LockShell.qml
    qml/ShutdownShell.qml
    qml/Desktop.qml
    qml/Dock.qml
    qml/StatusBar.qml
    qml/Launcher.qml
)

qt_add_executable(cutefish-shell
    main.cpp
    shellclient.cpp
    shellclient.h
)

qt_add_resources(cutefish-shell "shell_qml"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES ${SHELL_QML}
)

target_link_libraries(cutefish-shell PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

install(TARGETS cutefish-shell RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

include(CTest)
if(BUILD_TESTING)
    add_test(
        NAME shell_qml_offscreen_smoke
        COMMAND cutefish-shell --boot --self-test
    )
    set_tests_properties(shell_qml_offscreen_smoke PROPERTIES
        TIMEOUT 15
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
    )
endif()
