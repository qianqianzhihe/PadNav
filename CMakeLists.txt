cmake_minimum_required(VERSION 3.21)

project(PadNav VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options("/utf-8")
endif()

# set gcc/qt install path
# Modify these paths if the local compiler or Qt install location differs.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_C_COMPILER /usr/bin/gcc)
    set(CMAKE_PREFIX_PATH "/home/chenml/Software/qt/6.7.3/gcc_64")
else()
    set(CMAKE_PREFIX_PATH "D:/software/Qt/6.8.3/msvc2022_64")
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Svg
        SvgWidgets
        Network
        LinguistTools
        Concurrent
)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
        Core
        Gui
        Widgets
        Svg
        SvgWidgets
        Network
        LinguistTools
        Concurrent
)

include(CMake/module/third_party_deps.cmake)
include(CMake/module/copy_build_lib_file.cmake)
include(CTest)

add_subdirectory(Src)
add_subdirectory(Tests)
