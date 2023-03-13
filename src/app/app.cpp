/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "app.hpp"
#include "log.hpp"

/***********************************************************************************************************************
 * Variables
 **********************************************************************************************************************/

App App::sApp;

/***********************************************************************************************************************
 * Public
 **********************************************************************************************************************/

aos::Error App::Init()
{
    LOG_INF() << "Initialize application";

    aos::Error err;

    if (!(err = mStorage.Init()).IsNone()) {
        return err;
    }

    if (!(err = mRunner.Init(mLauncher)).IsNone()) {
        return err;
    }

    if (!(err = mLauncher.Init(mCMClient, mRunner, mStorage)).IsNone()) {
        return err;
    }

    if (!(err = mCMClient.Init(mLauncher, mResourceManager)).IsNone()) {
        return err;
    }

    return aos::ErrorEnum::eNone;
}
