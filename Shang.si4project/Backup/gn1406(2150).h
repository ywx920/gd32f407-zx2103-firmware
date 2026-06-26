/*!
    \file  gn1406.c
    \brief 
*/

#ifndef GN1406_H
#define GN1406_H


#include "gd32f4xx.h"
#include "gpio_i2c.h"


/*******************硬件定义*******************/
#define GN1406_ADDR0 0x14 //器件I2C地址
#define GN1406_ADDR1 0x16 //器件I2C地址
#define GN1406_ADDR2 0x1A //器件I2C地址
#define GN1406_ADDR3 0x1C //器件I2C地址
/*******************软件定义*******************/
#define GN1406_CHIP_NUM 4 //芯片个数

#define GN1406_CHIP0_D1 0 //第1个芯片 位号D1
#define GN1406_CHIP1_D3 1 //第2个芯片 位号D3
#define GN1406_CHIP2_D4 2 //第3个芯片 位号D4
#define GN1406_CHIP3_D5 3 //第4个芯片 位号D5

//速率配置
#define CFG_SPEED_NUM 4
#define CFG_6_25G 0
#define CFG_5G 1
#define CFG_3_125G 2
#define CFG_2_5G 3

//寄存器
#define GN1406_STA_REG 0x62 //Status Registers

/*!
    \brief      初始化gn1406芯片
    \param  	void
    \retval     ERR_STA
*/
extern ERR_STA gn1406Init(void);

/*!
    \brief      ¶ÁÈ¡gn1406µÄ×´Ì¬
    \param  	uint8_t chipx:ÄÄ¸öÐ¾Æ¬£¬È¡ÖµGN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
				uint8_t regAddr:¼Ä´æÆ÷µØÖ·
				uint16_t val:Ð´µ½µÄ¼Ä´æÆ÷Öµ
    \retval     ERR_STA
*/
ERR_STA gn1406Write(uint8_t chipx, uint8_t regAddr, uint16_t val);

/*!
    \brief      读取gn1406的状态
    \param  	uint8_t chipx:哪个芯片，取值GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
				uint8_t regAddr:寄存器地址
				uint16_t *val:读到的寄存器值
    \retval     ERR_STA
*/
ERR_STA gn1406Read(uint8_t chipx, uint8_t regAddr, uint16_t *val);

#endif /* GN1406_H */

