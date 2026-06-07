cmake_minimum_required(VERSION 3.21)

project(qt_url_browser VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets WebEngineWidgets)

qt_standard_project_setup()

qt_add_executable(qt_url_browser
    src/main.cpp
    src/browser/browser_tab.cpp
    src/browser/browser_window.cpp
)

target_include_directories(qt_url_browser
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(qt_url_browser
    PRIVATE
        Qt6::Widgets
        Qt6::WebEngineWidgets
)
