function(copy_library_after_build target_name output_dir)
    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${output_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "$<TARGET_FILE:${target_name}>"
                    "${output_dir}"
            COMMENT "Copying target ${target_name} to ${output_dir}"
            VERBATIM
    )
endfunction()

function(copy_qt_libraries_after_build target_name)
    set(options COPY_WINDOWS_PLATFORM_PLUGIN)
    set(one_value_args OUTPUT_DIR)
    set(multi_value_args COMPONENTS)

    cmake_parse_arguments(
            COPY_QT
            "${options}"
            "${one_value_args}"
            "${multi_value_args}"
            ${ARGN}
    )

    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target '${target_name}' does not exist.")
    endif()

    if(NOT DEFINED QT_VERSION_MAJOR)
        message(FATAL_ERROR "QT_VERSION_MAJOR is not defined. Call find_package(QT ...) before copy_qt_libraries_after_build().")
    endif()

    if(NOT COPY_QT_OUTPUT_DIR)
        set(COPY_QT_OUTPUT_DIR "$<TARGET_FILE_DIR:${target_name}>")
    endif()

    if(NOT COPY_QT_COMPONENTS)
        set(COPY_QT_COMPONENTS Core Gui Widgets)
    endif()

    add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${COPY_QT_OUTPUT_DIR}"
            COMMENT "Preparing Qt runtime output directory for ${target_name}"
            VERBATIM
    )

    foreach(qt_component IN LISTS COPY_QT_COMPONENTS)
        set(qt_target "Qt${QT_VERSION_MAJOR}::${qt_component}")

        if(TARGET ${qt_target})
            add_custom_command(TARGET ${target_name} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "$<TARGET_FILE:${qt_target}>"
                            "${COPY_QT_OUTPUT_DIR}"
                    COMMENT "Copying ${qt_target} runtime library for ${target_name}"
                    VERBATIM
            )
        else()
            message(STATUS "Skipping Qt runtime component '${qt_component}' because target '${qt_target}' does not exist.")
        endif()
    endforeach()

    if(WIN32 AND COPY_QT_COPY_WINDOWS_PLATFORM_PLUGIN)
        set(qt_core_target "Qt${QT_VERSION_MAJOR}::Core")

        if(NOT TARGET ${qt_core_target})
            message(FATAL_ERROR "Target '${qt_core_target}' does not exist.")
        endif()

        set(qt_windows_platform_plugin
                "$<TARGET_FILE_DIR:${qt_core_target}>/../plugins/platforms/$<IF:$<CONFIG:Debug>,qwindowsd.dll,qwindows.dll>"
        )

        add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "${COPY_QT_OUTPUT_DIR}/platforms"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${qt_windows_platform_plugin}"
                        "${COPY_QT_OUTPUT_DIR}/platforms"
                COMMENT "Copying Qt Windows platform plugin for ${target_name}"
                VERBATIM
        )
    endif()
endfunction()
