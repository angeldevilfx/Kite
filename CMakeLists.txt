cmake_minimum_required(VERSION 3.21)

project(KiteBrowser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets WebEngineWidgets)

qt_standard_project_setup()

qt_add_executable(kitebrowser
    src/main.cpp
    src/browser/browser_tab.cpp
    src/browser/browser_window.cpp
)

target_include_directories(kitebrowser
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(kitebrowser
    PRIVATE
        Qt6::Widgets
        Qt6::WebEngineWidgets
)
