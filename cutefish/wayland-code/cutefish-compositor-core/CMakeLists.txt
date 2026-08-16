cmake_minimum_required(VERSION 3.16)

project(cutefish-compositor-core VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui DBus)
find_package(PkgConfig REQUIRED)
pkg_check_modules(WAYLAND_SERVER REQUIRED IMPORTED_TARGET wayland-server>=1.20)
pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client>=1.20)
pkg_check_modules(DRM REQUIRED IMPORTED_TARGET libdrm)
pkg_check_modules(GBM REQUIRED IMPORTED_TARGET gbm)
pkg_check_modules(LIBINPUT REQUIRED IMPORTED_TARGET libinput)
pkg_check_modules(UDEV REQUIRED IMPORTED_TARGET libudev)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)

find_program(WAYLAND_SCANNER NAMES wayland-scanner REQUIRED)

set(PROTOCOL_XML "${CMAKE_CURRENT_SOURCE_DIR}/protocol/cutefish-core-v1.xml")
set(PROTOCOL_C "${CMAKE_CURRENT_BINARY_DIR}/cutefish-core-v1-protocol.c")
set(PROTOCOL_SERVER_H "${CMAKE_CURRENT_BINARY_DIR}/cutefish-core-v1-server-protocol.h")
set(PROTOCOL_CLIENT_H "${CMAKE_CURRENT_BINARY_DIR}/cutefish-core-v1-client-protocol.h")

add_custom_command(
    OUTPUT "${PROTOCOL_C}" "${PROTOCOL_SERVER_H}" "${PROTOCOL_CLIENT_H}"
    COMMAND "${WAYLAND_SCANNER}" private-code "${PROTOCOL_XML}" "${PROTOCOL_C}"
    COMMAND "${WAYLAND_SCANNER}" server-header "${PROTOCOL_XML}" "${PROTOCOL_SERVER_H}"
    COMMAND "${WAYLAND_SCANNER}" client-header "${PROTOCOL_XML}" "${PROTOCOL_CLIENT_H}"
    DEPENDS "${PROTOCOL_XML}"
    VERBATIM
)

add_library(cutefish-core-v1-protocol STATIC "${PROTOCOL_C}")
target_include_directories(cutefish-core-v1-protocol PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(cutefish-core-v1-protocol PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Minimal xdg-shell (stable) protocol code. The implementation in
# wayland/server.cpp is intentionally stage-0 minimal and has no window
# management policy yet; it exists so a Qt Wayland client can initialize a
# shell connection against the VirtualBackend.
pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
set(XDG_SHELL_XML "${WAYLAND_PROTOCOLS_DATADIR}/stable/xdg-shell/xdg-shell.xml")
set(XDG_SHELL_C "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-protocol.c")
set(XDG_SHELL_SERVER_H "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-server-protocol.h")
set(XDG_SHELL_CLIENT_H "${CMAKE_CURRENT_BINARY_DIR}/xdg-shell-client-protocol.h")

add_custom_command(
    OUTPUT "${XDG_SHELL_C}" "${XDG_SHELL_SERVER_H}" "${XDG_SHELL_CLIENT_H}"
    COMMAND "${WAYLAND_SCANNER}" private-code "${XDG_SHELL_XML}" "${XDG_SHELL_C}"
    COMMAND "${WAYLAND_SCANNER}" server-header "${XDG_SHELL_XML}" "${XDG_SHELL_SERVER_H}"
    COMMAND "${WAYLAND_SCANNER}" client-header "${XDG_SHELL_XML}" "${XDG_SHELL_CLIENT_H}"
    DEPENDS "${XDG_SHELL_XML}"
    VERBATIM
)

add_library(cutefish-xdg-shell-protocol STATIC "${XDG_SHELL_C}")
target_include_directories(cutefish-xdg-shell-protocol PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(cutefish-xdg-shell-protocol PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

add_executable(cutefish-compositor-core
    main.cpp
    backend/virtual_backend.cpp
    backend/kms_backend.cpp
    input/libinput_backend.cpp
    session/logind_session.cpp
    core_state.cpp
    lifecycle/state_machine.cpp
    wm/window.cpp
    wm/workspace.cpp
    wayland/seat.cpp
    wayland/server.cpp
    wayland/xdg_shell.cpp
)
target_include_directories(cutefish-compositor-core PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)
target_link_libraries(cutefish-compositor-core PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::DBus
    PkgConfig::WAYLAND_SERVER
    PkgConfig::DRM
    PkgConfig::GBM
    PkgConfig::LIBINPUT
    PkgConfig::UDEV
    PkgConfig::XKBCOMMON
    cutefish-core-v1-protocol
    cutefish-xdg-shell-protocol
)

install(TARGETS cutefish-compositor-core RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES data/cutefish-compositor-core.service
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/systemd/system")

include(CTest)
if(BUILD_TESTING)
    add_executable(wayland_protocol_smoke tests/wayland_protocol_smoke.cpp)
    target_include_directories(wayland_protocol_smoke PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}"
    )
    target_link_libraries(wayland_protocol_smoke PRIVATE
        Qt6::Core
        PkgConfig::WAYLAND_CLIENT
        cutefish-core-v1-protocol
        cutefish-xdg-shell-protocol
    )
    add_test(
        NAME wayland_protocol_smoke
        COMMAND wayland_protocol_smoke "$<TARGET_FILE:cutefish-compositor-core>"
    )
    set_tests_properties(wayland_protocol_smoke PROPERTIES TIMEOUT 20)
endif()
