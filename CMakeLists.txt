cmake_minimum_required(VERSION 3.20)
project(iso-converter VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Widgets)

qt_add_executable(iso-converter
    src/main.cpp
    src/CueParser.h
    src/CueParser.cpp
    src/Converter.h
    src/Converter.cpp
    src/DropArea.h
    src/DropArea.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(iso-converter
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

set_target_properties(iso-converter PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE    TRUE
)

install(TARGETS iso-converter RUNTIME DESTINATION bin)
