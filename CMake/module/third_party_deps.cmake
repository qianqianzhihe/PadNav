# Third-party dependency orchestration.
# Keep third-party paths and link details out of the root CMakeLists.txt.

set(OSS_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/oss")
set(OSS_INCLUDE_DIR "${OSS_ROOT}/include")
set(OSS_INCLUDE_DIR "${OSS_INCLUDE_DIR}" CACHE PATH "OSS Include Directory")

add_library(oss_sdk INTERFACE)
set(OSS_DLLS "" CACHE INTERNAL "OSS runtime DLLs")

if(EXISTS "${OSS_INCLUDE_DIR}")
    target_include_directories(oss_sdk INTERFACE "${OSS_INCLUDE_DIR}")
endif()

if(WIN32)
    set(OSS_LIB_DIR "${OSS_ROOT}/lib/win/x86_64/$<IF:$<CONFIG:Debug>,debug,release>")
    set(OSS_CORE_LIB "${OSS_ROOT}/lib/win/x86_64/release/alibabacloud-oss-cpp-sdk.lib")

    if(EXISTS "${OSS_CORE_LIB}")
        target_link_libraries(oss_sdk INTERFACE
                "${OSS_LIB_DIR}/alibabacloud-oss-cpp-sdk.lib"
                "${OSS_LIB_DIR}/libcurl.lib"
                "${OSS_LIB_DIR}/libeay32.lib"
                "${OSS_LIB_DIR}/ssleay32.lib"
        )

        set(OSS_DLLS
                "${OSS_LIB_DIR}/libcurl.dll"
                "${OSS_LIB_DIR}/libeay32.dll"
                "${OSS_LIB_DIR}/ssleay32.dll"
                "${OSS_LIB_DIR}/zlibwapi.dll"
                CACHE INTERNAL "OSS runtime DLLs"
        )
    else()
        message(STATUS "oss_sdk libraries not found, using empty oss_sdk target")
    endif()
else()
    set(CURL_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/curl")
    set(OPENSSL_ROOT "${CMAKE_SOURCE_DIR}/ThirdParty/openssl")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(OSS_LIB_DIR "${OSS_ROOT}/lib/linux/x64/debug")
        set(CURL_LIB_DIR "${CURL_ROOT}/lib/linux/x64/debug")
        set(OPENSSL_LIB_DIR "${OPENSSL_ROOT}/lib/linux/x64/debug")
    else()
        set(OSS_LIB_DIR "${OSS_ROOT}/lib/linux/x64/release")
        set(CURL_LIB_DIR "${CURL_ROOT}/lib/linux/x64/release")
        set(OPENSSL_LIB_DIR "${OPENSSL_ROOT}/lib/linux/x64/release")
    endif()

    set(CURL_INCLUDE_DIR "${CURL_ROOT}/include")
    set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT}/include")
    set(OSS_CORE_LIB "${OSS_LIB_DIR}/libalibabacloud-oss-cpp-sdk.a")

    if(EXISTS "${OSS_CORE_LIB}")
        if(EXISTS "${CURL_INCLUDE_DIR}")
            target_include_directories(oss_sdk INTERFACE "${CURL_INCLUDE_DIR}")
        endif()
        if(EXISTS "${OPENSSL_INCLUDE_DIR}")
            target_include_directories(oss_sdk INTERFACE "${OPENSSL_INCLUDE_DIR}")
        endif()

        target_link_libraries(oss_sdk INTERFACE
                "${OSS_LIB_DIR}/libalibabacloud-oss-cpp-sdk.a"
                "${CURL_LIB_DIR}/libcurl.a"
                "${OPENSSL_LIB_DIR}/libssl.a"
                "${OPENSSL_LIB_DIR}/libcrypto.a"
                psl
        )
    else()
        message(STATUS "oss_sdk libraries not found, using empty oss_sdk target")
    endif()
endif()
