# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr COMPONENTS REQUIRED HINTS $ENV{ZEPHYR_BASE})

set(CMAKE_MODULE_PATH ${APPLICATION_SOURCE_DIR}/../../CMakeModules)

find_package(CoreAPI)

project(cmclient_test)

# ######################################################################################################################
# Includes
# ######################################################################################################################

target_include_directories(app PRIVATE ${APPLICATION_SOURCE_DIR}/../../src)
target_include_directories(app PRIVATE ${APPLICATION_SOURCE_DIR}/../../../aos_core_lib_cpp/include)
target_include_directories(app PRIVATE ${APPLICATION_SOURCE_DIR}/../../../zephyr-xenlib/vch/include)
target_include_directories(app PRIVATE ${APPLICATION_SOURCE_DIR}/../../../modules/lib/nanopb)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# ######################################################################################################################
# Target
# ######################################################################################################################

generate_api(${APPLICATION_SOURCE_DIR}/../../../aos_core_api)

target_sources(app PRIVATE src/main.cpp ../../src/cmclient/cmclient.cpp)
