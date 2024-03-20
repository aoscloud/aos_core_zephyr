/*
 * Copyright (c) 2023 EPAM Systems
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <domain.h>
#include <string.h>
#include <xen_dom_mgmt.h>
#include <zephyr/xen/public/domctl.h>

#include <domains/dom_runner.h>

static char* domd_dtdevs[] = {
    "/soc/dma-controller@e7350000",
    "/soc/dma-controller@e7351000",
    "/soc/ethernet@e68c0000",
    "/soc/pcie@e65d0000",
    "/soc/scsi@e6860000",
};
static char* dt_passthrough_nodes[] = {
    "/extal",
    "/extalr",
    "/pcie_bus",
    "/scif",
    "/soc",
    "/ufs30_refclk_v",
    "/regulator-1p8v",
    "/regulator-3p3v",
    "/regulator-vcc-sdhi",
    "/reserved-memory",
};

static struct xen_domain_iomem domd_iomems[] = {
    {.first_mfn = 0xe6020, .nr_mfns = 0x1},
    {.first_mfn = 0xe6050, .nr_mfns = 0x1},
    {.first_mfn = 0xe6051, .nr_mfns = 0x1},
    {.first_mfn = 0xe6150, .nr_mfns = 0x4}, /* CPG */
    {.first_mfn = 0xe6160, .nr_mfns = 0x4}, /* RST */
    {.first_mfn = 0xe6180, .nr_mfns = 0x4}, /* SYSC */
    {.first_mfn = 0xe6500, .nr_mfns = 0x1},
    {.first_mfn = 0xe66d8, .nr_mfns = 0x1},
    {.first_mfn = 0xe6860, .nr_mfns = 0x1}, /* UFS HCD */
    {.first_mfn = 0xe68c0, .nr_mfns = 0x20}, /* EtherAVB */
    {.first_mfn = 0xe6444, .nr_mfns = 0x3}, /* EtherAVB */
    {.first_mfn = 0xe6550, .nr_mfns = 0x1},
    {.first_mfn = 0xe6e60, .nr_mfns = 0x1},
    {.first_mfn = 0xe7350, .nr_mfns = 0x1},
    {.first_mfn = 0xe7300, .nr_mfns = 0x10},
    {.first_mfn = 0xe7351, .nr_mfns = 0x1},
    {.first_mfn = 0xe7310, .nr_mfns = 0x10},
    {.first_mfn = 0xfff00, .nr_mfns = 0x1},
    {.first_mfn = 0xffd60, .nr_mfns = 0x1},
    {.first_mfn = 0xffc10, .nr_mfns = 0x10},
    {.first_mfn = 0xffd61, .nr_mfns = 0x1},
    {.first_mfn = 0xffc20, .nr_mfns = 0x10},
    {.first_mfn = 0xffd62, .nr_mfns = 0x1},
    {.first_mfn = 0xffd70, .nr_mfns = 0x10},
    {.first_mfn = 0xffd63, .nr_mfns = 0x1},
    {.first_mfn = 0xffd80, .nr_mfns = 0x10},
    {.first_mfn = 0xe65d3, .nr_mfns = 0x2},
    {.first_mfn = 0xe65d5, .nr_mfns = 0x2},
    {.first_mfn = 0xe65d6, .nr_mfns = 0x1},
    {.first_mfn = 0xe65d7, .nr_mfns = 0x1},
    {.first_mfn = 0xe6198, .nr_mfns = 0x1},
    {.first_mfn = 0xe61a0, .nr_mfns = 0x1},
    {.first_mfn = 0xe61a0, .nr_mfns = 0x1},
    {.first_mfn = 0xe61b0, .nr_mfns = 0x1},
    {.first_mfn = 0xe6260, .nr_mfns = 0x10},
    {.first_mfn = 0xdfd91, .nr_mfns = 0x1},
    {.first_mfn = 0xe6078, .nr_mfns = 0x1},
};

