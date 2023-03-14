/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>

#include "logger.hpp"

/***********************************************************************************************************************
 * Log module callbacks
 **********************************************************************************************************************/

#define LOG_CALLBACK(name)                                                                                             \
    namespace log_##name                                                                                               \
    {                                                                                                                  \
        LOG_MODULE_REGISTER(name, CONFIG_LOG_DEFAULT_LEVEL);                                                           \
                                                                                                                       \
        static void LogCallback(aos::LogLevel level, const aos::String& message)                                       \
        {                                                                                                              \
            switch (level.GetValue()) {                                                                                \
            case aos::LogLevelEnum::eDebug:                                                                            \
                LOG_DBG("%s", message.CStr());                                                                         \
                                                                                                                       \
                break;                                                                                                 \
                                                                                                                       \
            case aos::LogLevelEnum::eInfo:                                                                             \
                LOG_INF("%s", message.CStr());                                                                         \
                                                                                                                       \
                break;                                                                                                 \
                                                                                                                       \
            case aos::LogLevelEnum::eWarning:                                                                          \
                LOG_WRN("%s", message.CStr());                                                                         \
                                                                                                                       \
                break;                                                                                                 \
                                                                                                                       \
            case aos::LogLevelEnum::eError:                                                                            \
                LOG_ERR("%s", message.CStr());                                                                         \
                                                                                                                       \
                break;                                                                                                 \
                                                                                                                       \
            default:                                                                                                   \
                LOG_ERR("Unknown log level received: %s", level.ToString().CStr());                                    \
                                                                                                                       \
                break;                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }

LOG_CALLBACK(app);
LOG_CALLBACK(cmclient);
LOG_CALLBACK(launcher);
LOG_CALLBACK(runner);
LOG_CALLBACK(storage);

/***********************************************************************************************************************
 * Public
 **********************************************************************************************************************/

void Logger::Init()
{
    aos::Log::SetCallback(LogCallback);
}

/***********************************************************************************************************************
 * Public
 **********************************************************************************************************************/

void Logger::LogCallback(aos::LogModule module, aos::LogLevel level, const aos::String& message)
{
    switch (static_cast<int>(module.GetValue())) {
    case static_cast<int>(Module::eApp):
        log_app::LogCallback(level, message);

        break;

    case static_cast<int>(Module::eCMClient):
        log_cmclient::LogCallback(level, message);

        break;

    case static_cast<int>(Module::eRunner):
        log_runner::LogCallback(level, message);

        break;

    case static_cast<int>(Module::eStorage):
        log_storage::LogCallback(level, message);

        break;

    case static_cast<int>(aos::LogModuleEnum::eSMLauncher):
        log_launcher::LogCallback(level, message);

        break;

    default:
        __ASSERT(false, "Log from unknown module received: %s", module.ToString().CStr());
    }
}
