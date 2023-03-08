/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LOG_HPP_
#define LOG_HPP_

#include "logger/logger.hpp"

#define LOG_DBG() LOG_MODULE_DBG(static_cast<aos::LogModule::EnumType>(Logger::Module::eResourceMgr))
#define LOG_INF() LOG_MODULE_INF(static_cast<aos::LogModule::EnumType>(Logger::Module::eResourceMgr))
#define LOG_WRN() LOG_MODULE_WRN(static_cast<aos::LogModule::EnumType>(Logger::Module::eResourceMgr))
#define LOG_ERR() LOG_MODULE_ERR(static_cast<aos::LogModule::EnumType>(Logger::Module::eResourceMgr))

#endif
