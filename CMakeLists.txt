cmake_minimum_required(VERSION 3.16)
project(cpu-scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(cpu-scheduler
    main.cpp
    mainwindow.cpp
    mainwindow.h
    scheduler.cpp
    scheduler.hpp
)

target_link_libraries(cpu-scheduler PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(MSVC)
    target_compile_options(cpu-scheduler PRIVATE /W4 /permissive-)
else()
    target_compile_options(cpu-scheduler PRIVATE -Wall -Wextra -Wpedantic)
endif()
