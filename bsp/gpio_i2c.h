/*!
    \file  gpio_i2c.h
    \brief 
*/

#ifndef _GPIO_I2C_H
#define _GPIO_I2C_H

#include "gd32f4xx.h"
#include "common.h"

/*******************硬件信号定义*******************/
#define GPIO_I2C_NUM 2
/*******************软件定义*******************/
#define GPIO_I2C0 0
#define GPIO_I2C1 1

/*!
    \brief      GPIO模拟I2C初始化
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
    \retval     void
*/
extern void gpioI2cInit(uint8_t gpioI2cx);

/*
 * 只发送从机写地址并检查ACK，不访问寄存器。
 * 用于区分“器件地址错误”和“寄存器访问格式错误”。
 */
extern ERR_STA gpioI2cProbe(uint8_t gpioI2cx, uint8_t slaveAddr);

/*
 * 读取总线当前空闲电平：bit1=SCL，bit0=SDA。
 * 正常空闲状态应为0x03；其他值通常表示缺少上拉、短路或从机拉低总线。
 */
extern uint8_t gpioI2cGetLineState(uint8_t gpioI2cx);

/*! 
    \brief      GPIO模拟I2C读操作
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
				uint8_t slaveAddr:从机地址
				uint8_t reg:寄存器地址
				uint8_t *buf:读取的数据
				uint8_t size:读取多少字节
    \retval     ERR_STA:STA_OK/STA_ERR
*/
extern ERR_STA gpioI2cRead(uint8_t gpioI2cx, uint8_t slaveAddr, uint8_t reg, uint8_t *buf, uint8_t size);
/*!
    \brief      GPIO模拟I2C读操作
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
				uint8_t slaveAddr:从机地址
				uint8_t reg:寄存器地址
				uint8_t *buf:写入的数据
				uint8_t size:写入多少字节
    \retval     ERR_STA:STA_OK/STA_ERR
*/
extern ERR_STA gpioI2cWrite(uint8_t gpioI2cx, uint8_t slaveAddr, uint8_t reg, uint8_t *buf, uint8_t size);

//extern void gpioI2cTxDat(uint8_t gpioI2cx, uint8_t dat);



#endif /* LED_H */

