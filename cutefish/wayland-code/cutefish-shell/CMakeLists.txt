cmake_minimum_required(VERSION 3.16)

project(cutefish-shell VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2 DBus)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)

find_program(WAYLAND_SCANNER NAMES wayland-scanner REQUIRED)
set(PROTOCOL_XML "${CMAKE_CURRENT_SOURCE_DIR}/protocol/cutefish-core-v1.xml")
set(PROTOCOL_C "${CMAKE_CURRENT_BINARY_DIR}/cutefish-core-v1-protocol.c")
set(PROTOCOL_CLIENT_H "${CMAKE_CURRENT_BINARY_DIR}/cutefish-core-v1-client-protocol.h")
add_custom_command(
    OUTPUT "${PROTOCOL_C}" "${PROTOCOL_CLIENT_H}"
    COMMAND "${WAYLAND_SCANNER}" private-code "${PROTOCOL_XML}" "${PROTOCOL_C}"
    COMMAND "${WAYLAND_SCANNER}" client-header "${PROTOCOL_XML}" "${PROTOCOL_CLIENT_H}"
    DEPENDS "${PROTOCOL_XML}"
    VERBATIM
)

qt6_add_dbus_adaptor(DBUS_SOURCES
    com.cutefish.Display.xml
    displayservice.h
    DisplayService
    displayadaptor
    DisplayAdaptor
)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_library(shell-core-protocol STATIC "${PROTOCOL_C}")
target_include_directories(shell-core-protocol PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(shell-core-protocol PROPERTIES POSITION_INDEPENDENT_CODE ON)


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
    shellcoreclient.cpp
    shellcoreclient.h
    displayservice.cpp
    displayservice.h
    ${DBUS_SOURCES}
)

target_include_directories(cutefish-shell PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
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
    Qt6::DBus
    PkgConfig::WAYLAND_CLIENT
    shell-core-protocol
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
