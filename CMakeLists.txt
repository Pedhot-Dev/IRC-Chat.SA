cmake_minimum_required(VERSION 3.15)
project(IRCChat.SA LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(PLUGIN_SDK_URL "https://github.com/juicermv/plugin-sdk/releases/latest/download/msvc.zip")

FetchContent_Declare(plugin_sdk
        URL ${PLUGIN_SDK_URL}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(plugin_sdk)
add_library(${PROJECT_NAME} SHARED
        src/Main.cpp
        src/IRCClient.h
        src/IRCClient.cpp
        src/util/Config.h
        src/util/Config.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME}
        SUFFIX ".asi"
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/sampapi/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/mini/src"
)

target_include_directories(${PROJECT_NAME} PRIVATE
        "${plugin_sdk_SOURCE_DIR}/src"
        "${plugin_sdk_SOURCE_DIR}/src/plugin_sa"
        "${plugin_sdk_SOURCE_DIR}/src/plugin_sa/game_sa"
        "${plugin_sdk_SOURCE_DIR}/src/plugin_sa/game_sa/rw"
        "${plugin_sdk_SOURCE_DIR}/src/shared"
        "${plugin_sdk_SOURCE_DIR}/src/shared/game"
)

target_link_directories(${PROJECT_NAME} PRIVATE
        "${plugin_sdk_SOURCE_DIR}/lib/plugin_sa"
        "${CMAKE_CURRENT_SOURCE_DIR}/external/sampapi"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_NON_CONFORMING_SWPRINTFS
        GTASA
        "GTAGAME_NAME=\"San Andreas\""
        "GTAGAME_ABBR=\"SA\""
        "GTAGAME_ABBRLOW=\"sa\""
        "GTAGAME_PROTAGONISTNAME=\"CJ\""
        "GTAGAME_CITYNAME=\"San Andreas\""
        _LA_SUPPORT
        _DX9_SDK_INSTALLED
        PLUGIN_SGV_10US
        _USE_MATH_DEFINES
        RW
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:_NDEBUG>
)

if(MSVC)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(FATAL_ERROR "This project must be compiled as 32-bit (x86). Please use the correct generator platform (e.g., -A Win32 for Visual Studio).")
    endif()

    target_compile_options(${PROJECT_NAME} PRIVATE
            /std:c++latest /W3 /EHsc /MT
            /Ox /Ob1 /GT /fp:fast /Gy /Oi
    )

    target_link_options(${PROJECT_NAME} PRIVATE
            /SUBSYSTEM:WINDOWS /DLL
            /OPT:ICF /OPT:REF
            "/NODEFAULTLIB:MSVCRTD"
    )

    # MSVC Libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
            plugin.lib sampapi.lib ws2_32
    )
else()
    message(WARNING "Compiler not recognized. Build might fail. Supported compilers: MSVC, MinGW/GCC.")
endif()

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/IRCChat.SA.ini"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/IRCChat.SA.ini"
        COMMENT "Copying additional files to output directory"
)