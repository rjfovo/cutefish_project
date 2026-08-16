cmake_minimum_required(VERSION 3.16)
project(cutefish-notificationd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core DBus LinguistTools)

set(SRCS
    application.cpp
    main.cpp
    datehelper.cpp
    notificationserver.cpp
    notificationsmodel.cpp
    notification.cpp
    historymodel.cpp
    settings.cpp
    utils.cpp
    dbus/notificationsadaptor.cpp
)

qt6_add_dbus_adaptor(DBUS_SOURCES
    com.cutefish.Notification.xml
    application.h
    Application
)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(cutefish-notificationd ${SRCS} ${DBUS_SOURCES})
target_include_directories(cutefish-notificationd PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/dbus
)
target_link_libraries(cutefish-notificationd PRIVATE Qt6::Core Qt6::DBus)

install(TARGETS cutefish-notificationd RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
