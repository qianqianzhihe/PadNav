set(MODULE_NAME pn_core)

add_subdirectory(Interface)
add_subdirectory(Network)
add_subdirectory(CommonUtils)

add_library(${MODULE_NAME} INTERFACE)
add_library(Pn::Core ALIAS ${MODULE_NAME})

target_link_libraries(${MODULE_NAME} INTERFACE
        Pn::Interface
        Pn::Network
        Pn::CommonUtils
)
