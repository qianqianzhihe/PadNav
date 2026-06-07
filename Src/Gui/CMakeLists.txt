set(MODULE_NAME pn_gui)

add_subdirectory(Scene)
add_subdirectory(CommonWidget)

add_library(${MODULE_NAME} INTERFACE)
add_library(Pn::Gui ALIAS ${MODULE_NAME})

target_link_libraries(${MODULE_NAME} INTERFACE
        Pn::Scene
        Pn::CommonWidget
)
