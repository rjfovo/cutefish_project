project(cutefish-session LANGUAGES CXX)
set(TARGET cutefish-session)

find_package(Qt6 REQUIRED COMPONENTS Core DBus)

set(SOURCES
    main.cpp
    sessionapplication.cpp
    processmanager.cpp
    networkproxymanager.cpp
    powermanager/power.cpp
)

qt6_add_dbus_adaptor(DBUS_SOURCES
    com.cutefish.Session.xml
    sessionapplication.h
    SessionApplication
    sessionadaptor
    SessionAdaptor
)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${TARGET} ${SOURCES} ${DBUS_SOURCES})
target_include_directories(${TARGET} PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${TARGET} PRIVATE Qt6::Core Qt6::DBus)
install(TARGETS ${TARGET} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
