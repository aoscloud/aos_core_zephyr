#
# Copyright (C) 2023 Renesas Electronics Corporation.
# Copyright (C) 2023 EPAM Systems, Inc.
#
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

project(aoszephyrapp)

# ######################################################################################################################
# Config
# ######################################################################################################################

set(AOS_CORE_CONFIG aoscoreconfig.hpp)

# ######################################################################################################################
# Includes
# ######################################################################################################################

zephyr_include_directories(${CMAKE_CURRENT_BINARY_DIR} src)

if(BOARD MATCHES "native_posix*")
    zephyr_include_directories(mocks/include)
endif()

if(TA_DEPLOY_DIR)
    set(TA_BIN_DIRECTORY ta_bin)
    set(GENERATED_DIRECTORY ta_table)
    execute_process(
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/prepare_ta.sh -t ${CMAKE_CURRENT_SOURCE_DIR} -f ${TA_DEPLOY_DIR} -b
                ${TA_BIN_DIRECTORY} -g ${GENERATED_DIRECTORY}
    )
    include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/${TA_BIN_DIRECTORY})
    target_sources(app PRIVATE ${GENERATED_DIRECTORY}/ta_table.c ${GENERATED_DIRECTORY}/ta_table.S)
endif()

# ######################################################################################################################
# Definitions
# ######################################################################################################################

# Aos core configuration
add_definitions(-include ${AOS_CORE_CONFIG})

# ######################################################################################################################
# Compile options
# ######################################################################################################################

# Add stack usage warning
target_compile_options(app PRIVATE -Wstack-usage=2048)

# ######################################################################################################################
# Taget
# ######################################################################################################################

target_sources(
    app
    PRIVATE src/main.cpp
            src/logger/logger.cpp
            src/app/app.cpp
            src/cmclient/cmclient.cpp
            src/runner/runner.cpp
            src/storage/storage.cpp
            src/downloader/downloader.cpp
            src/resourcemanager/resourcemanager.cpp
            src/ocispec/ocispec.cpp
            src/resourceusageprovider/resourceusageprovider.cpp
            src/utils/checksum.cpp
)

# Enable vchannels and xrun mocks for native posix
target_sources_ifdef(CONFIG_NATIVE_APPLICATION app PRIVATE mocks/vch/vch.cpp mocks/xrun/xrun.cpp mocks/xstat/xstat.cpp)

# Include domain IPL and dtb for xen_dom0 shield
target_sources_ifdef(CONFIG_XEN_DOM0 app PRIVATE src/domains/domd/domain_bins.S)

# Include source files for DomD support

target_sources_ifdef(CONFIG_SOC_SERIES_RCAR_GEN4 app PRIVATE src/domains/domd/domd_cfg_spider.c)
target_sources_ifdef(CONFIG_BOARD_RCAR_SALVATOR_XS_M3 app PRIVATE src/domains/domd/domd_cfg_salvator_xs.c)
target_sources_ifdef(CONFIG_BOARD_RCAR_H3ULCB_CA57 app PRIVATE src/domains/domd/domd_cfg_h3ulcb.c)

# Include source files for qemu boards.
target_sources_ifdef(CONFIG_BOARD_QEMU_CORTEX_A53 app PRIVATE src/domains/domd/domd_cfg_xenvm.c)

# Include HW-specific source file
target_sources_ifndef(CONFIG_NATIVE_APPLICATION app PRIVATE src/bsp/reboot.c src/bsp/mount.c)

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
#  Generate API
# ######################################################################################################################

find_package(CoreAPI)

set(CORE_API_CXX_FLAGS -I${CMAKE_CURRENT_SOURCE_DIR}/src -I${CMAKE_CURRENT_SOURCE_DIR}/../aos_core_lib_cpp/include
                       -include ${CMAKE_BINARY_DIR}/zephyr/include/generated/autoconf.h -include ${AOS_CORE_CONFIG}
)

core_api_generate(${CMAKE_CURRENT_SOURCE_DIR}/../aos_core_api ${CMAKE_CURRENT_SOURCE_DIR}/scripts)

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

set(aos_cxx_flags "${external_project_cxx_flags} -Wno-pedantic -Wno-error -include ${AOS_CORE_CONFIG}")
set(aoscore_build_dir ${CMAKE_CURRENT_BINARY_DIR}/aoscore)

ExternalProject_Add(
    aoscore
    PREFIX ${aoscore_build_dir}
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${aoscore_build_dir} -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
               -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
               -DCMAKE_SYSTEM_PROCESSOR=${CMAKE_SYSTEM_PROCESSOR} -DCMAKE_CXX_FLAGS=${aos_cxx_flags}
    SOURCE_DIR ${APPLICATION_SOURCE_DIR}/../aos_core_lib_cpp
    BUILD_BYPRODUCTS ${aoscore_build_dir}/lib/libaossmcpp.a ${aoscore_build_dir}/lib/libaosiamcpp.a
                     ${aoscore_build_dir}/lib/libaoscommoncpp.a
    DEPENDS zephyr_interface
)

# aoscommon
add_library(aoscommon STATIC IMPORTED GLOBAL)

add_dependencies(aoscommon aoscore)

set_target_properties(aoscommon PROPERTIES IMPORTED_LOCATION ${aoscore_build_dir}/lib/libaoscommoncpp.a)
set_target_properties(aoscommon PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${aoscore_build_dir}/include)

target_link_libraries(app PUBLIC aoscommon)

# aossm
file(MAKE_DIRECTORY ${aoscore_build_dir}/include)

add_library(aossm STATIC IMPORTED GLOBAL)

add_dependencies(aossm aoscore)

set_target_properties(aossm PROPERTIES IMPORTED_LOCATION ${aoscore_build_dir}/lib/libaossmcpp.a)
set_target_properties(aossm PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${aoscore_build_dir}/include)

target_link_libraries(app PUBLIC aossm)
