project(cutefish-settings-daemon LANGUAGES CXX)
set(TARGET cutefish-settings-daemon)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus Xml LinguistTools)

set(SOURCES
    application.cpp
    dbusadaptor.h
    main.cpp
    defaultapplications.cpp
    battery/battery.cpp
    battery/upowerdevice.cpp
    battery/upowermanager.cpp
    brightness/brightnessmanager.cpp
    theme/thememanager.cpp
    language/language.cpp
    dock/dock.cpp
    mouse/mousemanager.cpp
    touchpad/touchpadmanager.cpp
)

qt6_add_dbus_adaptor(DBUS_SOURCES
                     brightness/com.cutefish.Brightness.xml
                     brightness/brightnessmanager.h BrightnessManager)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     theme/com.cutefish.Theme.xml
                     theme/thememanager.h ThemeManager)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     battery/com.cutefish.PrimaryBattery.xml
                     battery/battery.h Battery)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     language/com.cutefish.Language.xml
                     language/language.h Language)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     dock/com.cutefish.Dock.xml
                     dock/dock.h Dock)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     mouse/com.cutefish.Mouse.xml
                     mouse/mousemanager.h Mouse)
qt6_add_dbus_adaptor(DBUS_SOURCES
                     touchpad/com.cutefish.Touchpad.xml
                     touchpad/touchpadmanager.h TouchpadManager)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_include_directories(${TARGET} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${TARGET} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
    Qt6::Xml
)

install(TARGETS ${TARGET} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

file(GLOB TS_FILES translations/*.ts)
if(TS_FILES)
    qt_add_lrelease(${TARGET} QM_FILES ${TS_FILES})
    add_custom_target(settings-daemon-translations DEPENDS ${QM_FILES})
    add_dependencies(${TARGET} settings-daemon-translations)
    install(FILES ${QM_FILES} DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/${TARGET}/translations")
endif()
