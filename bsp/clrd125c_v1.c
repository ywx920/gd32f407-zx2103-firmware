#include "clrd125c_v1.h"
#include "systick.h"
#include <stdio.h>

typedef struct
{
    uint8_t reg;
    uint8_t mask;
    uint8_t value;
    const char *name;
} CLRD125C_CFG_ITEM;

typedef struct
{
    uint8_t reg;
    const char *name;
    const char *meaning;
} CLRD125C_KEY_REG_INFO;

static uint8_t g_clrd125cDeviceAddr = CLRD125C_DEVICE_ADDR_DEFAULT;
static uint8_t g_clrd125cWireBase = CLRD125C_REG_WIRE_BASE;

/* 0: 只打印配置校验summary；1: 同时打印LT1258等效关键寄存器写/读值。 */
#define CLRD125C_PRINT_KEY_CONFIG          0U
/* 0: 正常时只打印总summary；1: 同时打印每片芯片的配置过程summary。 */
#define CLRD125C_PRINT_DEVICE_CONFIG       0U
/* 0: 配置阶段只写入，不回读校验配置值；1: 写入后完整回读校验。 */
#define CLRD125C_VERIFY_CONFIG_READBACK    0U

/*
 * 本配置表用于复现原LT1258正在使用的有效路径和信号参数：
 *
 *   S_INB0 -> D_OUT0    CH1
 *   S_INB1 -> D_OUT1    CH3
 *   D_IN0  -> S_OUTA0   CH4
 *   D_IN1  -> S_OUTA1   CH6
 *
 * CTLE约17.7 dB，VOD约0.8 Vp-p，DEM约-5 dB，Fanout关闭。
 * CLRD125C没有-4.4 dB挡位，因此使用最接近LT1258的-5 dB。
 */
static const CLRD125C_CFG_ITEM g_clrd125cConfig[] =
{
    /* 首先屏蔽相关外部控制引脚，确保后续寄存器配置真正生效。 */
    {CLRD125C_REG_CONTROL_OVERRIDE, 0xE0U, CLRD125C_CONTROL_OVERRIDE_VALUE, "EQ/VGA/DEM override"},
    {CLRD125C_REG_MODE_OVERRIDE,    0x0CU, CLRD125C_MODE_OVERRIDE_VALUE,    "MODE/EN_HF override"},
    {CLRD125C_REG_ROUTE_OVERRIDE,   0x07U, CLRD125C_ROUTE_OVERRIDE_VALUE,   "route override"},

    /* 开启通道并选择B侧输入、A侧输出，保持与原LT1258路径一致。 */
    {CLRD125C_REG_CHANNEL_PD,       0xFFU, 0x00U,                           "channel enable"},
    {CLRD125C_REG_ROUTE,            0xFFU, CLRD125C_ROUTE_VALUE,            "LT1258-equivalent route"},

    /* 非线性模式按厂商建议打开有效通道的高频增强。 */
    {CLRD125C_REG_EN_HF,            0xFFU, CLRD125C_ACTIVE_CHANNEL_MASK,    "active channel EN_HF"},

    /* EN_HF=1且CTLE code=0x09时，6.25 GHz附近典型补偿约17.7 dB。 */
    {CLRD125C_REG_CTLE_CH1,         CLRD125C_CTLE_MASK,     CLRD125C_CTLE_VALUE,     "CH1 CTLE"},
    {CLRD125C_REG_CTLE_CH3,         CLRD125C_CTLE_MASK,     CLRD125C_CTLE_VALUE,     "CH3 CTLE"},
    {CLRD125C_REG_CTLE_CH4,         CLRD125C_CTLE_MASK,     CLRD125C_CTLE_VALUE,     "CH4 CTLE"},
    {CLRD125C_REG_CTLE_CH6,         CLRD125C_CTLE_MASK,     CLRD125C_CTLE_VALUE,     "CH6 CTLE"},

    /* bit6置1进入非线性模式，bit[2:0]=010设置约0.8 V输出摆幅。 */
    {CLRD125C_REG_MODE_VOD_CH1,     CLRD125C_MODE_VOD_MASK, CLRD125C_MODE_VOD_VALUE, "D_OUT0 mode/VOD"},
    {CLRD125C_REG_MODE_VOD_CH3,     CLRD125C_MODE_VOD_MASK, CLRD125C_MODE_VOD_VALUE, "D_OUT1 mode/VOD"},
    {CLRD125C_REG_MODE_VOD_CH4,     CLRD125C_MODE_VOD_MASK, CLRD125C_MODE_VOD_VALUE, "S_OUTA0 mode/VOD"},
    {CLRD125C_REG_MODE_VOD_CH6,     CLRD125C_MODE_VOD_MASK, CLRD125C_MODE_VOD_VALUE, "S_OUTA1 mode/VOD"},

    /* DEM code=010约为-5 dB。只配置当前实际使用的四个输出。 */
    {CLRD125C_REG_DEM_CH1,          CLRD125C_DEM_MASK,      CLRD125C_DEM_VALUE,      "D_OUT0 DEM"},
    {CLRD125C_REG_DEM_CH3,          CLRD125C_DEM_MASK,      CLRD125C_DEM_VALUE,      "D_OUT1 DEM"},
    {CLRD125C_REG_DEM_CH4,          CLRD125C_DEM_MASK,      CLRD125C_DEM_VALUE,      "S_OUTA0 DEM"},
    {CLRD125C_REG_DEM_CH6,          CLRD125C_DEM_MASK,      CLRD125C_DEM_VALUE,      "S_OUTA1 DEM"}
};

