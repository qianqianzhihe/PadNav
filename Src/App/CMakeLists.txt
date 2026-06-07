set(MODULE_NAME padnav)
set(MODULE_ALIAS Pn::PadNav)

qt_add_resources(PADNAV_RESOURCES
        "${CMAKE_SOURCE_DIR}/Resources/images.qrc"
)

add_executable(${MODULE_NAME})
add_executable(${MODULE_ALIAS} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        main.cpp
        singleinstanceguard.cpp
        singleinstanceguard.h
        ${PADNAV_RESOURCES}
)

target_link_libraries(${MODULE_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Widgets
        Pn::PadNavGui
        Pn::Application
        Pn::WindowsPlatform
)

target_precompile_headers(${MODULE_NAME}
    PUBLIC
        "${CMAKE_SOURCE_DIR}/Src/App/pch.h"
)

copy_qt_libraries_after_build(${MODULE_NAME}
        COPY_WINDOWS_PLATFORM_PLUGIN
        COMPONENTS
            Core
            Gui
            Widgets
            Svg
            SvgWidgets
            Network
            LinguistTools
            Concurrent
)
