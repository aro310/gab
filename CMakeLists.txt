cmake_minimum_required(VERSION 3.16)
project(atm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_executable(atm
    main.cpp
    bridge/AtmBridge.hpp
)

qt_add_qml_module(atm
    URI AtmApp
    VERSION 1.0
    QML_FILES
        Main.qml
        MenuPage.qml
        HistoriquePage.qml
        MoneyRow.qml
)

target_include_directories(atm PRIVATE bridge core)
target_link_libraries(atm PRIVATE Qt6::Core Qt6::Gui Qt6::Quick)