/*
 * CLRD125C-K V1.0完整寄存器配置表。
 *
 * 当前表是在docs/125C-XAZX.ini基础上，按原LT1258有效链路效果做等效调整：
 *   1) 保持路由 D[79]=0x50：S_INB0/S_INB1 -> D_OUT0/D_OUT1，
 *      D_IN0/D_IN1 -> S_OUTA0/S_OUTA1，Fanout关闭。
 *   2) 由寄存器接管EQ/VGA/DEM、MODE/ENHF、SEL/INPUT_EN控制。
 *   3) 对实际使用的CH1/CH3/CH4/CH6配置接近LT1258的输出效果：
 *      EN_HF开启，CTLE约17.7 dB，VOD约0.8 Vp-p，DEM约-5 dB。
 *
 * 地址换算：
 *   D[n]对应SMBus线上寄存器地址 = 0x80 + n
 *   D[0]~D[79]                  = 0x80~0xCF
 *
 * 配置分类速查表：
 *
 * 1) 全局/控制/引脚覆盖类
 *   D[00] 0x80  addr<3:0>/rd_done/block_rst/rst，地址观测与复位相关，实测回读随AD地址变化
 *   D[01] 0x81  pd<7:0>，CH0~CH7通道power-down/enable控制
 *   D[02] 0x82  or_rstpin，是否禁用RESET引脚控制，改由寄存器控制
 *   D[03] 0x83  rxdet_pw_ctrl/rstn_rxdet/reg19_ctrl，RX Detect默认控制
 *   D[04] 0x84  reg21_ctrl，默认控制
 *   D[05] 0x85  en_rxlos<7:0>，CH0~CH7 RX LOS检测使能
 *   D[06] 0x86  or_eq_pin/or_vga_pin/or_dem_pin，禁用外部EQ/VGA/DEM引脚控制，改由寄存器控制
 *   D[07] 0x87  en_hf<7:0>，CH0~CH7高频增强/boosting使能；当前0x5A开启CH1/CH3/CH4/CH6
 *   D[08] 0x88  or_mode_pin/or_enhf_pin，禁用外部MODE/ENHF引脚控制，改由寄存器控制
 *
 * 2) RX Detect / Driver Delay / 参考阈值类
 *   D[09] 0x89  CH1 drv_dly/rxdet_vref控制
 *   D[10] 0x8A  CH3 drv_dly/rxdet_vref控制
 *   D[11] 0x8B  CH4 A/B侧rxdet_vref控制
 *   D[12] 0x8C  CH6 A/B侧rxdet_vref控制
 *   D[13] 0x8D  CH4/CH6 drv_dly控制
 *
 * 3) CH0参数组：D_OUT0 -> S_INA0
 *   D[14] 0x8E  ctle_mf_ch0/rxdet_ch0
 *   D[15] 0x8F  ctle_ctrl_ch0，EQ/CTLE主调节值
 *   D[16] 0x90  ib_tempco_ctrl，温漂/偏置相关，通常保持默认
 *   D[17] 0x91  reserved
 *   D[18] 0x92  reserved
 *   D[19] 0x93  reserved
 *   D[20] 0x94  reserved
 *
 * 4) CH1参数组：D_OUT0 -> S_INB0，当前等效LT1258使用通道之一
 *   D[21] 0x95  ctle_mf_ch1/rxdet_ch1/vga_ctrl_ch1
 *   D[22] 0x96  ctle_ctrl_ch1，EQ/CTLE主调节值
 *   D[23] 0x97  scp_ch1/mode_ch1/vod_ch1，短路保护/线性模式/输出摆幅
 *   D[24] 0x98  dem_ch1，去加重DEM
 *   D[25] 0x99  reserved
 *   D[26] 0x9A  reserved
 *   D[27] 0x9B  reserved
 *
 * 5) CH2参数组：D_OUT1 -> S_INA1
 *   D[28] 0x9C  ctle_mf_ch2/rxdet_ch2
 *   D[29] 0x9D  ctle_ctrl_ch2，EQ/CTLE主调节值
 *   D[30] 0x9E  reserved
 *   D[31] 0x9F  reserved
 *   D[32] 0xA0  reserved
 *   D[33] 0xA1  reserved
 *   D[34] 0xA2  reserved
 *
 * 6) CH3参数组：D_OUT1 -> S_INB1，当前等效LT1258使用通道之一
 *   D[35] 0xA3  ctle_mf_ch3/rxdet_ch3/vga_ctrl_ch3
 *   D[36] 0xA4  ctle_ctrl_ch3，EQ/CTLE主调节值
 *   D[37] 0xA5  scp_ch3/mode_ch3/vod_ch3，短路保护/线性模式/输出摆幅
 *   D[38] 0xA6  dem_ch3，去加重DEM
 *   D[39] 0xA7  reserved
 *
 * 7) 信号检测阈值类
 *   D[40] 0xA8  sd_vth/fast_sd，信号检测阈值与快速检测控制
 *   D[41] 0xA9  reserved
 *   D[42] 0xAA  reserved
 *
 * 8) CH4参数组：D_IN0 -> S_OUTA0，当前等效LT1258使用通道之一
 *   D[43] 0xAB  ctle_mf_ch4/rxdet_ch4/vga_ctrl_ch4
 *   D[44] 0xAC  ctle_ctrl_ch4，EQ/CTLE主调节值
 *   D[45] 0xAD  scp_ch4/mode_ch4/vod_ch4，短路保护/线性模式/输出摆幅
 *   D[46] 0xAE  dem_ch4，去加重DEM
 *   D[47] 0xAF  reserved
 *   D[48] 0xB0  reserved
 *   D[49] 0xB1  reserved
 *
 * 9) CH5参数组：D_IN0 -> S_OUTB0
 *   D[50] 0xB2  ctle_mf_ch5/rxdet_ch5
 *   D[51] 0xB3  ctle_ctrl_ch5，EQ/CTLE主调节值
 *   D[52] 0xB4  scp_ch5/mode_ch5/vod_ch5，短路保护/线性模式/输出摆幅
 *   D[53] 0xB5  dem_ch5，去加重DEM
 *   D[54] 0xB6  reserved
 *   D[55] 0xB7  reserved
 *   D[56] 0xB8  reserved
 *
 * 10) CH6参数组：D_IN1 -> S_OUTA1，当前等效LT1258使用通道之一
 *   D[57] 0xB9  ctle_mf_ch6/rxdet_ch6/vga_ctrl_ch6
 *   D[58] 0xBA  ctle_ctrl_ch6，EQ/CTLE主调节值
 *   D[59] 0xBB  scp_ch6/mode_ch6/vod_ch6，短路保护/线性模式/输出摆幅
 *   D[60] 0xBC  dem_ch6，去加重DEM
 *   D[61] 0xBD  reserved
 *   D[62] 0xBE  reserved
 *   D[63] 0xBF  reserved
 *
 * 11) CH7参数组：D_IN1 -> S_OUTB1
 *   D[64] 0xC0  ctle_mf_ch7/rxdet_ch7
 *   D[65] 0xC1  ctle_ctrl_ch7，EQ/CTLE主调节值
 *   D[66] 0xC2  scp_ch7/mode_ch7/vod_ch7，短路保护/线性模式/输出摆幅
 *   D[67] 0xC3  dem_ch7，去加重DEM
 *
 * 12) 状态/测试/路由类
 *   D[68] 0xC4  rxdet_sel<7:0>，RX Detect检测次数/选择，当前0xFF表示CH0~CH7均采用二次检测模式
 *   D[69] 0xC5  rx_det<7:0>，RX Detect结果状态，实测写0xAA后读回0x5A
 *   D[70] 0xC6  det_losb<7:0>，RX LOS检测结果状态，实测写0xA0后读回0xA5
 *   D[71] 0xC7  reserved
 *   D[72] 0xC8  reserved
 *   D[73] 0xC9  en_clktst/dctst_ctrl，测试控制，通常保持默认
 *   D[74] 0xCA  en_tstbuf/bp_tstbuf/rxdet_tctrl，测试控制
 *   D[75] 0xCB  rxdet_tctrl_ch4~ch7，测试控制
 *   D[76] 0xCC  vcm_ctrl_rxterm，RX端共模/终端相关控制
 *   D[77] 0xCD  envod_ldrv<7:0>，输出驱动/VOD相关控制
 *   D[78] 0xCE  or_sel1/or_sel0/or_inputen，禁用外部SEL/INPUT_EN引脚控制，改由寄存器控制
 *   D[79] 0xCF  sel1/sel0/input_en，路由选择与输入使能，当前0x50用于等效LT1258路径
 */
