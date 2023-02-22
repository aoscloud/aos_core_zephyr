#
# Copyright (C) 2023 Renesas Electronics Corporation.
# Copyright (C) 2023 EPAM Systems, Inc.
#
# SPDX-License-Identifier: Apache-2.0
#
#  ====================================================================
#
# GENERATE_API (public function)
# GENERATE_API (CORE_API_DIR)
#   CORE_API_DIR = Variabla to define dir with aos_core_api repository
#
#  ====================================================================

function(GENERATE_API CORE_API_DIR)
  set(NANOPB_OPTIONS "-I${CMAKE_CURRENT_BINARY_DIR}")

  set(NANOPB_IMPORT_DIRS ${CORE_API_DIR}/proto/servicemanager/v3 /usr/include/)

  nanopb_generate_cpp(SM_PROTO_SRCS SM_PROTO_HDRS ${CORE_API_DIR}/proto/servicemanager/v3/servicemanager.proto)

  nanopb_generate_cpp(TIME_PROTO_SRCS TIME_PROTO_HDRS RELPATH /usr/include /usr/include/google/protobuf/timestamp.proto)

  target_sources(app PRIVATE ${SM_PROTO_SRCS} ${TIME_PROTO_SRCS})

  file(COPY ${CORE_API_DIR}/vchanapi/vchanapi.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endfunction()
