set(MODULE_NAME padnav_tests)

add_executable(${MODULE_NAME}
        main.cpp
        Application/padnavcontrollertests.cpp
        PadNavCore/mappingenginetests.cpp
        PadNavCore/profiletests.cpp
)
add_executable(Pn::PadNav::Tests ALIAS ${MODULE_NAME})

target_include_directories(${MODULE_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${MODULE_NAME} PRIVATE
        Pn::PadNavCore
        Pn::Application
)

add_test(NAME padnav_tests COMMAND padnav_tests)