static const uint8_t g_clrd125cFullConfig[CLRD125C_REG_NUM] =
{
    0x00U, 0x00U, 0x00U, 0xA2U, 0x02U, 0xFFU, 0xE0U, 0x5AU,
    0x0CU, 0x30U, 0x30U, 0x00U, 0x00U, 0x33U, 0xACU, 0x08U,
    0x04U, 0x00U, 0x00U, 0x00U, 0x00U, 0xAEU, 0x09U, 0x42U,
    0x02U, 0x00U, 0x00U, 0x00U, 0xACU, 0x08U, 0x00U, 0x00U,
    0x00U, 0x00U, 0x00U, 0xAEU, 0x09U, 0x42U, 0x02U, 0x00U,
    0x2AU, 0x00U, 0x00U, 0xAEU, 0x09U, 0x42U, 0x02U, 0x00U,
    0x00U, 0x00U, 0xACU, 0x08U, 0x05U, 0x05U, 0x00U, 0x00U,
    0x00U, 0xAEU, 0x09U, 0x42U, 0x02U, 0x00U, 0x00U, 0x00U,
    0xACU, 0x08U, 0x05U, 0x05U, 0xFFU, 0xAAU, 0xA0U, 0x00U,
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x00U, 0x07U, 0x50U
};

/*
 * 125C-XAZX.ini原始配置表，仅作为参考配置保留，当前不参与写入。
 *
 * 与当前g_clrd125cFullConfig[]的关系：
 *   1) 本数组完全按docs/125C-XAZX.ini生成，保留厂商/调试建议原始值。
 *   2) 当前实际写入已通过CLRD125C_ACTIVE_CONFIG切换到本ini原始配置。
 *   3) 如需切回LT1258等效配置，可将CLRD125C_ACTIVE_CONFIG改为g_clrd125cFullConfig。
 *
 * 主要配置含义：
 *   - D[00]~D[08]：全局控制、通道PD、RX LOS检测、外部引脚override。
 *   - D[09]~D[13]：RX Detect阈值、driver delay相关默认参数。
 *   - D[14]~D[20]：CH0参数组，包含ctle_mf、rxdet、ctle_ctrl及保留位。
 *   - D[21]~D[27]：CH1参数组，包含VGA、CTLE、MODE/VOD、DEM。
 *   - D[28]~D[34]：CH2参数组，包含rxdet、CTLE及保留位。
 *   - D[35]~D[39]：CH3参数组，包含VGA、CTLE、MODE/VOD、DEM。
 *   - D[40]~D[42]：信号检测阈值/快速检测控制及保留位。
 *   - D[43]~D[49]：CH4参数组，包含VGA、CTLE、MODE/VOD、DEM及保留位。
 *   - D[50]~D[56]：CH5参数组，包含rxdet、CTLE、MODE/VOD、DEM及保留位。
 *   - D[57]~D[63]：CH6参数组，包含VGA、CTLE、MODE/VOD、DEM及保留位。
 *   - D[64]~D[67]：CH7参数组，包含rxdet、CTLE、MODE/VOD、DEM。
 *   - D[68]~D[70]：RX Detect选择/结果、LOS结果状态。
 *   - D[71]~D[77]：保留/测试/终端/VOD驱动相关控制。
 *   - D[78]~D[79]：SEL/INPUT_EN override与路由/输入使能控制。
 *
 * 该ini表与LT1258等效表的关键差异：
 *   - D[07]：ini=0x00，LT1258等效=0x5A，后者开启CH1/CH3/CH4/CH6高频增强。
 *   - CH1 D[22:24]：ini=08/05/05，LT1258等效=09/42/02。
 *   - CH3 D[36:38]：ini=08/05/05，LT1258等效=09/42/02。
 *   - CH4 D[44:46]：ini=04/05/05，LT1258等效=09/42/02。
 *   - CH6 D[58:60]：ini=04/05/05，LT1258等效=09/42/02。
 *   - D[68]：ini=0x00，当前为了状态检测临时增强为0xFF。
 */
const uint8_t g_clrd125cXazxIniConfig[CLRD125C_REG_NUM] =
{
    /* D[00]~D[07] 0x80~0x87: status/power/rxlos/override/en_hf */
    0x00U, 0x00U, 0x00U, 0xA2U, 0x02U, 0xFFU, 0xE0U, 0x00U,

    /* D[08]~D[15] 0x88~0x8F: mode override, rxdet threshold/delay, CH0 */
    0x0CU, 0x30U, 0x30U, 0x00U, 0x00U, 0x33U, 0xACU, 0x08U,

    /* D[16]~D[23] 0x90~0x97: CH0 reserved/tempco, CH1 ctle_mf/ctle/mode_vod */
    0x04U, 0x00U, 0x00U, 0x00U, 0x00U, 0xAEU, 0x08U, 0x05U,

    /* D[24]~D[31] 0x98~0x9F: CH1 dem, CH2 ctle_mf/ctle, reserved */
    0x05U, 0x00U, 0x00U, 0x00U, 0xACU, 0x08U, 0x00U, 0x00U,

    /* D[32]~D[39] 0xA0~0xA7: CH2 reserved, CH3 ctle_mf/ctle/mode_vod/dem */
    0x00U, 0x00U, 0x00U, 0xAEU, 0x08U, 0x05U, 0x05U, 0x00U,

    /* D[40]~D[47] 0xA8~0xAF: signal detect threshold, CH4 ctle_mf/ctle/mode_vod/dem */
    0x2AU, 0x00U, 0x00U, 0xAEU, 0x04U, 0x05U, 0x05U, 0x00U,

    /* D[48]~D[55] 0xB0~0xB7: CH4 reserved, CH5 ctle_mf/ctle/mode_vod/dem */
    0x00U, 0x00U, 0xACU, 0x08U, 0x05U, 0x05U, 0x00U, 0x00U,

    /* D[56]~D[63] 0xB8~0xBF: CH5 reserved, CH6 ctle_mf/ctle/mode_vod/dem */
    0x00U, 0xAEU, 0x04U, 0x05U, 0x05U, 0x00U, 0x00U, 0x00U,

    /* D[64]~D[71] 0xC0~0xC7: CH7 ctle_mf/ctle/mode_vod/dem, rxdet/los status */
    0xACU, 0x08U, 0x05U, 0x05U, 0x00U, 0xAAU, 0xA0U, 0x00U,

    /* D[72]~D[79] 0xC8~0xCF: test/default control, route override, route */
    0x00U, 0x00U, 0x00U, 0x00U, 0x02U, 0x00U, 0x07U, 0x50U
};

