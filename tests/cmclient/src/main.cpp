/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/tc_util.h>
#include <zephyr/ztest.h>

#include <pb_decode.h>
#include <servicemanager.pb.h>
#include <tinycrypt/constants.h>
#include <tinycrypt/sha256.h>
#include <vchanapi.h>

#include <aos/sm/launcher.hpp>

#include <aos/common/log.hpp>

#include "cmclient/cmclient.hpp"
#include "resourcemanager/resourcemanager.hpp"

static constexpr char testVersion[6] = "1.0.1";

bool                                 gWaitHeader = true;
bool                                 gReadHead = true;
bool                                 gShutdown = false;
bool                                 gReadyToRead = true;
VchanMessageHeader                   gCurrentHeader;
servicemanager_v3_SMOutgoingMessages gReceivedMessage;
servicemanager_v3_SMIncomingMessages gSendMessage;
uint8_t                              gSendBuffer[servicemanager_v3_SMIncomingMessages_size];
aos::Mutex                           gWaitMessageMutex;
aos::ConditionalVariable             gWaitMessageCondVar(gWaitMessageMutex);
aos::Mutex                           gReadMutex;
aos::ConditionalVariable             gReadCondVar(gReadMutex);
size_t                               gCurrentSendBufferSize;
CMClient                             gTestCMClient;

using namespace aos;

void TestLogCallback(LogModule module, LogLevel level, const char* message)
{
    TC_PRINT("[client] %s \n", message);
}

class MockLauncher : public sm::launcher::LauncherItf {
public:
    Error RunInstances(const Array<InstanceInfo>& instances, bool forceRestart = false) { return ErrorEnum::eNone; }
};

int vch_open(domid_t domain, const char* path, size_t min_rs, size_t min_ws, struct vch_handle* h)
{
    return 0;
}

aos::Error ResourceManager::GetUnitConfigInfo(char* version) const
{
    TC_PRINT("[test] GetUnitConfigInfo\n");

    strcpy(version, testVersion);

    return aos::ErrorEnum::eNone;
}

aos::Error ResourceManager::CheckUnitConfig(const char* version, const char* unitConfig) const
{
    return aos::ErrorEnum::eNone;
}

aos::Error ResourceManager::UpdateUnitConfig(const char* version, const char* unitConfig)
{
    return aos::ErrorEnum::eNone;
}

int vch_connect(domid_t domain, const char* path, struct vch_handle* h)
{
    TC_PRINT("[test] Connection Done\n");
    {
        UniqueLock lock(gWaitMessageMutex);
        gWaitMessageCondVar.NotifyOne();
    }

    {
        UniqueLock lock(gReadMutex);
        gReadCondVar.Wait([&] { return gReadyToRead; });
        gReadyToRead = false;
    }

    return 0;
}

void vch_close(struct vch_handle* h)
{
}

int vch_read(struct vch_handle* h, void* buf, size_t size)
{
    TC_PRINT("[test] Start read  %d \n", size);
    if (gReadHead) {
        UniqueLock lock(gReadMutex);
        gReadCondVar.Wait([&] { return gReadyToRead; });
        gReadyToRead = false;
        if (gShutdown) {
            return -1;
        }
    }

    if (gReadHead) {
        auto outStream = pb_ostream_from_buffer(gSendBuffer, servicemanager_v3_SMIncomingMessages_size);
        auto status = pb_encode(&outStream, servicemanager_v3_SMIncomingMessages_fields, &gSendMessage);
        zassert_true(status, "Encoding failed: %s\n", PB_GET_ERROR(&outStream));

        VchanMessageHeader     header;
        tc_sha256_state_struct s;

        int err = tc_sha256_init(&s);
        zassert_false((TC_CRYPTO_SUCCESS != err), "Can't init SHA256: %d", err);

        err = tc_sha256_update(&s, gSendBuffer, outStream.bytes_written);
        zassert_false((TC_CRYPTO_SUCCESS != err), "Can't hash message: %d", err);

        err = tc_sha256_final(header.sha256, &s);
        zassert_false((TC_CRYPTO_SUCCESS != err), "Can't finish hash message: %d", err);

        header.dataSize = outStream.bytes_written;

        memcpy(buf, static_cast<void*>(&header), sizeof(VchanMessageHeader));
        gCurrentSendBufferSize = outStream.bytes_written;
    } else {
        memcpy(buf, gSendBuffer, gCurrentSendBufferSize);
    }

    gReadHead = !gReadHead;

    TC_PRINT("[test] END read  %d \n", size);
    return size;
}