static uint32_t domd_irqs[] = {
    // gpio@e6050180
    854,
    // gpio@e6050980
    858,
    // gpio@e6051180
    948,
    // gpio@e6051980
    952,
    // scsi@e6860000 (UFS)
    267,
    // i2c@e6500000
    270,
    // i2c@e66d8000
    274,
    // serial@e6550000
    278,
    // serial@e6e60000
    281,
    // serial@e6c50000 - used by Xen
    //   284,
    // dma-controller@e7350000
    133,
    134,
    135,
    136,
    137,
    138,
    139,
    140,
    141,
    142,
    143,
    144,
    145,
    146,
    147,
    148,
    149,
    // dma-controller@e7351000
    151,
    158,
    157,
    152,
    179,
    180,
    181,
    182,
    183,
    184,
    185,
    186,
    155,
    156,
    154,
    153,
    159,
    // dma-controller@ffd60000
    64,
    65,
    66,
    67,
    68,
    69,
    70,
    71,
    72,
    73,
    74,
    75,
    76,
    77,
    61,
    62,
    63,
    // dma-controller@ffd61000
    79,
    80,
    81,
    82,
    83,
    84,
    85,
    86,
    87,
    88,
    89,
    90,
    91,
    92,
    93,
    94,
    95,
    // dma-controller@ffd62000
    97,
    98,
    99,
    100,
    101,
    102,
    103,
    104,
    105,
    106,
    107,
    108,
    109,
    110,
    111,
    112,
    113,
    // dma-controller@ffd63000
    128,
    129,
    130,
    131,
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    123,
    124,
    125,
    126,
    127,
    // pcie@e65d0000
    448,
    449,
    450,
    451,
    452,
    453,
    454,
    // mfis@e6260000
    352,
    // iccom01
    354,
    // iccom02
    356,
    // iccom03
    358,
    // iccom04
    360,
    // iccom05
    362,
    // iccom06
    364,
    // iccom07
    366,
    // iccom016
    386,
    // iccom017
    388,
    // iccom018
    390,
    // iccom019
    392,
    // iccom020
    394,
    // iccom021
    396,
    // iccom022
    398,
    // iccom023
    400,
    // ethernet@e68c0000
    312,
    313,
    314,
    315,
    316,
    317,
    318,
    319,
    320,
    321,
    322,
    323,
    324,
    325,
    326,
    327,
    328,
    329,
    330,
    331,
};

extern char __img_ipl_start[];
extern char __img_ipl_end[];
extern char __dtb_ipl_start[];
extern char __dtb_ipl_end[];

static int load_ipl_image(uint8_t* buf, size_t bufsize, uint64_t image_load_offset, void* image_info)
{
    ARG_UNUSED(image_info);
    memcpy(buf, __img_ipl_start + image_load_offset, bufsize);
    return 0;
}

static ssize_t get_ipl_image_size(void* image_info, uint64_t* size)
{
    ARG_UNUSED(image_info);
    *size = __img_ipl_end - __img_ipl_start;
    return 0;
}

struct xen_domain_cfg domd_cfg = {
    .machine_dt_compat = "renesas,r8a779f0",
    .mem_kb            = 0x100000, /* 1Gb */

    .flags               = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap | XEN_DOMCTL_CDF_iommu),
    .max_evtchns         = 10,
    .max_vcpus           = 4,
    .gnt_frames          = 32,
    .max_maptrack_frames = 1,

    .iomems    = domd_iomems,
    .nr_iomems = ARRAY_SIZE(domd_iomems),

    .irqs    = domd_irqs,
    .nr_irqs = ARRAY_SIZE(domd_irqs),

    .gic_version = XEN_DOMCTL_CONFIG_GIC_V3,
    .tee_type    = XEN_DOMCTL_CONFIG_TEE_OPTEE,

    .dtdevs    = domd_dtdevs,
    .nr_dtdevs = ARRAY_SIZE(domd_dtdevs),

    .dt_passthrough    = dt_passthrough_nodes,
    .nr_dt_passthrough = ARRAY_SIZE(dt_passthrough_nodes),
    .load_image_bytes  = load_ipl_image,
    .get_image_size    = get_ipl_image_size,
    .image_info        = NULL,
    .cmdline = "root=/dev/sda4 ro rootwait console=hvc0 clk_ignore_unused pci=pcie_bus_perf vardir.disk=/dev/sda5 "
               "opendisk.target=/dev/sda6 opendisk.pkcs11=optee aosupdate.disk=/dev/aosvg/workdirs "
               "aosupdate.path=um/update_rootfs aosupdate.selinux_module=/usr/share/selinux/aos/base.pp",

    .dtb_start = __dtb_ipl_start,
    .dtb_end   = __dtb_ipl_end,
};

int create_domains(void)
{
    return domain_create(&domd_cfg, 1);
};
