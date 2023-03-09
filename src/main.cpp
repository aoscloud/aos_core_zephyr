/*
 * Copyright (C) 2023 Renesas Electronics Corporation.
 * Copyright (C) 2023 EPAM Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#define DEBUG

#include <zephyr/sys/__assert.h>
#include <zephyr/sys/printk.h>

#include <aos/common/version.hpp>

#include "app/app.hpp"
#include "logger/logger.hpp"
#include "version.hpp"

#if defined(DEBUG)
extern "C" {

#include <storage.h>
#include <xen_dom_mgmt.h>
#include <xrun.h>

char json_msg[] = "{"
	 "\"ociVersion\" : \"1.0.1\", "
		"\"vm\" : { "
			"\"hypervisor\": { "
			"\"path\": \"xen\", "
			"\"parameters\": [\"pvcalls=true\"] "
		"}, "
		"\"kernel\": { "
			"\"path\" : \"unikernel.bin\", "
			"\"parameters\" : [ \"port=8124\", \"hello world\" ]"
		"}, "
		"\"hwconfig\": { "
			"\"devicetree\": \"uni.dtb\" "
		"} "
	"} "
"}";

extern char __img_unikraft_start[];
extern char __img_unikraft_end[];
extern char __dtb_unikraft_start[];
extern char __dtb_unikraft_end[];

extern void init_root();

static void prepare_configs()
{
	int ret;

	init_root();

	printk("dtb = %p\n", (void *)__dtb_unikraft_start);
	ret = write_file("/lfs", "config.json", json_msg, sizeof(json_msg));
	printk("Write config.json file ret = %d\n", ret);
	ret = write_file("/lfs", "unikernel.bin", __img_unikraft_start,
			__img_unikraft_end - __img_unikraft_start);
	printk("Write unikraft.bim file ret = %d\n", ret);
	ret = write_file("/lfs", "uni.dtb", __dtb_unikraft_start,
			__dtb_unikraft_end - __dtb_unikraft_start);
	printk("Write unikraft.dtb file ret = %d\n", ret);
	ret = lsdir("/lfs");
	printk("lsdir result = %d\n", ret);
}

} /* extern "C" */
#endif /* DEBUG */

int main(void)
{
    printk("*** Aos zephyr application: %s ***\n", AOS_ZEPHYR_APP_VERSION);
    printk("*** Aos core library: %s ***\n", AOS_CORE_VERSION);

	prepare_configs();

    Logger::Init();

    auto& app = App::Get();

    auto err = app.Init();
    __ASSERT(err.IsNone(), "Error initializing application: %s", err.ToString());

    return 0;
}
