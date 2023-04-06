/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CMCLIENT_HPP_
#define CMCLIENT_HPP_

#include "zephyr/sys/atomic.h"

#include <aos/common/tools/buffer.hpp>
#include <aos/common/tools/error.hpp>
#include <aos/common/tools/thread.hpp>
#include <aos/sm/launcher.hpp>

#include <pb_encode.h>
#include <servicemanager.pb.h>
extern "C" {
#include <vch.h>
}

#include "downloader/downloader.hpp"
#include "resourcemanager/resourcemanager.hpp"

using SHA256Digest = uint8_t[32];

/**
 * CM client instance.
 */
class CMClient : public aos::sm::launcher::InstanceStatusReceiverItf,
                 public DownloadRequesterItf,
                 private aos::NonCopyable {
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
     * @param resourceManager resourcemanager instance.
     * @return aos::Error.
     */
    aos::Error Init(
        aos::sm::launcher::LauncherItf& launcher, ResourceManager& resourceManager, DownloadReceiverItf& downloader);

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

    /**
     * Send image content request
     *
     * @param request image content request
     * @return Error
     */
    aos::Error SendImageContentRequest(const ImageContentRequest& request) override;

private:
    static constexpr auto cConnectionTimeoutSec = 5;
    static constexpr auto cDomdID = CONFIG_AOS_DOMD_ID;
    static constexpr auto cSmVchanPath = CONFIG_AOS_SM_VCHAN_PATH;
    static constexpr auto cNodeID = CONFIG_AOS_NODE_ID;
    static constexpr auto cNodeType = CONFIG_AOS_NODE_TYPE;
    static constexpr auto cNumCPUs = CONFIG_AOS_NUM_CPU;
    static constexpr auto cTotalRAM = CONFIG_AOS_TOTAL_RAM;
    static constexpr auto cPartitionSize = CONFIG_AOS_PARTITION_SIZE;
    static constexpr auto CReadDelayUSec = 50000;

    void                             ProcessMessages();
    void                             ConnectToCM();
    void                             SendNodeConfiguration();
    aos::Error                       SendPbMessageToVchan();
    aos::Error                       SendBufferToVchan(vch_handle* vChanHandler, const uint8_t* buffer, size_t msgSize);
    aos::Error                       CalculateSha256(const aos::Buffer& buffer, size_t size, SHA256Digest& digest);
    servicemanager_v3_InstanceStatus InstanceStatusToPB(const aos::InstanceStatus& instanceStatus) const;
    void                             ProcessGetUnitConfigStatus();
    void                             ProcessCheckUnitConfig();
    void                             ProcessSetUnitConfig();
    void                             ProcessRunInstancesMessage();
    void                             ProcessImageContentInfo();
    void                             ProcessImageContentChunk();
    void                             ReadDataFromVChan(vch_handle* vchanHandler, void* des, size_t size);

    aos::sm::launcher::LauncherItf*                              mLauncher;
    ResourceManager*                                             mResourceManager;
    DownloadReceiverItf*                                         mDownloader;
    aos::Thread<>                                                mThread;
    atomic_t                                                     mFinishReadTrigger;
    vch_handle                                                   mSMvchanHandler;
    servicemanager_v3_SMIncomingMessages                         mIncomingMessage;
    servicemanager_v3_SMOutgoingMessages                         mOutgoingMessage;
    aos::Mutex                                                   mSendMutex;
    aos::StaticBuffer<servicemanager_v3_SMOutgoingMessages_size> mSendBuffer;
    aos::StaticBuffer<aos::Max(size_t(servicemanager_v3_SMIncomingMessages_size),
        sizeof(aos::InstanceInfoStaticArray) + sizeof(aos::ServiceInfoStaticArray) + sizeof(aos::LayerInfoStaticArray))>
        mReceiveBuffer;
};

#endif
