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
    "/soc/ethernet@e6800000",
    "/soc/dma-controller@e6700000",
    "/soc/dma-controller@e7300000",
    "/soc/dma-controller@e7310000",
    "/soc/dma-controller@ec700000",
    "/soc/dma-controller@ec720000",
    "/soc/dma-controller@e65a0000",
    "/soc/dma-controller@e65b0000",
    "/soc/usb@ee000000",
    "/soc/usb@ee080000",
    "/soc/usb@ee080100",
    "/soc/usb@ee0a0000",
    "/soc/usb@ee0a0100",
    "/soc/mmc@ee100000",
    "/soc/mmc@ee160000",
    "/soc/fdpm@fe940000",
    "/soc/vspm@fe960000",
    "/soc/vspm@fe9a0000",
    "/soc/fcp@fea27000",
    "/soc/fcp@fea2f000",
    "/soc/fcp@fea37000",
    "/soc/hdmi@fead0000",
    "/soc/vcp4@fe90f000",
    "/soc/gsx_pv0_domd",
    "/soc/gsx_pv1_domd",
    "/soc/pcie@fe000000",
    "/soc/pcie@ee800000",
    "/soc/dma-controller@ffc10000",
    "/soc/video@e6ef0000",
    "/soc/video@e6ef1000",
    "/soc/video@e6ef2000",
    "/soc/video@e6ef3000",
    "/soc/video@e6ef4000",
    "/soc/video@e6ef5000",
    "/soc/video@e6ef6000",
    "/soc/video@e6ef7000",
};

static char* dt_passthrough_nodes[] = {
    "/gsx_opp_table0",
    "/gsx_opp_table1",
    "/gsx_opp_table2",
    "/gsx_opp_table3",
    "/gsx_opp_table4",
    "/gsx_opp_table5",
    "/gsx_opp_table6",
    "/gsx_opp_table7",
    "/extal",
    "/extalr",
    "/audio_clk_a",
    "/audio_clk_b",
    "/audio_clk_c",
    "/can",
    "/scif",
    "/pcie_bus",
    "/soc",
    "/audio-clkout",
    "/avb-mch@ec5a0100",
    "/backlight",
    "/cvbs-in",
    "/hdmi-in",
    "/regulator0",
    "/regulator1",
    "/regulator2",
    "/sound",
    "/regulator-vbus0-usb2",
    "/regulator-vcc-sdhi0",
    "/regulator-vccq-sdhi0",
    "/regulator-vcc-sdhi3",
    "/regulator-vccq-sdhi3",
    "/hdmi0-out",
    "/hdmi1-out",
    "/vga",
    "/vga-encoder",
    "/lvds",
    "/x12",
    "/x21-clock",
    "/x22-clock",
    "/x23-clock",
    "/vspm_if",
    "/usb3s0",
    "/usb_extal",
    "/reserved-memory",
    "/mmngr",
    "/mmngrbuf",
    "/__symbols__",
};

