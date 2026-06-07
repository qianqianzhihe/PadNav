set(MODULE_NAME pn_common_widget)
set(MODULE_ALIAS Pn::CommonWidget)

set(MODULE_SOURCES
)

add_library(${MODULE_NAME} STATIC)

add_library(${MODULE_ALIAS} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME}
    PRIVATE
        ${MODULE_SOURCES}
)

target_include_directories(${MODULE_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${MODULE_NAME}
    PUBLIC
        Qt${QT_VERSION_MAJOR}::Core
    PRIVATE
)

target_precompile_headers(${MODULE_NAME}
    PUBLIC
        "${CMAKE_SOURCE_DIR}/Src/App/pch.h"
)
