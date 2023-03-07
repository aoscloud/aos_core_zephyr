/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CMCLIENT_HPP_
#define CMCLIENT_HPP_

#include "zephyr/sys/atomic.h"

#include <aos/common/buffer.hpp>
#include <aos/common/error.hpp>
#include <aos/common/thread.hpp>
#include <aos/sm/launcher.hpp>
#include <pb_encode.h>
#include <servicemanager.pb.h>
extern "C" {
#include <vch.h>
}

using SHA256Digest = uint8_t[32];

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
        atomic_clear_bit(&mFinishReadTrigger, 0);
    }

    /**
     * Destructor.
     */
    ~CMClient();

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
    aos::sm::launcher::LauncherItf*                              mLauncher;
    aos::Thread<>                                                mThread;
    atomic_t                                                     mFinishReadTrigger;
    vch_handle                                                   mSMvchanHandler;
    aos::StaticBuffer<servicemanager_v3_SMOutgoingMessages_size> mSendBuffer;
    aos::StaticBuffer<servicemanager_v3_SMIncomingMessages_size> mReceiveBuffer;
    servicemanager_v3_SMIncomingMessages                         mIncomingMessage;
    servicemanager_v3_SMOutgoingMessages                         mOutgoingMessage;
    aos::Mutex                                                   mSendMutex;

    void                             ProcessMessages();
    void                             ConnectToCM();
    void                             SendNodeConfiguration();
    aos::Error                       SendPbMessageToVchan();
    aos::Error                       SendBufferToVchan(vch_handle* vChanHandler, const uint8_t* buffer, size_t msgSize);
    aos::Error                       CalculateSha256(const aos::Buffer& buffer, size_t size, SHA256Digest& digest);
    servicemanager_v3_InstanceStatus InstanceStatusToPB(const aos::InstanceStatus& instanceStatus) const;
};

#endif
