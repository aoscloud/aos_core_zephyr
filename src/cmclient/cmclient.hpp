/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CMCLIENT_HPP_
#define CMCLIENT_HPP_

#include <aos/common/thread.hpp>
#include <aos/sm/launcher.hpp>

/**
 * CM client instance.
 */
class CMClient : public aos::sm::launcher::InstanceStatusReceiverItf, private aos::NonCopyable {
public:
    /**
     * Creates CM client.
     */
    CMClient()
        : mLauncher(nullptr)
        , mThread()
    {
    }

    /**
     * Initializes CM client instance.
     * @param launcher instance launcher.
     * @return aos::Error.
     */
    aos::Error Init(aos::sm::launcher::LauncherItf& launcher);

    /**
     * Sends instances run status.
     *
     * @param instances instances status array.
     * @return Error.
     */
    aos::Error InstancesRunStatus(const aos::Array<aos::InstanceStatus>& instances) override;

    /**
     * Sends instances update status.
     * @param instances instances status array.
     *
     * @return Error.
     */
    aos::Error InstancesUpdateStatus(const aos::Array<aos::InstanceStatus>& instances) override;

private:
    aos::sm::launcher::LauncherItf* mLauncher;
    aos::Thread<>                   mThread;

    void ProcessMessages();
};

#endif