/* 当前实际写入CLRD125C的配置表。 */
#define CLRD125C_ACTIVE_CONFIG             g_clrd125cXazxIniConfig
#define CLRD125C_ACTIVE_CONFIG_NAME        "125C-XAZX.ini"

/*
 * 精简打印表：只显示会直接影响当前LT1258等效工作状态的关键寄存器。
 * 其余寄存器仍然会写入和回读校验，只有失败或异常时才额外打印。
 */
static const CLRD125C_KEY_REG_INFO g_clrd125cKeyRegInfo[] =
{
    {0x06U, "override EQ/VGA/DEM", "0xE0: reg controls EQ/VGA/DEM"},
    {0x07U, "EN_HF",               "0x5A: enable HF boost on CH1/CH3/CH4/CH6"},
    {0x08U, "override MODE/ENHF",  "0x0C: reg controls MODE/ENHF"},

    {0x16U, "CH1 CTLE",            "0x09: CTLE about 17.7dB"},
    {0x17U, "CH1 MODE/VOD",        "0x42: nonlinear mode, VOD about 0.8Vp-p"},
    {0x18U, "CH1 DEM",             "0x02: DEM about -5dB"},

    {0x24U, "CH3 CTLE",            "0x09: CTLE about 17.7dB"},
    {0x25U, "CH3 MODE/VOD",        "0x42: nonlinear mode, VOD about 0.8Vp-p"},
    {0x26U, "CH3 DEM",             "0x02: DEM about -5dB"},

    {0x2CU, "CH4 CTLE",            "0x09: CTLE about 17.7dB"},
    {0x2DU, "CH4 MODE/VOD",        "0x42: nonlinear mode, VOD about 0.8Vp-p"},
    {0x2EU, "CH4 DEM",             "0x02: DEM about -5dB"},

    {0x3AU, "CH6 CTLE",            "0x09: CTLE about 17.7dB"},
    {0x3BU, "CH6 MODE/VOD",        "0x42: nonlinear mode, VOD about 0.8Vp-p"},
    {0x3CU, "CH6 DEM",             "0x02: DEM about -5dB"},

    {0x4EU, "override route",      "0x07: reg controls SEL/INPUT_EN"},
    {0x4FU, "route",               "0x50: LT1258-like route, fanout off"}
};

static const uint8_t g_clrd125cDeviceAddrList[CLRD125C_DEVICE_NUM] =
{
    0x58U,
    0x5CU,
    0x60U,
    0x64U
};

typedef struct
{
    uint8_t bus;
    uint8_t addr7;
    const char *name;
} CLRD125C_FIXED_PROBE_ITEM;

static const CLRD125C_FIXED_PROBE_ITEM g_clrd125cFixedProbeList[] =
{
    {GPIO_I2C1, 0x58U, "CLRD125C V1.0 AD=0000"},
    {GPIO_I2C1, 0x5CU, "CLRD125C V1.0 AD=0100 / wire B8"},
    {GPIO_I2C1, 0x1CU, "LT1258 legacy wire 38"}
};

static uint8_t clrd125cWireReg(uint8_t regAddr)
{
    return (uint8_t)(g_clrd125cWireBase + regAddr);
}

static uint8_t clrd125cIsAllowedVerifyDiff(uint8_t regIndex)
{
    /*
     * 当前实测4片CLRD125C均存在以下回读差异：
     * D[0]  / wire 0x80：读回值随芯片地址变化，疑似状态/地址相关只读位。
     * D[69] / wire 0xC5：写0xAA后固定读回0x5A。
     * D[70] / wire 0xC6：写0xA0后固定读回0xA5。
     *
     * 这些差异不作为通信或配置失败处理，只打印WARN用于追踪。
     */
    return ((regIndex == 0U) || (regIndex == 69U) || (regIndex == 70U)) ? 1U : 0U;
}

static const CLRD125C_KEY_REG_INFO *clrd125cFindKeyRegInfo(uint8_t regIndex)
{
    uint32_t i;

    for (i = 0U;
         i < (sizeof(g_clrd125cKeyRegInfo) / sizeof(g_clrd125cKeyRegInfo[0]));
         i++)
    {
        if (g_clrd125cKeyRegInfo[i].reg == regIndex)
        {
            return &g_clrd125cKeyRegInfo[i];
        }
    }

    return 0;
}

ERR_STA clrd125cRead(uint8_t regAddr, uint8_t *val)
{
#if (CLRD125C_CONFIG_MODE == CLRD125C_CONFIG_MODE_PIN)
    (void)regAddr;
    (void)val;
    return STA_ERR;
#else
    if (val == 0)
    {
        return STA_ERR;
    }

    return gpioI2cRead(CLRD125C_I2C,
                       g_clrd125cDeviceAddr,
                       clrd125cWireReg(regAddr),
                       val,
                       1U);
#endif
}

ERR_STA clrd125cWrite(uint8_t regAddr, uint8_t val)
{
#if (CLRD125C_CONFIG_MODE == CLRD125C_CONFIG_MODE_PIN)
    (void)regAddr;
    (void)val;
    return STA_ERR;
#else
    uint8_t data;

    data = val;
    return gpioI2cWrite(CLRD125C_I2C,
                        g_clrd125cDeviceAddr,
                        clrd125cWireReg(regAddr),
                        &data,
                        1U);
#endif
}

static ERR_STA clrd125cReadRetry(uint8_t regAddr, uint8_t *value)
{
    uint8_t retry;

    for (retry = 0U; retry < CLRD125C_RETRY_CNT; retry++)
    {
        if (STA_OK == clrd125cRead(regAddr, value))
        {
            return STA_OK;
        }
        delay_1ms(2U);
    }

    printf("[CLRD125C] read failed: reg=%u wire=0x%02X\r\n",
           regAddr, clrd125cWireReg(regAddr));
    return STA_ERR;
}

static ERR_STA clrd125cWriteRetry(uint8_t regAddr, uint8_t value)
{
    uint8_t retry;

    for (retry = 0U; retry < CLRD125C_RETRY_CNT; retry++)
    {
        if (STA_OK == clrd125cWrite(regAddr, value))
        {
            return STA_OK;
        }
        delay_1ms(2U);
    }

    printf("[CLRD125C] write failed: reg=%u value=0x%02X\r\n",
           regAddr, value);
    return STA_ERR;
}

static ERR_STA clrd125cUpdateAndVerify(const CLRD125C_CFG_ITEM *item)
{
    uint8_t oldValue;
    uint8_t writeValue;
    uint8_t readValue;

    if (item == 0)
    {
        return STA_ERR;
    }
    if (STA_OK != clrd125cReadRetry(item->reg, &oldValue))
    {
        return STA_ERR;
    }

    writeValue = (uint8_t)((oldValue & (uint8_t)(~item->mask)) |
                           (item->value & item->mask));
    if (STA_OK != clrd125cWriteRetry(item->reg, writeValue))
    {
        return STA_ERR;
    }
    if (STA_OK != clrd125cReadRetry(item->reg, &readValue))
    {
        return STA_ERR;
    }
    if ((readValue & item->mask) != (writeValue & item->mask))
    {
        printf("[CLRD125C] verify failed: %s reg=%u expected=0x%02X actual=0x%02X\r\n",
               item->name, item->reg, writeValue, readValue);
        return STA_ERR;
    }

    printf("[CLRD125C] %-23s reg=%u old=0x%02X new=0x%02X\r\n",
           item->name, item->reg, oldValue, readValue);
    return STA_OK;
}

static ERR_STA clrd125cFindAddress(void)
{
    uint8_t candidateAddr;
    uint8_t lineState;

    gpioI2cInit(CLRD125C_I2C);

    lineState = gpioI2cGetLineState(CLRD125C_I2C);
    printf("[CLRD125C] bus idle: SCL=%u SDA=%u\r\n",
           (lineState >> 1) & 0x01U,
           lineState & 0x01U);
    if (lineState != 0x03U)
    {
        printf("[CLRD125C] bus is not idle-high; check pull-ups, power and shorts\r\n");
        return STA_ERR;
    }

    g_clrd125cWireBase = CLRD125C_REG_WIRE_BASE;

    printf("[CLRD125C] scan V1.0 as 7-bit address: 7-bit 0x%02X~0x%02X, wire-write 0x%02X~0x%02X\r\n",
           CLRD125C_DEVICE_ADDR_V1_MIN,
           CLRD125C_DEVICE_ADDR_V1_MAX,
           (uint8_t)(CLRD125C_DEVICE_ADDR_V1_MIN << 1),
           (uint8_t)(CLRD125C_DEVICE_ADDR_V1_MAX << 1));
    for (candidateAddr = CLRD125C_DEVICE_ADDR_V1_MIN;
         candidateAddr <= CLRD125C_DEVICE_ADDR_V1_MAX;
         candidateAddr++)
    {
        printf("[CLRD125C] probe V1.0 addr-as-7bit 7-bit=0x%02X wire-write=0x%02X ... ",
               candidateAddr,
               (uint8_t)(candidateAddr << 1));
        if (STA_OK == gpioI2cProbe(CLRD125C_I2C, candidateAddr))
        {
            printf("ACK\r\n");
            g_clrd125cDeviceAddr = candidateAddr;
            return STA_OK;
        }
        printf("NACK\r\n");
    }

    printf("[CLRD125C] no candidate address responded\r\n");
    return STA_ERR;
}

static uint8_t clrd125cScanRange(uint8_t bus,
                                 const char *title,
                                 uint8_t addrMin,
                                 uint8_t addrMax,
                                 uint8_t printNack)
{
    uint8_t candidateAddr;
    uint8_t ackCount;

    ackCount = 0U;
    printf("\r\n[CLRD125C-SCAN] bus=I2C%u === %s ===\r\n",
           bus,
           title);

    for (candidateAddr = addrMin; candidateAddr <= addrMax; candidateAddr++)
    {
        if (STA_OK == gpioI2cProbe(bus, candidateAddr))
        {
            ackCount++;
            printf("[CLRD125C-SCAN] bus=I2C%u ACK  7-bit=0x%02X wire-write=0x%02X wire-read=0x%02X\r\n",
                   bus,
                   candidateAddr,
                   (uint8_t)(candidateAddr << 1),
                   (uint8_t)((candidateAddr << 1) | 0x01U));
        }
        else if (printNack != 0U)
        {
            printf("[CLRD125C-SCAN] bus=I2C%u NACK 7-bit=0x%02X wire-write=0x%02X wire-read=0x%02X\r\n",
                   bus,
                   candidateAddr,
                   (uint8_t)(candidateAddr << 1),
                   (uint8_t)((candidateAddr << 1) | 0x01U));
        }
    }

    printf("[CLRD125C-SCAN] bus=I2C%u %s result: %u ACK\r\n",
           bus,
           title,
           ackCount);

    return ackCount;
}

static uint8_t clrd125cScan8BitWriteRange(uint8_t bus,
                                           uint8_t wireWriteMin,
                                           uint8_t wireWriteMax)
{
    uint16_t wireWrite;
    uint8_t addr7;
    uint8_t ackCount;

    ackCount = 0U;
    printf("\r\n[CLRD125C-SCAN8] bus=I2C%u === full 8-bit WRITE address range ===\r\n",
           bus);
    printf("[CLRD125C-SCAN8] scan even wire-write bytes 0x%02X~0x%02X, ACK only\r\n",
           wireWriteMin,
           wireWriteMax);

    for (wireWrite = wireWriteMin; wireWrite <= wireWriteMax; wireWrite = (uint16_t)(wireWrite + 2U))
    {
        addr7 = (uint8_t)(wireWrite >> 1);
        if (STA_OK == gpioI2cProbe(bus, addr7))
        {
            ackCount++;
            printf("[CLRD125C-SCAN8] bus=I2C%u ACK  wire-write=0x%02X wire-read=0x%02X equivalent-7bit=0x%02X\r\n",
                   bus,
                   (uint8_t)wireWrite,
                   (uint8_t)(wireWrite | 0x01U),
                   addr7);
        }
    }

    printf("[CLRD125C-SCAN8] bus=I2C%u full 8-bit WRITE result: %u ACK\r\n",
           bus,
           ackCount);

    return ackCount;
}

