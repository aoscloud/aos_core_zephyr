/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VCHAN_H_
#define VCHAN_H_

#include <stddef.h>
#include <stdint.h>

extern "C" {

typedef uint16_t domid_t;

struct vch_handle { };

/**
 * Set up vchannel, allocate & grant page(s), initialize vchannel object
 * @param domid ID of domain from which connection is expected
 * @param path xenstore base path for exchanging grant/event IDs
 * @param min_rs minimum size (in bytes) of the server side receiving ring
 * @param min_ws minimum size (in bytes) of the server side sending ring
 * @param handle pre-allocated vchannel object to be initialized
 * @return 0 on success, a negative errno value on error.
 */
int vch_open(domid_t domid, const char* path, size_t min_rs, size_t min_ws, struct vch_handle* handle);

/**
 * Connect to an existing opened vchannel.
 *
 * @param domid ID of domain to connect to
 * @param path xenstore base path for exchanging grant/event IDs
 * @param handle pre-allocated vchannel object to be initialized
 * @return 0 on success, a negative errno value on error.
 */

int vch_connect(domid_t domid, const char* path, struct vch_handle* handle);

/*
 * Close a vchannel, free its resources and notify the other side.
 * @param handle vchannel object
 */
void vch_close(struct vch_handle* handle);

/**
 * Stream-based receive: reads as much data as possible.
 * @param handle vchannel object
 * @param buf pre-allocated buffer to copy received data to
 * @param size size of the buffer
 * @return amount of data read or negative errno on error
 */

int vch_read(struct vch_handle* handle, void* buf, size_t size);

/**
 * Stream-based send: send as much data as possible.
 * @param handle vchannel object
 * @param buf buffer to copy sent data from
 * @param size size of the buffer
 * @return amount of data sent or negative errno on error
 */

int vch_write(struct vch_handle* handle, const void* buf, size_t size);

} // extern "C"

#endif
