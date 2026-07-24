cmake_minimum_required(VERSION 3.20)

project(ModuleTestCli LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")

add_executable(module_test_cli
    module_test_cli.cpp
    ModuleTestDispatcher.h
    ModuleTestDispatcher.cpp
    ${SRC_DIR}/Transport.h
    ${SRC_DIR}/Packet.h
    ${SRC_DIR}/KeyboardProtocol.h
    ${SRC_DIR}/KeyboardProtocol.cpp
    ${SRC_DIR}/MouseProtocol.h
    ${SRC_DIR}/MouseProtocol.cpp
    ${SRC_DIR}/MouseInputManager.h
    ${SRC_DIR}/MouseInputManager.cpp
    ${SRC_DIR}/HudpProtocol.h
    ${SRC_DIR}/HudpProtocol.cpp
    ${SRC_DIR}/InputQueue.h
    ${SRC_DIR}/InputQueue.cpp
    ${SRC_DIR}/TransportSelector.h
    ${SRC_DIR}/TransportSelector.cpp
    ${SRC_DIR}/AutoClickerManager.h
    ${SRC_DIR}/AutoClickerManager.cpp
    ${SRC_DIR}/LogManager.h
    ${SRC_DIR}/LogManager.cpp
    ${SRC_DIR}/KeySettingManager.h
    ${SRC_DIR}/KeySettingManager.cpp
    ${SRC_DIR}/SettingsManager.h
    ${SRC_DIR}/SettingsManager.cpp
    ${SRC_DIR}/DebugTrace.h
    ${SRC_DIR}/DebugTrace.cpp
)

target_include_directories(module_test_cli PRIVATE "${SRC_DIR}")
target_link_libraries(module_test_cli PRIVATE Qt6::Core)

enable_testing()
add_test(
    NAME module_cli_contracts
    COMMAND module_test_cli --verify "${CMAKE_CURRENT_SOURCE_DIR}/cases/unit_cases.jsonl"
)
set_tests_properties(module_cli_contracts PROPERTIES
    ENVIRONMENT "QT_LOGGING_RULES=*.debug=false"
    ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
)
