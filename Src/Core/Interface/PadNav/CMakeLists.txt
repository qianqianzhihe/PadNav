set(MODULE_NAME pn_pad_nav_core)
set(MODULE_ALIAS Pn::PadNavCore)

add_library(${MODULE_NAME} STATIC)
add_library(${MODULE_ALIAS} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME} PRIVATE
        mappingengine.cpp
        profile.cpp
)

target_include_directories(${MODULE_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)
