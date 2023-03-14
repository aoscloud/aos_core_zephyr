/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <unistd.h>

#include "cmclient.hpp"
#include "log.hpp"

using namespace aos::sm::launcher;

/***********************************************************************************************************************
 * Public
 **********************************************************************************************************************/

aos::Error CMClient::Init(LauncherItf& launcher)
{
    LOG_DBG() << "Initialize CM client";

    mLauncher = &launcher;

    auto err = mThread.Run([this](void*) { ProcessMessages(); });
    if (!err.IsNone()) {
        return err;
    }

    return aos::ErrorEnum::eNone;
}

aos::Error CMClient::InstancesRunStatus(const aos::Array<aos::InstanceStatus>& instances)
{
    (void)instances;

    return aos::ErrorEnum::eNone;
}

aos::Error CMClient::InstancesUpdateStatus(const aos::Array<aos::InstanceStatus>& instances)
{
    (void)instances;

    return aos::ErrorEnum::eNone;
}

/***********************************************************************************************************************
 * Private
 **********************************************************************************************************************/

void CMClient::ProcessMessages()
{
    auto i = 0;

    while (true) {
        LOG_DBG() << "Process message: " << i++;

        sleep(2);
    }
}
