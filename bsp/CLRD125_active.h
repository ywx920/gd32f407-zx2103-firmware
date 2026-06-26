#ifndef __CLRD125_ACTIVE_H
#define __CLRD125_ACTIVE_H

#include "gd32f4xx.h"
#include "common.h"
#include "gpio_i2c.h"

/* gpio_i2c.c accepts a 7-bit address and shifts it before transmission. */
#define CLRD125_DEVICE_ADDR              0x30U
#define CLRD125_I2C                      GPIO_I2C1
#define CLRD125_DEVICE_ID                0x55U

#define CLRD125_RETRY_CNT                3U
#define CLRD125_POWER_ON_DELAY_MS        500U
#define CLRD125_RETRY_DELAY_MS           100U

#define CLRD125_REG_CHANNEL_PD           0x01U
#define CLRD125_REG_SMBUS_ENABLE         0x06U
#define CLRD125_REG_PIN_OVERRIDE         0x08U
#define CLRD125_REG_PIN_ROUTE_OVERRIDE   0x4EU
#define CLRD125_REG_ROUTE_FANOUT         0x4FU
#define CLRD125_REG_DEVICE_ID            0x44U
#define CLRD125_REG_RX_DETECT            0x45U
#define CLRD125_REG_RX_LOS               0x46U

/* Receiver CTLE registers used by the selected bidirectional paths. */
#define CLRD125_REG_CTLE_CH0             0x0FU /* S_INA0 */
#define CLRD125_REG_CTLE_CH2             0x1DU /* S_INA1 */
#define CLRD125_REG_CTLE_CH4             0x2CU /* D_IN0 */
#define CLRD125_REG_CTLE_CH6             0x3AU /* D_IN1 */

/* Transmitter VOD/mode/SCP registers. */
#define CLRD125_REG_VOD_CH1              0x17U /* D_OUT0 */
#define CLRD125_REG_VOD_CH3              0x25U /* D_OUT1 */
#define CLRD125_REG_VOD_CH4              0x2DU /* S_OUTA0 */
#define CLRD125_REG_VOD_CH5              0x34U /* S_OUTB0 */
#define CLRD125_REG_VOD_CH6              0x3BU /* S_OUTA1 */
#define CLRD125_REG_VOD_CH7              0x42U /* S_OUTB1 */

#define CLRD125_REG_DEM_CH1              0x18U
#define CLRD125_REG_DEM_CH3              0x26U
#define CLRD125_REG_DEM_CH4              0x2EU
#define CLRD125_REG_DEM_CH5              0x35U
#define CLRD125_REG_DEM_CH6              0x3CU
#define CLRD125_REG_DEM_CH7              0x43U

#define CLRD125_SMBUS_ENABLE_MASK        0x08U
#define CLRD125_PIN_OVERRIDE_MASK        0x0CU
#define CLRD125_ROUTE_OVERRIDE_MASK      0x07U
#define CLRD125_CTLE_MASK                0x0FU
#define CLRD125_VOD_MASK                 0x07U
#define CLRD125_DEM_MASK                 0x07U

/*
 * Board profile: select S_INA0/S_INA1 and enable D_IN0/D_IN1 fanout to
 * S_OUTA/S_OUTB. MODE and short-circuit-protection bits are preserved.
 */
#define CLRD125_ROUTE_FANOUT_VALUE       0xFCU
#define CLRD125_CTLE_VALUE               0x0FU /* maximum setting, up to 30 dB */
#define CLRD125_DEM_VALUE                0x01U /* -3.5 dB */
#define CLRD125_VOD_VALUE                0x04U /* 1.0 Vp-p */

ERR_STA clrd125Init(void);
ERR_STA clrd125InitWithRetry(void);
ERR_STA clrd125Read(uint8_t regAddr, uint8_t *val);
ERR_STA clrd125Write(uint8_t regAddr, uint8_t val);
ERR_STA clrd125ReadStatus(uint8_t *rxDetect, uint8_t *rxLos);

#endif
