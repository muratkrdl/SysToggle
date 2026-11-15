cmake_minimum_required(VERSION 3.10)

set(BOREALIS_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/library/borealis)
set(BOREALIS_LIBRARY ${BOREALIS_ROOT}/library)
set(BOREALIS_PATCH ${CMAKE_CURRENT_SOURCE_DIR}/patches/borealis-latin-default-font.patch)

execute_process(
    COMMAND git apply --ignore-whitespace --reverse --check ${BOREALIS_PATCH}
    WORKING_DIRECTORY ${BOREALIS_ROOT}
    RESULT_VARIABLE BOREALIS_PATCH_PRESENT
    OUTPUT_QUIET ERROR_QUIET)
if (NOT BOREALIS_PATCH_PRESENT EQUAL 0)
    execute_process(
        COMMAND git apply --ignore-whitespace ${BOREALIS_PATCH}
        WORKING_DIRECTORY ${BOREALIS_ROOT}
        RESULT_VARIABLE BOREALIS_PATCH_RESULT)
    if (NOT BOREALIS_PATCH_RESULT EQUAL 0)
        message(FATAL_ERROR "Could not apply ${BOREALIS_PATCH}")
    endif ()
    message(STATUS "Applied ${BOREALIS_PATCH}")
endif ()

include(${BOREALIS_LIBRARY}/cmake/commonOption.cmake)

option(USE_SHARED_LIB "Whether to use shared libs provided by system" OFF)
cmake_dependent_option(USE_SYSTEM_FMT "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_TINYXML2 "" OFF "NOT USE_SHARED_LIB" ON)
cmake_dependent_option(USE_SYSTEM_TWEENY "" OFF "NOT USE_SHARED_LIB" ON)

include(${BOREALIS_LIBRARY}/cmake/toolchain.cmake)

project(SysToggle)
set(VERSION_MAJOR "1")
set(VERSION_MINOR "0")
set(VERSION_ALTER "0")
set(VERSION_TEXT "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}")
set(PROJECT_AUTHOR "muratkrdl")
set(PROJECT_SOURCE_URL "github.com/muratkrdl/SysToggle")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icon/icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)

if (USE_LIBROMFS)
    add_libromfs(${PROJECT_NAME} ${PROJECT_RESOURCES})
endif ()

add_subdirectory(${BOREALIS_LIBRARY} borealis)

find_package(Threads REQUIRED)
list(APPEND APP_PLATFORM_LIB ${CMAKE_THREAD_LIBS_INIT})

file(GLOB_RECURSE MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp)

if (PLATFORM_SWITCH)
    list(APPEND MAIN_SRC ${BOREALIS_LIBRARY}/lib/platforms/switch/switch_wrapper.c)
endif ()

program_target(${PROJECT_NAME} "${MAIN_SRC}")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

if (PLATFORM_DESKTOP)
    add_custom_target(${PROJECT_NAME}.data
        COMMAND "${CMAKE_COMMAND}" -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_CURRENT_BINARY_DIR}/resources)
    if (NOT USE_LIBROMFS)
        add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}.data)
    endif ()
elseif (PLATFORM_SWITCH)
    set(STAGED_I18N ${CMAKE_BINARY_DIR}/resources/i18n)
    add_custom_target(${PROJECT_NAME}.nro DEPENDS ${PROJECT_NAME}
        COMMAND ${NX_NACPTOOL_EXE} --create "${PROJECT_NAME}" "${PROJECT_AUTHOR}" "${VERSION_TEXT}" ${PROJECT_NAME}.nacp
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES} ${CMAKE_BINARY_DIR}/resources
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/resources/icon
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${STAGED_I18N}/en-US ${STAGED_I18N}/en-GB
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${STAGED_I18N}/fr ${STAGED_I18N}/fr-CA
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${STAGED_I18N}/es ${STAGED_I18N}/es-419
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${STAGED_I18N}/pt ${STAGED_I18N}/pt-BR
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${STAGED_I18N}/zh-Hans ${STAGED_I18N}/zh-CN
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${STAGED_I18N}/zh-Hant ${STAGED_I18N}/zh-TW
        COMMAND ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro --icon=${PROJECT_ICON} --nacp=${PROJECT_NAME}.nacp --romfsdir=${CMAKE_BINARY_DIR}/resources)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/app ${APP_PLATFORM_INCLUDE})
target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_VERSION_TEXT="${VERSION_TEXT}"
    APP_AUTHOR_TEXT="${PROJECT_AUTHOR}"
    APP_SOURCE_URL="${PROJECT_SOURCE_URL}")
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -ffunction-sections -fdata-sections ${APP_PLATFORM_OPTION})
target_link_libraries(${PROJECT_NAME} PRIVATE borealis ${APP_PLATFORM_LIB})
