/*
 * Copyright (c) 2024 EPAM Systems
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <domain.h>
#include <zephyr/devicetree.h>
#include <zephyr/xen/public/domctl.h>
#include <zephyr/logging/log.h>
#include <xen_dom_mgmt.h>

LOG_MODULE_REGISTER(xen_domu, CONFIG_LOG_DEFAULT_LEVEL);

#define FILL_DOMU_VBD_CFG(node_id)                                                                 \
	{                                                                                          \
		.configured = true,                                                                \
		.backend_domain_id = DT_PROP(node_id, backend_domain_id),                          \
		.target = DT_PROP(node_id, target),                                                \
		.script = DT_PROP(node_id, script),                                                \
		.backendtype = DT_PROP(node_id, backendtype),                                      \
		.vdev = DT_PROP(node_id, vdev),                                                    \
		.access = DT_PROP(node_id, access),						   \
	},

#define FILL_VBD_CFG_IF_COMPAT(node_id)                                                            \
	COND_CODE_1(DT_NODE_HAS_COMPAT(node_id, xen_domu_vbd), (FILL_DOMU_VBD_CFG(node_id)), ())

#define FILL_DOMU_VIF_CFG(node_id)                                                                 \
	{                                                                                          \
		.configured = true,                                                                \
		.backend_domain_id = DT_PROP(node_id, backend_domain_id),                          \
		.mac = DT_PROP(node_id, mac),                                                      \
		.script = DT_PROP(node_id, script),                                                \
		.bridge = DT_PROP(node_id, bridge),                                                \
		.type = DT_PROP(node_id, vif_type),                                                \
		.ip = DT_PROP_OR(node_id, ip, ""),                                                 \
	},

#define FILL_VIF_CFG_IF_COMPAT(node_id)                                                            \
	COND_CODE_1(DT_NODE_HAS_COMPAT(node_id, xen_domu_vif), (FILL_DOMU_VIF_CFG(node_id)), ())

#define FILL_DOMU_BACK_CFGS(node_id)								   \
	.disks = {DT_FOREACH_CHILD_STATUS_OKAY(node_id, FILL_VBD_CFG_IF_COMPAT)},		   \
	.vifs  = {DT_FOREACH_CHILD_STATUS_OKAY(node_id, FILL_VIF_CFG_IF_COMPAT)},

#define FILL_DOMU_CFG_ITEM(node_id)                                                                \
	{                                                                                          \
		.name = DT_NODE_FULL_NAME(node_id),                                                \
		.mem_kb = DT_PROP(node_id, mem_size) / 1024,                                       \
		.max_evtchns = DT_PROP(node_id, max_evtchns),                                      \
		.max_vcpus = DT_PROP(node_id, max_vcpus),                                          \
		.gic_version = DT_PROP(node_id, gic_version),                                      \
		.max_maptrack_frames = DT_PROP(node_id, max_maptrack_frames),                      \
		.gnt_frames = DT_PROP(node_id, gnt_frames),                                        \
		.cmdline = DT_PROP(node_id, cmdline),                                              \
                                                                                                   \
		.flags = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap | XEN_DOMCTL_CDF_iommu),         \
                                                                                                   \
		.tee_type = XEN_DOMCTL_CONFIG_TEE_NONE,                                            \
		.load_image_bytes = load_domu_image,                                               \
		.get_image_size = get_domu_image_size,                                             \
		.dtb_start = __dtb_u_start,                                                        \
		.dtb_end = __dtb_u_end,                                                            \
                                                                                                   \
		.back_cfg = {FILL_DOMU_BACK_CFGS(node_id)},                                        \
	},

#ifdef CONFIG_DOMU

extern char __img_u_start[];
extern char __img_u_end[];
extern char __dtb_u_start[];
extern char __dtb_u_end[];

static int load_domu_image(uint8_t *buf, size_t bufsize, uint64_t image_load_offset,
			   void *image_info)
{
	ARG_UNUSED(image_info);
	memcpy(buf, __img_u_start + image_load_offset, bufsize);
	return 0;
}

static ssize_t get_domu_image_size(void *image_info, uint64_t *size)
{
	ARG_UNUSED(image_info);
	*size = __img_u_end - __img_u_start;
	return 0;
}

static struct xen_domain_cfg domu_cfg[] = {DT_FOREACH_STATUS_OKAY(xen_domu, FILL_DOMU_CFG_ITEM)};
#endif /* CONFIG_DOMU */

int find_cfg_and_create_domu(uint32_t domid)
{
#ifdef CONFIG_DOMU
	int ret;

	for (unsigned int i = 0; i < ARRAY_SIZE(domu_cfg); i++) {
		char *id = strstr(domu_cfg[i].name, "@");

		if (!id) {
			LOG_WRN("Can't find domain id begging in the config name: %s",
				domu_cfg[i].name);
			continue;
		}

		if (domid != strtol(id + 1, NULL, 16)) {
			continue;
		}

		ret = domain_create(&domu_cfg[i], domid);
		if (!ret) {
			return domain_post_create(&domu_cfg[i], domid);
		}

		return ret;
	}
#endif /* CONFIG_DOMU */

	LOG_ERR("There isn't any domain in dts with requested ID: %u", domid);
	return -EINVAL;
}