int vch_write(struct vch_handle* h, const void* buf, size_t size)
{
    TC_PRINT("[test] Start write %d header %d \n", size, gWaitHeader);

    if (gWaitHeader) {
        gWaitHeader = false;
        zassert_equal(size, sizeof(VchanMessageHeader), "Incorrect header size");
        memcpy(&gCurrentHeader, buf, sizeof(VchanMessageHeader));

        return size;
    }

    gWaitHeader = true;

    zassert_equal(size, gCurrentHeader.dataSize, "Header and message length mismatch");

    tc_sha256_state_struct s;
    uint8_t                sha256[32];

    int ret = tc_sha256_init(&s);
    zassert_false((TC_CRYPTO_SUCCESS != ret), "Can't init SHA256: %d", ret);

    ret = tc_sha256_update(&s, (const uint8_t*)buf, size);
    zassert_false((TC_CRYPTO_SUCCESS != ret), "Can't hash message: %d", ret);

    ret = tc_sha256_final(sha256, &s);
    zassert_false((TC_CRYPTO_SUCCESS != ret), "Can't finish hash message: %d", ret);
    zassert_false((0 != memcmp(sha256, gCurrentHeader.sha256, 32)), "Sha256 mismatch");

    gReceivedMessage = servicemanager_v3_SMOutgoingMessages servicemanager_v3_SMOutgoingMessages_init_zero;

    pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t*)buf, size);
    auto         status = pb_decode(&stream, servicemanager_v3_SMOutgoingMessages_fields, &gReceivedMessage);
    zassert_true(status, "Decoding failed: %s", PB_GET_ERROR(&stream));

    TC_PRINT("[test] Notify msg %d \n", gReceivedMessage.which_SMOutgoingMessage);
    {
        UniqueLock lock(gWaitMessageMutex);
        gWaitMessageCondVar.NotifyOne();
    }

    return size;
}

void tesUpdateAndRunInstanceStatuses()
{
    StaticArray<InstanceStatus, 1> instances;
    InstanceStatus                 status;
    status.mInstanceIdent = InstanceIdent {"service1", "subject1", 0};
    status.mAosVersion = 12;
    status.mRunState = InstanceRunStateEnum::eActive;
    status.mError = ErrorEnum::eNone;

    instances.PushBack(status);

    auto err = gTestCMClient.InstancesRunStatus(instances);
    zassert_true(err.IsNone(), "Error send run status: %s", err.ToString());
    zassert_equal(gReceivedMessage.which_SMOutgoingMessage,
        servicemanager_v3_SMOutgoingMessages_run_instances_status_tag, "Instance run status expected %d",
        gReceivedMessage.which_SMOutgoingMessage);
    zassert_equal(
        gReceivedMessage.SMOutgoingMessage.run_instances_status.instances_count, 1, "Incorrect run instances count");
    zassert_false(
        strcmp(gReceivedMessage.SMOutgoingMessage.run_instances_status.instances[0].instance.service_id, "service1"),
        "Incorrect Service id  %s",
        gReceivedMessage.SMOutgoingMessage.run_instances_status.instances[0].instance.service_id);

    err = gTestCMClient.InstancesUpdateStatus(instances);
    zassert_true(err.IsNone(), "Error send update status: %s", err.ToString());
    zassert_equal(gReceivedMessage.which_SMOutgoingMessage,
        servicemanager_v3_SMOutgoingMessages_update_instances_status_tag, "Instance run status expected %d",
        gReceivedMessage.which_SMOutgoingMessage);
    zassert_equal(gReceivedMessage.SMOutgoingMessage.update_instances_status.instances_count, 1,
        "Incorrect update instances count");
    zassert_false(
        strcmp(gReceivedMessage.SMOutgoingMessage.update_instances_status.instances[0].instance.subject_id, "subject1"),
        "Incorrect Subject id  %s",
        gReceivedMessage.SMOutgoingMessage.update_instances_status.instances[0].instance.subject_id);
}

