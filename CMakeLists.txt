#
# Copyright (C) 2023 Renesas Electronics Corporation.
# Copyright (C) 2023 EPAM Systems, Inc.
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

set(UTILS_SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
if (DTC)
  add_custom_target(
    fix_dtb
    DEPENDS ${ZEPHYR_FINAL_EXECUTABLE}
    COMMAND ${UTILS_SCRIPTS_DIR}/fix_dtb.sh ${UTILS_SCRIPTS_DIR}/gicv2.tmpl ${ZEPHYR_BINARY_DIR}/virt_gicv2.dtb ${ZEPHYR_BINARY_DIR}/zephyr.bin)
else()
  message(WARNING "No dtc program")
endif()

project(aoszephyrapp)

# ######################################################################################################################
# Includes
# ######################################################################################################################

zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR} src)
# Add prebuilt to includes so assembler can find this files
include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt)

# ######################################################################################################################
# Taget
# ######################################################################################################################

target_sources(
    app PRIVATE src/main.cpp src/logger/logger.cpp src/app/app.cpp src/cmclient/cmclient.cpp src/runner/runner.cpp
                src/storage/storage.cpp src/domain_bins.S
)

# ######################################################################################################################
# Versioning
# ######################################################################################################################

find_package(Git)

if(NOT GIT_FOUND)
    set(GIT_EXECUTABLE git)
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

add_custom_target(
    version
    COMMAND
        ${CMAKE_COMMAND} -D GIT_EXECUTABLE=${GIT_EXECUTABLE} -D INPUT_FILE=${CMAKE_SOURCE_DIR}/src/version.hpp.in -D
        OUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/version.hpp -D GIT_SOURCE_DIR=${CMAKE_SOURCE_DIR} -P
        ${CMAKE_MODULE_PATH}/GenerateVersion.cmake
)

add_dependencies(app version)

# ######################################################################################################################
# External libs
# ######################################################################################################################

include(ExternalProject)

zephyr_get_include_directories_for_lang_as_string(CXX includes)
zephyr_get_system_include_directories_for_lang_as_string(CXX system_includes)
zephyr_get_compile_definitions_for_lang_as_string(CXX definitions)
zephyr_get_compile_options_for_lang_as_string(CXX options)

set(external_project_cxx_flags "${includes} ${system_includes} ${definitions} ${options}")

# ######################################################################################################################
# aos_core_lib_cpp
# ######################################################################################################################

set(aos_cxx_flags "${external_project_cxx_flags} -Wno-pedantic -Wno-error")

# Aos core configuration

if(NOT BOARD MATCHES "native_posix*")
    zephyr_compile_definitions(-DAOS_CONFIG_DEFAULT_THREAD_STACK_SIZE=1024)
endif()

set(AOSCORE_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/aoscore)

ExternalProject_Add(
    aoscore
    PREFIX ${AOSCORE_BUILD_DIR}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${AOSCORE_BUILD_DIR} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
               -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
               -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR} -DCMAKE_CXX_FLAGS=${aos_cxx_flags}
    SOURCE_DIR ${APPLICATION_SOURCE_DIR}/../aos_core_lib_cpp
    BUILD_BYPRODUCTS ${AOSCORE_BUILD_DIR}/lib/libaossmcpp.a ${AOSCORE_BUILD_DIR}/lib/libaosiamcpp.a
)

# aossm

file(MAKE_DIRECTORY ${AOSCORE_BUILD_DIR}/include)

add_library(aossm STATIC IMPORTED GLOBAL)

add_dependencies(aossm aoscore)

set_target_properties(aossm PROPERTIES IMPORTED_LOCATION ${AOSCORE_BUILD_DIR}/lib/libaossmcpp.a)
set_target_properties(aossm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${AOSCORE_BUILD_DIR}/include)

target_link_libraries(app PUBLIC aossm)
