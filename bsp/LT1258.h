/*!
    \file  LT1258.c
    \brief
*/

#ifndef LT1258_H
#define LT1258_H

#include "gd32f4xx.h"
#include "gpio_i2c.h"

// 芯片ID为0x8100, 0x8101，值应该始终为0x20, 0x02。0x8102是芯片版本寄存器，当前版本的值是0xE1
// 使能I2C通道1
#define LT1258_I2C GPIO_I2C1
/*******************硬件定义*******************/
// 用于I2C通信，指定与特定设备的通信目标地址
// LT1258 device address
#define LT1258_DEVICE_ADDR_1 0x18 // 0x30 >>1
#define LT1258_DEVICE_ADDR_2 0x1C // 0x38 >>1
#define LT1258_DEVICE_ADDR_3 0x20 // 0x40 >>1
#define LT1258_DEVICE_ADDR_4 0x24 // 0x48 >>1

/*!
    \brief      初始化LT1258芯片
    \param      void
    \retval     ERR_STA
*/
extern ERR_STA lt1258Init(void);

/*!
    \brief      ¶ÁÈ¡LT1258µÄ×´Ì¬
    \param      uint8_t chipx:ÄÄ¸öÐ¾Æ¬£¬È¡ÖµLT1258_CHIP0_D1/LT1258_CHIP1_D3/LT1258_CHIP2_D4/LT1258_CHIP3_D5
                uint8_t regAddr:¼Ä´æÆ÷µØÖ·
                uint16_t val:Ð´µ½µÄ¼Ä´æÆ÷Öµ
    \retval     ERR_STA
*/
ERR_STA lt1258Write(uint8_t deviceAddr, uint8_t regAddr, uint8_t val);

/*!
    \brief      读取LT1258的状态
    \param      uint8_t chipx:哪个芯片，取值LT1258_CHIP0_D1/LT1258_CHIP1_D3/LT1258_CHIP2_D4/LT1258_CHIP3_D5
                uint8_t regAddr:寄存器地址
                uint16_t *val:读到的寄存器值
    \retval     ERR_STA
*/
ERR_STA lt1258Read(uint8_t deviceAddr, uint8_t regAddr, uint8_t *val);

#endif /* LT1258_H */
