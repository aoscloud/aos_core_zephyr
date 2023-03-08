/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef RESOURCEMANAGER_HPP_
#define RESOURCEMANAGER_HPP_

#include <aos/common/error.hpp>
#include <aos/common/noncopyable.hpp>

/**
 * Resource manager instance.
 */

class ResourceManager : public aos::NonCopyable {
public:
    /**
     * Get current unit config version.
     * 
     * @param version  [out] param to store version.
     * @return aos::Error.
     */
    aos::Error GetUnitConfigInfo(char* version) const;

    /**
     * Check new unit configuration.
     * 
     * @param version unit config version
     * @param unitConfig string with unit configuration.
     * @return aos::Error.
     */
    aos::Error CheckUnitConfig(const char* version, const char* unitConfig) const;

    /**
     * Update unit configuration.
     * 
     * @param version unit config version.
     * @param unitConfig string with unit configuration.
     * @return aos::Error.
     */
    aos::Error UpdateUnitConfig(const char* version, const char* unitConfig);

private:
    static constexpr size_t sVersionLen = 6;
    static constexpr size_t sUnitConfigMaxSize = 1024;
};

#endif