static struct xen_domain_iomem domd_iomems[] = {
    {.first_mfn = 0x54000, .nr_mfns = 0x3000},
    {.first_mfn = 0xe6020, .nr_mfns = 0x1},
    {.first_mfn = 0xe6050, .nr_mfns = 0x1},
    {.first_mfn = 0xe6051, .nr_mfns = 0x1},
    {.first_mfn = 0xe6052, .nr_mfns = 0x1},
    {.first_mfn = 0xe6053, .nr_mfns = 0x1},
    {.first_mfn = 0xe6054, .nr_mfns = 0x1},
    {.first_mfn = 0xe6055, .nr_mfns = 0x1},
    {.first_mfn = 0xe6260, .nr_mfns = 0x1},
    {.first_mfn = 0xe6060, .nr_mfns = 0x1},
    {.first_mfn = 0xe6150, .nr_mfns = 0x1},
    {.first_mfn = 0xe6160, .nr_mfns = 0x1},
    {.first_mfn = 0xfff00, .nr_mfns = 0x1},
    {.first_mfn = 0xe6180, .nr_mfns = 0x1},
    {.first_mfn = 0xe61c0, .nr_mfns = 0x1},
    {.first_mfn = 0xe6e31, .nr_mfns = 0x1},
    {.first_mfn = 0xe6e32, .nr_mfns = 0x1},
    {.first_mfn = 0xe6510, .nr_mfns = 0x1},
    {.first_mfn = 0xe66d8, .nr_mfns = 0x1},
    {.first_mfn = 0xe65ee, .nr_mfns = 0x1},
    {.first_mfn = 0xe6601, .nr_mfns = 0x1},
    {.first_mfn = 0xe6800, .nr_mfns = 0x1},
    {.first_mfn = 0xe6a00, .nr_mfns = 0x10},
    {.first_mfn = 0xe6e68, .nr_mfns = 0x1},
    {.first_mfn = 0xfea80, .nr_mfns = 0x10},
    {.first_mfn = 0xfeaa0, .nr_mfns = 0x10},
    {.first_mfn = 0xe6ef0, .nr_mfns = 0x1},
    {.first_mfn = 0xe6ef1, .nr_mfns = 0x1},
    {.first_mfn = 0xe6ef2, .nr_mfns = 0x1},
    {.first_mfn = 0xe6ef3, .nr_mfns = 0x1},
    {.first_mfn = 0xe6ef4, .nr_mfns = 0x1},
    {.first_mfn = 0xe6ef5, .nr_mfns = 0x1},
    {.first_mfn = 0xe6ef6, .nr_mfns = 0x1},
    {.first_mfn = 0xe6ef7, .nr_mfns = 0x1},
    {.first_mfn = 0xe6700, .nr_mfns = 0x10},
    {.first_mfn = 0xe7300, .nr_mfns = 0x10},
    {.first_mfn = 0xe7310, .nr_mfns = 0x10},
    {.first_mfn = 0xec700, .nr_mfns = 0x10},
    {.first_mfn = 0xec720, .nr_mfns = 0x10},
    {.first_mfn = 0xe65a0, .nr_mfns = 0x1},
    {.first_mfn = 0xe65b0, .nr_mfns = 0x1},
    {.first_mfn = 0xe6590, .nr_mfns = 0x1},
    {.first_mfn = 0xee000, .nr_mfns = 0x1},
    {.first_mfn = 0xee020, .nr_mfns = 0x1},
    {.first_mfn = 0xee080, .nr_mfns = 0x1},
    {.first_mfn = 0xee0a0, .nr_mfns = 0x1},
    {.first_mfn = 0xee100, .nr_mfns = 0x2},
    {.first_mfn = 0xee160, .nr_mfns = 0x2},
    {.first_mfn = 0xec500, .nr_mfns = 0x1},
    {.first_mfn = 0xec5a0, .nr_mfns = 0x1},
    {.first_mfn = 0xec540, .nr_mfns = 0x1},
    {.first_mfn = 0xec541, .nr_mfns = 0x1},
    {.first_mfn = 0xec760, .nr_mfns = 0x1},
    {.first_mfn = 0xec000, .nr_mfns = 0x1},
    {.first_mfn = 0xec008, .nr_mfns = 0x1},
    {.first_mfn = 0xfe000, .nr_mfns = 0x80},
    {.first_mfn = 0xee800, .nr_mfns = 0x80},
    {.first_mfn = 0xe67e0, .nr_mfns = 0x11},
    {.first_mfn = 0xfd000, .nr_mfns = 0x40},
    {.first_mfn = 0xfe900, .nr_mfns = 0x1},
    {.first_mfn = 0xfe910, .nr_mfns = 0x1},
    {.first_mfn = 0xfe8d0, .nr_mfns = 0x1},
    {.first_mfn = 0xfe90f, .nr_mfns = 0x1},
    {.first_mfn = 0xfe940, .nr_mfns = 0x3},
    {.first_mfn = 0xfe950, .nr_mfns = 0x1},
    {.first_mfn = 0xfe960, .nr_mfns = 0x8},
    {.first_mfn = 0xfe96f, .nr_mfns = 0x1},
    {.first_mfn = 0xfe9a0, .nr_mfns = 0x8},
    {.first_mfn = 0xfe9af, .nr_mfns = 0x1},
    {.first_mfn = 0xfea20, .nr_mfns = 0x5},
    {.first_mfn = 0xfea27, .nr_mfns = 0x1},
    {.first_mfn = 0xfea28, .nr_mfns = 0x5},
    {.first_mfn = 0xfea2f, .nr_mfns = 0x1},
    {.first_mfn = 0xfea30, .nr_mfns = 0x5},
    {.first_mfn = 0xfea37, .nr_mfns = 0x1},
    {.first_mfn = 0xfead0, .nr_mfns = 0x10},
    {.first_mfn = 0xfeb00, .nr_mfns = 0x70},
    {.first_mfn = 0xfeb90, .nr_mfns = 0x1},
    {.first_mfn = 0xe6198, .nr_mfns = 0x1},
    {.first_mfn = 0xe61a0, .nr_mfns = 0x1},
    {.first_mfn = 0xe61a8, .nr_mfns = 0x1},
    {.first_mfn = 0xe60b0, .nr_mfns = 0x1},
    {.first_mfn = 0xe60a0, .nr_mfns = 0x1},
    {.first_mfn = 0xffc10, .nr_mfns = 0x10},
    {.first_mfn = 0xe6550, .nr_mfns = 0x1},
};

