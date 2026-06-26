#ifndef CLRD125C_V1_H
#define CLRD125C_V1_H

#include "gd32f4xx.h"
#include "common.h"
#include "gpio_i2c.h"

/* CLRD125C控制方式：PIN为引脚配置模式，SMBUS为寄存器配置模式。 */
#define CLRD125C_CONFIG_MODE_PIN          0U
#define CLRD125C_CONFIG_MODE_SMBUS        1U

#ifndef CLRD125C_CONFIG_MODE
#define CLRD125C_CONFIG_MODE              CLRD125C_CONFIG_MODE_SMBUS
#endif

#ifndef CLRD125C_LT1258_STYLE_TEST
#define CLRD125C_LT1258_STYLE_TEST        0U
#endif

/*
 * CLRD125C调试测试模式：
 * 1) CLRD125C_TEST_MODE_SCAN_ONLY
 *    扫描V1.0手册规定的CLRD125C地址范围，同时扫描LT1258旧地址范围。
 *    只探测ACK，不写寄存器，用于确认当前板卡上4片CLRD125C实际响应哪些地址。
 *
 * 2) CLRD125C_TEST_MODE_WRITE_4DEV
 *    按g_clrd125cDeviceAddrList[]里假设的4个地址，写入完整80字节配置表，
 *    然后全部回读比对。
 *
 * 3) CLRD125C_TEST_MODE_FIXED_PROBE
 *    固定几个重点地址循环探测，只发地址检查ACK，不写寄存器。
 *    用于示波器/逻辑分析仪稳定抓取0xB0/0xB8/0x38等地址波形。
 */
#define CLRD125C_TEST_MODE_SCAN_ONLY      0U
#define CLRD125C_TEST_MODE_WRITE_4DEV     1U
#define CLRD125C_TEST_MODE_FIXED_PROBE    2U

#ifndef CLRD125C_TEST_MODE
#define CLRD125C_TEST_MODE                CLRD125C_TEST_MODE_WRITE_4DEV
#endif

#if ((CLRD125C_CONFIG_MODE != CLRD125C_CONFIG_MODE_PIN) && \
     (CLRD125C_CONFIG_MODE != CLRD125C_CONFIG_MODE_SMBUS))
#error "Invalid CLRD125C_CONFIG_MODE"
#endif

#if ((CLRD125C_TEST_MODE != CLRD125C_TEST_MODE_SCAN_ONLY) && \
     (CLRD125C_TEST_MODE != CLRD125C_TEST_MODE_WRITE_4DEV) && \
     (CLRD125C_TEST_MODE != CLRD125C_TEST_MODE_FIXED_PROBE))
#error "Invalid CLRD125C_TEST_MODE"
#endif

/* gpio_i2c.c接收7位地址并在发送前左移一位。 */
#define CLRD125C_DEVICE_ADDR_V1_MIN       0x58U
#define CLRD125C_DEVICE_ADDR_V1_MAX       0x67U
#define CLRD125C_DEVICE_ADDR_DEFAULT      CLRD125C_DEVICE_ADDR_V1_MIN
#define CLRD125C_DEVICE_NUM               4U
#define CLRD125C_I2C                      GPIO_I2C1

/* LT1258旧工程/旧手册中I2C/SMBus Device Address为0x30~0x4E，换算成7位地址为0x18~0x27。 */
#define CLRD125C_DEVICE_ADDR_LT_MIN       0x18U
#define CLRD125C_DEVICE_ADDR_LT_MAX       0x27U

/*
 * I2C 7位地址全覆盖扫描范围。
 * 注意：0x00包含General Call，0x78~0x7F属于特殊/保留地址。
 * 当前扫描只发送地址并检查ACK，不写寄存器；用于排查未知地址。
 */
#define CLRD125C_DEVICE_ADDR_FULL_MIN     0x00U
#define CLRD125C_DEVICE_ADDR_FULL_MAX     0x7FU

#define CLRD125C_FIXED_PROBE_INTERVAL_MS  500U

#define CLRD125C_RETRY_CNT                3U
#define CLRD125C_POWER_ON_DELAY_MS        500U
#define CLRD125C_RETRY_DELAY_MS           100U

/* CLRD125C V1.0 SMBus线上寄存器地址从0x80开始。 */
#define CLRD125C_REG_WIRE_BASE            0x80U
#define CLRD125C_REG_NUM                  80U

/* CLRD125C V1.0逻辑寄存器偏移。 */
#define CLRD125C_REG_CHANNEL_PD           0x01U
#define CLRD125C_REG_CONTROL_OVERRIDE     0x06U
#define CLRD125C_REG_EN_HF                0x07U
#define CLRD125C_REG_MODE_OVERRIDE        0x08U

#define CLRD125C_REG_CTLE_CH1             0x16U
#define CLRD125C_REG_MODE_VOD_CH1         0x17U
#define CLRD125C_REG_DEM_CH1              0x18U
#define CLRD125C_REG_CTLE_CH3             0x24U
#define CLRD125C_REG_MODE_VOD_CH3         0x25U
#define CLRD125C_REG_DEM_CH3              0x26U
#define CLRD125C_REG_CTLE_CH4             0x2CU
#define CLRD125C_REG_MODE_VOD_CH4         0x2DU
#define CLRD125C_REG_DEM_CH4              0x2EU
#define CLRD125C_REG_CTLE_CH6             0x3AU
#define CLRD125C_REG_MODE_VOD_CH6         0x3BU
#define CLRD125C_REG_DEM_CH6              0x3CU

#define CLRD125C_REG_RX_DETECT            0x45U
#define CLRD125C_REG_RX_LOS               0x46U
#define CLRD125C_REG_ROUTE_OVERRIDE       0x4EU
#define CLRD125C_REG_ROUTE                0x4FU

/* LT1258-equivalent profile. */
#define CLRD125C_CONTROL_OVERRIDE_VALUE   0xE0U
#define CLRD125C_MODE_OVERRIDE_VALUE      0x0CU
#define CLRD125C_ROUTE_OVERRIDE_VALUE     0x07U
#define CLRD125C_ROUTE_VALUE              0x50U
#define CLRD125C_ACTIVE_CHANNEL_MASK      0x5AU
#define CLRD125C_CTLE_VALUE               0x09U
#define CLRD125C_MODE_VOD_VALUE           0x42U
#define CLRD125C_DEM_VALUE                0x02U

#define CLRD125C_CTLE_MASK                0x1FU
#define CLRD125C_MODE_VOD_MASK            0x47U
#define CLRD125C_DEM_MASK                 0x07U

ERR_STA clrd125cInit(void);
ERR_STA clrd125cInitWithRetry(void);
ERR_STA clrd125cInitSelectedMode(void);
ERR_STA clrd125cLt1258StyleTest(void);
ERR_STA clrd125cRead(uint8_t regAddr, uint8_t *val);
ERR_STA clrd125cWrite(uint8_t regAddr, uint8_t val);
ERR_STA clrd125cReadStatus(uint8_t *rxDetect, uint8_t *rxLos);
void clrd125cPrintAllLosStatus(void);
const char *clrd125cGetConfigModeName(void);

#endif
