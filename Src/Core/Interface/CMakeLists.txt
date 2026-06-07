set(MODULE_NAME pn_interface)
set(MODULE_ALIAS Pn::Interface)

add_library(${MODULE_NAME} INTERFACE)

add_library(${MODULE_ALIAS} ALIAS ${MODULE_NAME})

target_include_directories(${MODULE_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

add_subdirectory(PadNav)
add_subdirectory(WindowsPlatform)
add_subdirectory(Application)

target_link_libraries(${MODULE_NAME}
    INTERFACE
        Pn::Application
        Pn::PadNavCore
        Pn::WindowsPlatform
)
