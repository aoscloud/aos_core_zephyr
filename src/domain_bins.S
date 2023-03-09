/*
 * Copyright (c) 2023 EPAM Systems
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/toolchain.h>

.section .data.img_unikraft,"a"

GDATA(__img_unikraft_start)
GDATA(__img_unikraft_end)

__img_unikraft_start:
.incbin "helloworld_xen-arm64"
__img_unikraft_end:

.section .data.dtb_unikraft,"a"

GDATA(__dtb_unikraft_start)
GDATA(__dtb_unikraft_end)
__dtb_unikraft_start:
.align 8
.incbin "unikernel.dtb"
__dtb_unikraft_end:
