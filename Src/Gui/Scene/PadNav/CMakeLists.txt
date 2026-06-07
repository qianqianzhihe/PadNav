set(MODULE_NAME pn_pad_nav_gui)
set(MODULE_ALIAS Pn::PadNavGui)

add_library(${MODULE_NAME} STATIC)
add_library(${MODULE_ALIAS} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME} PRIVATE
        inputmonitorwindow.cpp
        inputmonitorwindow.h
        jsonprofilerepository.cpp
        jsonprofilerepository.h
        settingswindow.cpp
        settingswindow.h
        traycontroller.cpp
        traycontroller.h
)

target_link_libraries(${MODULE_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Widgets
        Pn::Application
        Pn::PadNavCore
)

target_include_directories(${MODULE_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_precompile_headers(${MODULE_NAME} PUBLIC
    "${CMAKE_SOURCE_DIR}/Src/App/pch.h"
)