ERR_STA clrd125cLt1258StyleTest(void)
{
#if (CLRD125C_TEST_MODE == CLRD125C_TEST_MODE_SCAN_ONLY)
    uint8_t bus;
    uint8_t lineState;
    uint8_t v1AckCount;
    uint8_t ltAckCount;
    uint8_t targetAckCount;

    printf("\r\n[CLRD125C-SCAN] CLRD125C bus suspected-address scan only, no register write\r\n");
    printf("[CLRD125C-SCAN] step1: scan CLRD125C V1.0 address table, wire-write 0xB0~0xCE\r\n");
    printf("[CLRD125C-SCAN] step1: scan as 7-bit address 0x%02X~0x%02X\r\n",
           CLRD125C_DEVICE_ADDR_V1_MIN,
           CLRD125C_DEVICE_ADDR_V1_MAX);
    printf("[CLRD125C-SCAN] step2: scan LT1258 legacy-compatible address range, wire-write 0x30~0x4E\r\n");
    printf("[CLRD125C-SCAN] step2: scan as 7-bit address 0x%02X~0x%02X\r\n",
           CLRD125C_DEVICE_ADDR_LT_MIN,
           CLRD125C_DEVICE_ADDR_LT_MAX);
    printf("[CLRD125C-SCAN] bus: I2C%u only\r\n", CLRD125C_I2C);

    delay_1ms(CLRD125C_POWER_ON_DELAY_MS);
    g_clrd125cWireBase = CLRD125C_REG_WIRE_BASE;
    bus = CLRD125C_I2C;
    gpioI2cInit(bus);
    lineState = gpioI2cGetLineState(bus);
    printf("\r\n[CLRD125C-SCAN] bus=I2C%u idle line: SCL=%u SDA=%u raw=0x%02X\r\n",
           bus,
           (unsigned int)((lineState >> 1) & 0x01U),
           (unsigned int)(lineState & 0x01U),
           lineState);

    v1AckCount = clrd125cScanRange(bus,
                                   "CLRD125C V1.0 range",
                                   CLRD125C_DEVICE_ADDR_V1_MIN,
                                   CLRD125C_DEVICE_ADDR_V1_MAX,
                                   1U);
    ltAckCount = clrd125cScanRange(bus,
                                   "LT1258 legacy-compatible range",
                                   CLRD125C_DEVICE_ADDR_LT_MIN,
                                   CLRD125C_DEVICE_ADDR_LT_MAX,
                                   1U);

    targetAckCount = (uint8_t)(v1AckCount + ltAckCount);

    printf("\r\n[CLRD125C-SCAN] bus=I2C%u summary: V1.0_ACK=%u, LT1258_legacy_ACK=%u\r\n",
           bus,
           v1AckCount,
           ltAckCount);
    printf("[CLRD125C-SCAN] total CLRD125C candidate ACK on I2C%u: %u\r\n",
           bus,
           targetAckCount);
    if (targetAckCount == 0U)
    {
        printf("[CLRD125C-SCAN] CLRD125C not found in V1.0 or LT1258 legacy address range\r\n");
    }
    else
    {
        printf("[CLRD125C-SCAN] use the CLRD125C candidate ACK 7-bit address(es) to fill g_clrd125cDeviceAddrList[]\r\n");
    }

    return (targetAckCount > 0U) ? STA_OK : STA_ERR;
#elif (CLRD125C_TEST_MODE == CLRD125C_TEST_MODE_FIXED_PROBE)
    uint32_t round;
    uint32_t i;
    uint8_t bus;
    uint8_t lineState;
    ERR_STA result;

    printf("\r\n[CLRD125C-FIXED] fixed address probe start, no register write\r\n");
    printf("[CLRD125C-FIXED] probe list: B0(7-bit 58), B8(7-bit 5C), 38(7-bit 1C) on I2C1 only\r\n");
    printf("[CLRD125C-FIXED] interval=%ums, use scope/logic analyzer to decode SCL/SDA\r\n",
           CLRD125C_FIXED_PROBE_INTERVAL_MS);

    delay_1ms(CLRD125C_POWER_ON_DELAY_MS);
    g_clrd125cWireBase = CLRD125C_REG_WIRE_BASE;

    bus = GPIO_I2C1;
    gpioI2cInit(bus);
    lineState = gpioI2cGetLineState(bus);
    printf("[CLRD125C-FIXED] bus=I2C%u idle line: SCL=%u SDA=%u raw=0x%02X\r\n",
           bus,
           (unsigned int)((lineState >> 1) & 0x01U),
           (unsigned int)(lineState & 0x01U),
           lineState);

    round = 0U;
    while (1)
    {
        round++;
        printf("\r\n[CLRD125C-FIXED] round %lu\r\n", (unsigned long)round);

        for (i = 0U;
             i < (sizeof(g_clrd125cFixedProbeList) / sizeof(g_clrd125cFixedProbeList[0]));
             i++)
        {
            bus = g_clrd125cFixedProbeList[i].bus;
            result = gpioI2cProbe(bus, g_clrd125cFixedProbeList[i].addr7);

            printf("[CLRD125C-FIXED] bus=I2C%u %-34s 7-bit=0x%02X wire-write=0x%02X wire-read=0x%02X ... %s\r\n",
                   bus,
                   g_clrd125cFixedProbeList[i].name,
                   g_clrd125cFixedProbeList[i].addr7,
                   (uint8_t)(g_clrd125cFixedProbeList[i].addr7 << 1),
                   (uint8_t)((g_clrd125cFixedProbeList[i].addr7 << 1) | 0x01U),
                   (result == STA_OK) ? "ACK" : "NACK");

            delay_1ms(CLRD125C_FIXED_PROBE_INTERVAL_MS);
        }
    }
#else
    uint32_t i;
    uint32_t dev;
    uint8_t readValue;
    uint16_t writeFailCount;
    uint16_t readFailCount;
    uint16_t verifyFailCount;
    uint16_t verifyWarnCount;
    uint16_t totalWriteFailCount;
    uint16_t totalReadFailCount;
    uint16_t totalVerifyFailCount;
    uint16_t totalVerifyWarnCount;
    uint16_t totalAddrFailCount;
    uint8_t writeOk[CLRD125C_REG_NUM];
    const char *resultText;
    const CLRD125C_KEY_REG_INFO *keyInfo;
    uint8_t lineState;

    printf("\r\n[CLRD125C-CONFIG] four-device full-table configure/readback start\r\n");
    delay_1ms(CLRD125C_POWER_ON_DELAY_MS);

    gpioI2cInit(CLRD125C_I2C);
    g_clrd125cWireBase = CLRD125C_REG_WIRE_BASE;
    lineState = gpioI2cGetLineState(CLRD125C_I2C);
    printf("[CLRD125C-CONFIG] bus=I2C%u idle line: SCL=%u SDA=%u raw=0x%02X\r\n",
           CLRD125C_I2C,
           (unsigned int)((lineState >> 1) & 0x01U),
           (unsigned int)(lineState & 0x01U),
           lineState);

    totalWriteFailCount = 0U;
    totalReadFailCount = 0U;
    totalVerifyFailCount = 0U;
    totalVerifyWarnCount = 0U;
    totalAddrFailCount = 0U;

    for (dev = 0U; dev < CLRD125C_DEVICE_NUM; dev++)
    {
        g_clrd125cDeviceAddr = g_clrd125cDeviceAddrList[dev];
        writeFailCount = 0U;
        readFailCount = 0U;
        verifyFailCount = 0U;
        verifyWarnCount = 0U;

#if (CLRD125C_PRINT_DEVICE_CONFIG != 0U)
        printf("\r\n[CLRD125C-CONFIG] device%u selected: 7-bit=0x%02X write=0x%02X read=0x%02X reg-base=0x%02X\r\n",
               (unsigned int)dev,
               g_clrd125cDeviceAddr,
               (uint8_t)(g_clrd125cDeviceAddr << 1),
               (uint8_t)((g_clrd125cDeviceAddr << 1) | 0x01U),
               g_clrd125cWireBase);
#endif

        if (STA_OK == gpioI2cProbe(CLRD125C_I2C, g_clrd125cDeviceAddr))
        {
#if (CLRD125C_PRINT_DEVICE_CONFIG != 0U)
            printf("[CLRD125C-CONFIG] device%u address probe ACK\r\n",
                   (unsigned int)dev);
#endif
        }
        else
        {
            totalAddrFailCount++;
            writeFailCount = CLRD125C_REG_NUM;
            printf("[CLRD125C-CONFIG] device%u address probe NACK: addr7=0x%02X write=0x%02X, skip register write\r\n",
                   (unsigned int)dev,
                   g_clrd125cDeviceAddr,
                   (uint8_t)(g_clrd125cDeviceAddr << 1));
            totalWriteFailCount = (uint16_t)(totalWriteFailCount + writeFailCount);
            continue;
        }

#if (CLRD125C_PRINT_DEVICE_CONFIG != 0U)
        printf("[CLRD125C-CONFIG] device%u write full %s table D[0]~D[79]\r\n",
               (unsigned int)dev,
               CLRD125C_ACTIVE_CONFIG_NAME);
#endif
        for (i = 0U; i < CLRD125C_REG_NUM; i++)
        {
            if (STA_OK != clrd125cWriteRetry((uint8_t)i, CLRD125C_ACTIVE_CONFIG[i]))
            {
                writeOk[i] = 0U;
                writeFailCount++;
                printf("[CLRD125C-CONFIG] device%u WRITE FAIL D[%u] wire=0x%02X value=0x%02X\r\n",
                       (unsigned int)dev,
                       (unsigned int)i,
                       clrd125cWireReg((uint8_t)i),
                       CLRD125C_ACTIVE_CONFIG[i]);
            }
            else
            {
                writeOk[i] = 1U;
            }
        }

#if (CLRD125C_VERIFY_CONFIG_READBACK != 0U)
#if (CLRD125C_PRINT_KEY_CONFIG != 0U)
        printf("[CLRD125C-CONFIG] device%u verify key settings after full readback\r\n",
               (unsigned int)dev);
#endif
        for (i = 0U; i < CLRD125C_REG_NUM; i++)
        {
            readValue = 0U;
#if (CLRD125C_PRINT_KEY_CONFIG != 0U)
            keyInfo = clrd125cFindKeyRegInfo((uint8_t)i);
#else
            keyInfo = 0;
#endif
            if (STA_OK != clrd125cReadRetry((uint8_t)i, &readValue))
            {
                readFailCount++;
                printf("[CLRD125C-CONFIG] device%u D[%02u] wire=0x%02X write=0x%02X read=-- READ_FAIL %s\r\n",
                       (unsigned int)dev,
                       (unsigned int)i,
                       clrd125cWireReg((uint8_t)i),
                       CLRD125C_ACTIVE_CONFIG[i],
                       (keyInfo != 0) ? keyInfo->name : "");
                continue;
            }

            if (readValue != CLRD125C_ACTIVE_CONFIG[i])
            {
                if (0U != clrd125cIsAllowedVerifyDiff((uint8_t)i))
                {
                    verifyWarnCount++;
                    resultText = "WARN_ALLOWED_DIFF";
                }
                else
                {
                    verifyFailCount++;
                    resultText = "DIFF";
                }
            }
            else
            {
                resultText = "OK";
            }

            if ((keyInfo != 0) || (resultText[0] == 'D') || (writeOk[i] == 0U))
            {
                if (keyInfo != 0)
                {
                    printf("[CLRD125C-CONFIG] device%u %-18s D[%02u]/0x%02X W=0x%02X R=0x%02X %-17s %s\r\n",
                           (unsigned int)dev,
                           keyInfo->name,
                           (unsigned int)i,
                           clrd125cWireReg((uint8_t)i),
                           CLRD125C_ACTIVE_CONFIG[i],
                           readValue,
                           resultText,
                           keyInfo->meaning);
                }
                else
                {
                    printf("[CLRD125C-CONFIG] device%u D[%02u]/0x%02X W=0x%02X R=0x%02X %s\r\n",
                           (unsigned int)dev,
                           (unsigned int)i,
                           clrd125cWireReg((uint8_t)i),
                           CLRD125C_ACTIVE_CONFIG[i],
                           readValue,
                           resultText);
                }
            }
        }
#endif

        if ((CLRD125C_PRINT_DEVICE_CONFIG != 0U) ||
            (writeFailCount != 0U) ||
            (readFailCount != 0U) ||
            (verifyFailCount != 0U))
        {
            printf("[CLRD125C-CONFIG] device%u summary: write_fail=%u read_fail=%u verify_diff=%u verify_warn=%u => %s\r\n",
                   (unsigned int)dev,
                   writeFailCount,
                   readFailCount,
                   verifyFailCount,
                   verifyWarnCount,
                   ((writeFailCount == 0U) && (readFailCount == 0U) && (verifyFailCount == 0U)) ? "OK" : "FAIL");
        }

        totalWriteFailCount = (uint16_t)(totalWriteFailCount + writeFailCount);
        totalReadFailCount = (uint16_t)(totalReadFailCount + readFailCount);
        totalVerifyFailCount = (uint16_t)(totalVerifyFailCount + verifyFailCount);
        totalVerifyWarnCount = (uint16_t)(totalVerifyWarnCount + verifyWarnCount);
    }

    printf("\r\n[CLRD125C-CONFIG] total summary: table=%s readback=%s addr_fail=%u write_fail=%u read_fail=%u verify_diff=%u verify_warn=%u => %s\r\n",
           CLRD125C_ACTIVE_CONFIG_NAME,
#if (CLRD125C_VERIFY_CONFIG_READBACK != 0U)
           "ON",
#else
           "OFF",
#endif
           totalAddrFailCount,
           totalWriteFailCount,
           totalReadFailCount,
           totalVerifyFailCount,
           totalVerifyWarnCount,
           ((totalAddrFailCount == 0U) && (totalWriteFailCount == 0U) && (totalReadFailCount == 0U) && (totalVerifyFailCount == 0U)) ? "OK" : "FAIL");

    return ((totalAddrFailCount == 0U) &&
            (totalWriteFailCount == 0U) &&
            (totalReadFailCount == 0U) &&
            (totalVerifyFailCount == 0U)) ? STA_OK : STA_ERR;
#endif
}

