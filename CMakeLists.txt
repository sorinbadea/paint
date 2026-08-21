cmake_minimum_required(VERSION 3.16)
project(QtTwoExecutables LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic build tools (required if using Q_OBJECT, UI files, or resources)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 1. Find the required Qt Widgets module (supports both Qt 5 and Qt 6)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# ----------------------------------------------------
# Binary 1: First Executable
# ----------------------------------------------------
add_executable(QtPaint
    src/drawing_canvas.cpp src/shapes.cpp src/main.cpp
)

target_link_libraries(QtPaint PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)
