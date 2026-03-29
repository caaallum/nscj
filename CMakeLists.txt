cmake_minimum_required(VERSION 4.2)

project(nscj)

option(UNICODE "Build with Unicode support" ON)

find_package(cJSON CONFIG REQUIRED)

file(GLOB SRC 
    nsis/api.h
    nsis/nsis_tchar.h
    nsis/pluginapi.h
    nsis/pluginapi.c

    src/http.h
    src/http.c
    src/util.h
    src/util.c
    src/tree.h
    src/tree.c
    src/nscj.c
)

message(STATUS "${CMAKE_CURRENT_LIST_DIR}")

add_library(${PROJECT_NAME} SHARED ${SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE cjson user32 winhttp)
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set(OUTPUT_PATH "")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    string(APPEND OUTPUT_PATH "x64")
else()
    string(APPEND OUTPUT_PATH "x86")
endif()

if (UNICODE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    string(APPEND OUTPUT_PATH "-unicode")
else()
    string(APPEND OUTPUT_PATH "-ansi")
endif()

foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${CONFIG} CONFIG_UPPER)
    set_target_properties(${PROJECT_NAME}
        PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/${OUTPUT_PATH}
    )
endforeach()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_package(cmocka CONFIG REQUIRED)

    set(TEST_SRC
        test/main.c
 
        nsis/pluginapi.c
 
        src/http.c
        src/util.c
        src/tree.c
        src/nscj.c
    )
 
    add_executable(nscj_test ${TEST_SRC})
 
    target_link_libraries(nscj_test PRIVATE cjson user32 winhttp cmocka::cmocka)
 
    target_include_directories(nscj_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/nsis
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
 
    if (UNICODE)
        target_compile_definitions(nscj_test PRIVATE UNICODE _UNICODE)
    endif()
 
    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${CONFIG} CONFIG_UPPER)
        set_target_properties(nscj_test PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER} ${CMAKE_BINARY_DIR}/${OUTPUT_PATH}
        )
    endforeach()
endif()