cmake_minimum_required(VERSION 3.16)

project(cutefish-installer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick QuickControls2)

set(INSTALLER_QML
    qml/Main.qml
    qml/Welcome.qml
    qml/Disk.qml
    qml/PartitionAdvanced.qml
    qml/Timezone.qml
    qml/User.qml
    qml/Summary.qml
    qml/Progress.qml
    qml/Finish.qml
)

qt_add_executable(cutefish-installer
    main.cpp
    installerbackend.cpp
    installerbackend.h
)

qt_add_resources(cutefish-installer "installer_qml"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES ${INSTALLER_QML}
)

target_link_libraries(cutefish-installer PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::QuickControls2
)

install(TARGETS cutefish-installer RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES config/install.conf DESTINATION "${CMAKE_INSTALL_DATADIR}/cutefish-installer")

include(CTest)
if(BUILD_TESTING)
    add_test(
        NAME installer_qml_offscreen_smoke
        COMMAND cutefish-installer --self-test
    )
    set_tests_properties(installer_qml_offscreen_smoke PROPERTIES
        TIMEOUT 15
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software"
    )
endif()
