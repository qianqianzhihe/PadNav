set(MODULE_NAME pn_application)
set(MODULE_ALIAS Pn::Application)

add_library(${MODULE_NAME} STATIC
        padnavcontroller.cpp
)
add_library(${MODULE_ALIAS} ALIAS ${MODULE_NAME})

target_link_libraries(${MODULE_NAME}
    PUBLIC
        Pn::PadNavCore
    PRIVATE
        Pn::WindowsPlatform
)

target_include_directories(${MODULE_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)