static uint32_t domd_irqs[] = {
    // gpio@e6050000
    36,
    // gpio@e6051000
    37,
    // gpio@e6052000
    38,
    // gpio@e6053000
    39,
    // gpio@e6054000
    40,
    // gpio@e6055000
    41,
    // gpio@e6055400
    42,
    // gpio@e6055800
    43,
    // interrupt-controller@e61c0000
    32,
    33,
    34,
    35,
    193,
    50,
    // i2c@e6510000
    318,
    // i2c@e66d8000
    51,
    // crypto@e6601000
    103,
    // ethernet@e6800000
    71,
    72,
    73,
    74,
    75,
    76,
    77,
    78,
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
    // serial@e6e68000
    185,
    // csi2@fea80000
    216,
    // csi2@feaa0000
    278,
    // video@e6ef0000
    220,
    // video@e6ef1000
    221,
    // video@e6ef2000
    222,
    // video@e6ef3000
    223,
    // video@e6ef4000
    206,
    // video@e6ef5000
    207,
    // video@e6ef6000
    208,
    // video@e6ef7000
    203,
    // dma-controller@e6700000
    231,
    232,
    233,
    234,
    235,
    236,
    237,
    238,
    239,
    240,
    241,
    242,
    243,
    244,
    245,
    246,
    247,
    // dma-controller@e7300000
    251,
    345,
    252,
    346,
    248,
    340,
    341,
    342,
    343,
    344,
    249,
    250,
    347,
    348,
    349,
    350,
    351,
    // dma-controller@e7310000
    448,
    449,
    450,
    451,
    452,
    453,
    454,
    455,
    456,
    457,
    458,
    459,
    460,
    461,
    462,
    463,
    429,
    // dma-controller@ec700000
    352,
    353,
    354,
    355,
    356,
    357,
    358,
    359,
    360,
    361,
    362,
    363,
    364,
    365,
    366,
    367,
    382,
    // dma-controller@ec720000
    383,
    368,
    369,
    370,
    371,
    372,
    373,
    374,
    375,
    376,
    377,
    378,
    379,
    380,
    381,
    414,
    415,
    // dma-controller@e65a0000
    141,
    // dma-controller@e65b0000
    142,
    // usb@e6590000
    139,
    // usb@ee000000
    134,
    // usb@ee020000
    136,
    // usb@ee080000
    140,
    // usb@ee0a0000
    144,
    // mmc@ee100000
    197,
    // mmc@ee160000
    200,
    // src-0
    384,
    // src-1
    385,
    // src-2
    386,
    // src-3
    387,
    // src-4
    388,
    // src-5
    389,
    // src-6
    390,
    // src-7
    391,
    // src-8
    392,
    // src-9
    393,
    // ssi-0
    402,
    // ssi-1
    403,
    // ssi-2
    404,
    // ssi-3
    405,
    // ssi-4
    406,
    // ssi-5
    407,
    // ssi-6
    408,
    // ssi-7
    409,
    // ssi-8
    410,
    // ssi-9
    411,
    // adsp@ec800000
    //  269,
    // pcie@fe000000
    148,
    149,
    150,
    // pcie@ee800000
    180,
    181,
    182,
    // gsx@fd000000
    151,
    // vcp4@fe910000
    292,
    293,
    // vcp4@fe900000
    272,
    273,
    // vcp4@fe8d0000
    412,
    413,
    255,
    // fdpm@fe940000
    294,
    // vspm@fe960000
    298,
    // vspm@fe9a0000
    476,
    // vsp@fea20000
    498,
    // vsp@fea28000
    499,
    // vsp@fea30000
    500,
    // hdmi@fead0000
    421,
    // display@feb00000
    288,
    300,
    301,
    // thermal@e6198000
    99,
    100,
    101,
    // i2c@e60b0000
    205,
    // dma-controller@ffc10000
    480,
    481,
    482,
    483,
    484,
    485,
    486,
    487,
    488,
    489,
    490,
    491,
    492,
    493,
    494,
    495,
    496,
    // serial@e6550000
    187,
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
    .mem_kb = 0x200000, /* 2Gb */

    .flags               = (XEN_DOMCTL_CDF_hvm | XEN_DOMCTL_CDF_hap | XEN_DOMCTL_CDF_iommu),
    .max_evtchns         = 10,
    .max_vcpus           = 4,
    .gnt_frames          = 32,
    .max_maptrack_frames = 1,

    .iomems    = domd_iomems,
    .nr_iomems = ARRAY_SIZE(domd_iomems),

    .irqs    = domd_irqs,
    .nr_irqs = ARRAY_SIZE(domd_irqs),

    .gic_version = XEN_DOMCTL_CONFIG_GIC_V2,
    .tee_type    = XEN_DOMCTL_CONFIG_TEE_OPTEE,

    .dtdevs    = domd_dtdevs,
    .nr_dtdevs = ARRAY_SIZE(domd_dtdevs),

    .dt_passthrough    = dt_passthrough_nodes,
    .nr_dt_passthrough = ARRAY_SIZE(dt_passthrough_nodes),
    .load_image_bytes  = load_ipl_image,
    .get_image_size    = get_ipl_image_size,
    .image_info        = NULL,
    .cmdline
    = "root=/dev/mmcblk1p4 ro rootwait console=hvc0 clk_ignore_unused pci=pcie_bus_perf vardir.disk=/dev/mmcblk1p5 "
      "opendisk.target=/dev/mmcblk1p6 opendisk.pkcs11=optee aosupdate.disk=/dev/aosvg/workdirs "
      "aosupdate.path=um/update_rootfs aosupdate.selinux_module=/usr/share/selinux/aos/base.pp",

    .dtb_start = __dtb_ipl_start,
    .dtb_end   = __dtb_ipl_end,
};

int create_domains(void)
{
    return domain_create(&domd_cfg, 1);
};
