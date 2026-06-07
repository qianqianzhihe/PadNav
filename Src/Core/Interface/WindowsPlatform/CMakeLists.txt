set(MODULE_NAME pn_windows_platform)
set(MODULE_ALIAS Pn::WindowsPlatform)

add_library(${MODULE_NAME} STATIC)
add_library(${MODULE_ALIAS} ALIAS ${MODULE_NAME})

target_sources(${MODULE_NAME} PRIVATE
        crashdump.cpp
        crashdump.h
        sendinputemitter.cpp
        xinputcontroller.cpp
)

target_include_directories(${MODULE_NAME} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${MODULE_NAME}
    PUBLIC
        Pn::PadNavCore
    PRIVATE
        DbgHelp
        Shell32
        XInput
)
