/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/fs/fs.h>

#include "log.hpp"
#include "resourcemanager.hpp"

static constexpr char UnitConfigFilePath[256] = "/tmp/unit_config.cfg"; // TODO define PATH

aos::Error ResourceManager::GetUnitConfigInfo(char* version) const
{
    struct fs_file_t file;

    fs_file_t_init(&file);

    auto ret = fs_open(&file, UnitConfigFilePath, FS_O_READ);
    if (ret < 0) {
        return ret;
    }

    if ((ret = fs_read(&file, version, sVersionLen)) < 0) {
        return ret;
    }

    if ((ret = fs_close(&file)) < 0) {
        return ret;
    }

    return aos::ErrorEnum::eNone;
}

aos::Error ResourceManager::CheckUnitConfig(const char* version, const char* unitConfig) const
{
    return aos::ErrorEnum::eNone;
}

aos::Error ResourceManager::UpdateUnitConfig(const char* version, const char* unitConfig)
{
    struct fs_file_t file;

    fs_file_t_init(&file);

    auto ret = fs_open(&file, UnitConfigFilePath, FS_O_CREATE | FS_O_WRITE);
    if (ret < 0) {
        return ret;
    }

    if ((ret = fs_write(&file, version, strlen(version)) < 0)) {
        return ret;
    }

    if ((ret = fs_close(&file)) < 0) {
        return ret;
    }

    return aos::ErrorEnum::eNone;
}