ERR_STA clrd125cInit(void)
{
    uint32_t i;
    uint8_t probeValue;
    uint8_t rxDetect;
    uint8_t rxLos;

    printf("\r\n[CLRD125C] V1.0 SMBus initialization start\r\n");
    if (STA_OK != clrd125cFindAddress())
    {
        return STA_ERR;
    }

    printf("[CLRD125C] selected: 7-bit=0x%02X write=0x%02X read=0x%02X reg-base=0x%02X\r\n",
           g_clrd125cDeviceAddr,
           (uint8_t)(g_clrd125cDeviceAddr << 1),
           (uint8_t)((g_clrd125cDeviceAddr << 1) | 0x01U),
           g_clrd125cWireBase);

    /*
     * V1.0已取消旧版reg0x44=0x55器件ID定义。
     * 此处读取reg0只用于确认当前从机地址有应答，不把返回值当作器件ID。
     */
    if (STA_OK != clrd125cReadRetry(0x00U, &probeValue))
    {
        printf("[CLRD125C] probe failed: slave=0x%02X reg=0x%02X\r\n",
               g_clrd125cDeviceAddr, clrd125cWireReg(0x00U));
        return STA_ERR;
    }

    printf("[CLRD125C] apply V1.0 LT1258-equivalent profile\r\n");
    for (i = 0U; i < (sizeof(g_clrd125cConfig) / sizeof(g_clrd125cConfig[0])); i++)
    {
        if (STA_OK != clrd125cUpdateAndVerify(&g_clrd125cConfig[i]))
        {
            return STA_ERR;
        }
    }

    if (STA_OK == clrd125cReadStatus(&rxDetect, &rxLos))
    {
        printf("[CLRD125C] RX_DETECT=0x%02X RX_LOS=0x%02X\r\n",
               rxDetect, rxLos);
    }

    printf("[CLRD125C] V1.0 initialization complete\r\n");
    return STA_OK;
}

