set(MODULE_NAME pn_common_utils)
set(MODULE_ALIAS Pn::CommonUtils)

add_library(${MODULE_NAME} INTERFACE)

add_library(${MODULE_ALIAS} ALIAS ${MODULE_NAME})

target_include_directories(${MODULE_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)
