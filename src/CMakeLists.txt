cmake_minimum_required(VERSION 3.20)

project(wirelesskm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

if(APPLE)
    set(APPLICATION_DIR_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/MacOS)
else()
    set(APPLICATION_DIR_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Quick SerialPort Bluetooth Network Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Quick SerialPort Bluetooth Network Widgets PrintSupport)

find_program(QT_LUPDATE NAMES lupdate)
find_program(QT_LRELEASE NAMES lrelease)
file(GLOB TS_FILE_PATHS ${CMAKE_CURRENT_LIST_DIR}/qml/*.ts)
add_custom_target(Script-UpdateTranslations
    COMMAND ${QT_LUPDATE} ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/qml -ts ${PROJECT_NAME}_en_US.ts WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/qml
    COMMAND ${QT_LUPDATE} ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/qml -ts ${PROJECT_NAME}_zh_CN.ts WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/qml
    COMMAND ${QT_LRELEASE} ${PROJECT_NAME}_en_US.ts WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/qml
    COMMAND ${QT_LRELEASE} ${PROJECT_NAME}_zh_CN.ts WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/qml
    COMMAND ${CMAKE_COMMAND} -E make_directory ${APPLICATION_DIR_PATH}/i18n
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_NAME}_en_US.qm ${PROJECT_NAME}_zh_CN.qm ${APPLICATION_DIR_PATH}/i18n
    SOURCES ${TS_FILE_PATHS}
)

set(PROJECT_SOURCES
    main.cpp
    DebugTrace.h
    DebugTrace.cpp
    KeyboardProtocol.h
    KeyboardProtocol.cpp
    MouseProtocol.h
    MouseProtocol.cpp
    MouseInputManager.h
    MouseInputManager.cpp
    Packet.h
    Transport.h
    SerialTransport.h
    SerialTransport.cpp
    BleTransport.h
    BleTransport.cpp
    HudpProtocol.h
    HudpProtocol.cpp
    HudpTransport.h
    HudpTransport.cpp
    TransportSelector.h
    TransportSelector.cpp
    InputQueue.h
    InputQueue.cpp
    InputOutputManager.h
    InputOutputManager.cpp
    AutoClickerManager.h
    AutoClickerManager.cpp
    TextInputSource.h
    TextInputSource.cpp
    ClipboardTextSource.h
    ClipboardTextSource.cpp
    LogManager.h
    LogManager.cpp
    SettingsManager.h
    SettingsManager.cpp
    ConnectionStateManager.h
    ConnectionStateManager.cpp
    InterfaceLayer.h
    InterfaceLayer.cpp
    InterfaceLayer_routes_key.cpp
    InterfaceLayer_routes_hudp.cpp
    InterfaceLayer_routes_ble.cpp
    InterfaceLayer_routes_uart.cpp
    InterfaceLayer_routes_text.cpp
    InterfaceLayer_routes_clipboard.cpp
    InterfaceLayer_routes_clicker.cpp
    InterfaceLayer_routes_mouse.cpp
    InterfaceLayer_routes_log.cpp
    InterfaceLayer_routes_app.cpp
    InterfaceLayer_routes_settings.cpp
    SerialManager.h
    SerialManager.cpp
    CaptureManager.h
    CaptureManager.cpp
    KeySettingManager.h
    KeySettingManager.cpp
    BleManager.h
    BleManager.cpp
    BleSystemRouter.h
    BleSystemRouter.cpp
    qml/qml.qrc
)

if(WIN32)
    list(APPEND PROJECT_SOURCES
        WinRtBleTransport.h
        WinRtBleTransport.cpp
    )
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
    )
endif()

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE app.rc)
endif()

add_dependencies(${PROJECT_NAME} Script-UpdateTranslations)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::SerialPort
    Qt${QT_VERSION_MAJOR}::Bluetooth
    Qt${QT_VERSION_MAJOR}::Network
    fluentuiplugin
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_qml_plugins(${PROJECT_NAME})
    qt_finalize_executable(${PROJECT_NAME})
endif()

if (CMAKE_BUILD_TYPE MATCHES "Release")
    if (APPLE)
        find_program(QT_DEPLOY_QT NAMES macdeployqt)
        add_custom_target(Script-DeployRelease
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${QT_DEPLOY_QT} ${CMAKE_SOURCE_DIR}/dist/${PROJECT_NAME}.app -qmldir=${CMAKE_CURRENT_LIST_DIR}/qml
            COMMENT "MacOs Deploying Qt Dependencies After Build........."
            SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
    endif()
    if(WIN32)
        find_program(QT_DEPLOY_QT NAMES windeployqt)
        add_custom_target(Script-DeployRelease
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${CMAKE_SOURCE_DIR}/dist
            COMMAND ${QT_DEPLOY_QT} --qmldir=${CMAKE_CURRENT_LIST_DIR}/qml --plugindir ${CMAKE_SOURCE_DIR}/dist/plugins --no-translations --compiler-runtime ${CMAKE_SOURCE_DIR}/dist/$<TARGET_FILE_NAME:${PROJECT_NAME}>
            COMMENT "Windows Deploying Qt Dependencies After Build........."
            SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )
endif()
endif()