ERR_STA clrd125cInitWithRetry(void)
{
    uint8_t attempt;

    /* 数据手册要求上电后至少等待500 ms再访问SMBus寄存器。 */
    delay_1ms(CLRD125C_POWER_ON_DELAY_MS);

    for (attempt = 1U; attempt <= CLRD125C_RETRY_CNT; attempt++)
    {
        if (STA_OK == clrd125cInit())
        {
            printf("[CLRD125C] initialized on attempt %u\r\n", attempt);
            return STA_OK;
        }

        printf("[CLRD125C] attempt %u/%u failed\r\n",
               attempt, CLRD125C_RETRY_CNT);
        delay_1ms(CLRD125C_RETRY_DELAY_MS);
    }

    printf("[CLRD125C] initialization failed\r\n");
    return STA_ERR;
}

const char *clrd125cGetConfigModeName(void)
{
#if (CLRD125C_CONFIG_MODE == CLRD125C_CONFIG_MODE_PIN)
    return "PIN";
#else
    return "SMBUS";
#endif
}

ERR_STA clrd125cInitSelectedMode(void)
{
#if (CLRD125C_CONFIG_MODE == CLRD125C_CONFIG_MODE_PIN)
    printf("\r\n[CLRD125C] PIN mode selected\r\n");
    printf("[CLRD125C] SMBus access disabled; settings are sampled from pins\r\n");
    return STA_OK;
#else
    return clrd125cInitWithRetry();
#endif
}

ERR_STA clrd125cReadStatus(uint8_t *rxDetect, uint8_t *rxLos)
{
    if ((rxDetect == 0) || (rxLos == 0))
    {
        return STA_ERR;
    }
    if (STA_OK != clrd125cReadRetry(CLRD125C_REG_RX_DETECT, rxDetect))
    {
        return STA_ERR;
    }

    return clrd125cReadRetry(CLRD125C_REG_RX_LOS, rxLos);
}

void clrd125cPrintAllLosStatus(void)
{
#if (CLRD125C_CONFIG_MODE == CLRD125C_CONFIG_MODE_PIN)
    printf("[CLRD125C-LOS] skipped in PIN mode\r\n");
#else
    uint32_t dev;
    uint8_t oldAddr;
    uint8_t reg0;
    uint8_t rxdetSel;
    uint8_t rxDetect;
    uint8_t rxLosb;
    uint8_t enRxLos;

    oldAddr = g_clrd125cDeviceAddr;

    for (dev = 0U; dev < CLRD125C_DEVICE_NUM; dev++)
    {
        g_clrd125cDeviceAddr = g_clrd125cDeviceAddrList[dev];
        reg0 = 0U;
        enRxLos = 0U;
        rxdetSel = 0U;
        rxDetect = 0U;
        rxLosb = 0U;

        if ((STA_OK == clrd125cReadRetry(0x00U, &reg0)) &&
            (STA_OK == clrd125cReadRetry(0x05U, &enRxLos)) &&
            (STA_OK == clrd125cReadRetry(0x44U, &rxdetSel)) &&
            (STA_OK == clrd125cReadStatus(&rxDetect, &rxLosb)))
        {
            printf("[CLRD125C-CH] dev%u 0x%02X reg70=0x%02X CH[7:0]=%u%u%u%u%u%u%u%u focus(CH1=%u CH3=%u CH4=%u CH6=%u)\r\n",
                   (unsigned int)dev,
                   g_clrd125cDeviceAddr,
                   rxLosb,
                   (unsigned int)((rxLosb >> 7) & 0x01U),
                   (unsigned int)((rxLosb >> 6) & 0x01U),
                   (unsigned int)((rxLosb >> 5) & 0x01U),
                   (unsigned int)((rxLosb >> 4) & 0x01U),
                   (unsigned int)((rxLosb >> 3) & 0x01U),
                   (unsigned int)((rxLosb >> 2) & 0x01U),
                   (unsigned int)((rxLosb >> 1) & 0x01U),
                   (unsigned int)((rxLosb >> 0) & 0x01U),
                   (unsigned int)((rxLosb >> 1) & 0x01U),
                   (unsigned int)((rxLosb >> 3) & 0x01U),
                   (unsigned int)((rxLosb >> 4) & 0x01U),
                   (unsigned int)((rxLosb >> 6) & 0x01U));
        }
        else
        {
            printf("[CLRD125C-CH] dev%u 0x%02X read status fail\r\n",
                   (unsigned int)dev,
                   g_clrd125cDeviceAddr);
        }
    }

    g_clrd125cDeviceAddr = oldAddr;
#endif
}