void testGetUnitConfigStatus()
{
    gSendMessage.which_SMIncomingMessage = servicemanager_v3_SMIncomingMessages_get_unit_config_status_tag;

    {
        UniqueLock lock(gReadMutex);
        gReadyToRead = true;
        gReadCondVar.NotifyOne();
    }

    // Wait node config status
    {
        UniqueLock lock(gWaitMessageMutex);
        gWaitMessageCondVar.Wait();
    }

    zassert_equal(gReceivedMessage.which_SMOutgoingMessage, servicemanager_v3_SMOutgoingMessages_unit_config_status_tag,
        "Unit configuration status expected");
    zassert_false((0 != strcmp(gReceivedMessage.SMOutgoingMessage.unit_config_status.vendor_version, testVersion)),
        "Incorrect unit config version");
}

void testCheckUnitConfig()
{
    gSendMessage.which_SMIncomingMessage = servicemanager_v3_SMIncomingMessages_check_unit_config_tag;
    strcpy(gSendMessage.SMIncomingMessage.check_unit_config.vendor_version, testVersion);
    {
        UniqueLock lock(gReadMutex);
        gReadyToRead = true;
        gReadCondVar.NotifyOne();
    }

    // Wait node config status
    {
        UniqueLock lock(gWaitMessageMutex);
        gWaitMessageCondVar.Wait();
    }

    zassert_equal(gReceivedMessage.which_SMOutgoingMessage, servicemanager_v3_SMOutgoingMessages_unit_config_status_tag,
        "Unit configuration status expected");
    zassert_false((0 != strcmp(gReceivedMessage.SMOutgoingMessage.unit_config_status.vendor_version, testVersion)),
        "Incorrect unit config version");
}

void testUpdateUnitConfig()
{
    gSendMessage.which_SMIncomingMessage = servicemanager_v3_SMIncomingMessages_set_unit_config_tag;
    strcpy(gSendMessage.SMIncomingMessage.set_unit_config.vendor_version, testVersion);
    {
        UniqueLock lock(gReadMutex);
        gReadyToRead = true;
        gReadCondVar.NotifyOne();
    }

    // Wait node config status
    {
        UniqueLock lock(gWaitMessageMutex);
        gWaitMessageCondVar.Wait();
    }

    zassert_equal(gReceivedMessage.which_SMOutgoingMessage, servicemanager_v3_SMOutgoingMessages_unit_config_status_tag,
        "Unit configuration status expected");

    zassert_false((0 != strcmp(gReceivedMessage.SMOutgoingMessage.unit_config_status.vendor_version, testVersion)),
        "Incorrect unit config version");
}

ZTEST_SUITE(cmclient, NULL, NULL, NULL, NULL, NULL);

ZTEST(cmclient, test_node_config)
{
    MockLauncher    mockLauncher;
    ResourceManager mockResourceManager;

    aos::Log::SetCallback(TestLogCallback);

    auto err = gTestCMClient.Init(mockLauncher, mockResourceManager);
    zassert_true(err.IsNone(), "init error: %s", err.ToString());

    // wait open
    {
        UniqueLock lock(gWaitMessageMutex);
        gWaitMessageCondVar.Wait();
    }

    {
        UniqueLock lock(gReadMutex);
        gReadyToRead = true;
        gReadCondVar.NotifyOne();
    }

    // Wait node config
    {
        UniqueLock lock(gWaitMessageMutex);
        gWaitMessageCondVar.Wait();
    }

    zassert_equal(gReceivedMessage.which_SMOutgoingMessage, servicemanager_v3_SMOutgoingMessages_node_configuration_tag,
        "Node configuration expected");

    tesUpdateAndRunInstanceStatuses();
    testGetUnitConfigStatus();
    testCheckUnitConfig();
    testUpdateUnitConfig();

    // release read tread to shutdown
    UniqueLock lock(gReadMutex);
    gReadyToRead = true;
    gShutdown = true;
    gReadCondVar.NotifyOne();
}
