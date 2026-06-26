/*!
    \file  I2C1_IE.c
    \brief The header file of I2C0 and I2C1 interrupt
*/

/*
    Copyright (C) 2017 GigaDevice

    2015-07-15, V1.0.0, firmware for GD32F20x
    2017-06-05, V2.0.0, firmware for GD32F20x
*/

#ifndef I2C_IE_H
#define I2C_IE_H

#include "gd32f4xx.h"
#include "common.h"

//本机I2C地址
//#define SELF_I2C_ADDR 0x01
#define SELF_I2C_ADDR 0xC0

//I2C通道个数
#define I2C_NUM 3
//I2C接收缓冲区大小
#define I2C_RX_BUF_SIZE 64

#define I2Cx_NONE 0xFF

//速度
//#define I2C_CLK 200000
#define I2C_CLK 400000

#define OK 0
#define ERROR 1

/*
 * 接收缓冲区数据结果
 */
typedef struct I2C_RX_BUF_TYPE_t{
	uint8_t buf[I2C_RX_BUF_SIZE];//接收缓冲
	uint8_t rx_cnt;//接收数据的个数
	ERR_STA valid;//是否已经收到完整的一包数据
}I2C_RX_BUF_TYPE;

//是否发生了总线错误中断，0表示没有发生过，其他表示错误代码
extern uint32_t i2c_err_flag[I2C_NUM];
/*驱动编号转控制器编号*/
uint32_t index_to_I2Cx(uint8_t index);

/*控制器编号转驱动编号*/
uint8_t I2Cx_to_index(uint32_t I2Cx);

/*!
    \brief      复位并重新初始化I2C硬件
    \param[in]  uint32_t i2c_x：第几路I2C  I2C0/I2C1/I2C2
    \param[out] none
    \retval     none
*/
void int_i2c_hard(uint32_t i2c_x);
/*!
    \brief      获取自己的I2C从机地址
    \param[in]  uint32_t i2c_x：第几路I2C  I2C0/I2C1/I2C2
    \retval     自己的I2C地址
*/
uint8_t get_i2c_selfaddr(uint32_t i2c_x);

/*!
    \brief      cofigure the NVIC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_nvic_config(void);

/*!
    \brief      Initialize I2C clock address and mode
    \param[in]  uint32_t i2c_x：第几路I2C   I2C0/I2C1/I2C2
								uint32_t self_addr:自己的地址
								uint32_t speed:速度
    \param[out] none
    \retval     none
*/
void init_i2c(uint32_t i2c_x, uint32_t self_addr, uint32_t speed);

/*!
    \brief      等待一包完整的数据
    \param[in]  none
    \param[out] none
    \retval     返回有完整数据包的通道
*/
uint32_t wait_rx_pack(void);

/*!
    \brief      从指定通道拷贝数据
    \param[in]  uint32_t i2c_x：第几路I2C  I2C0/I2C1/I2C2
								uint8_t *recv_buf 用户的数据空间
    \param[out] none
    \retval     返回数据包的数据个数
*/
uint8_t recv_i2c_pack(uint32_t i2c_x, uint8_t *recv_buf);

/*!
    \brief      Completely send one byte of data from I2C1
    \param[in]  addr: addr of transmission
		\param[in]	data: data of transmission
    \param[out] none
    \retval     none
*/

void i2c_data_send(uint32_t i2c_x,uint32_t des_addr, uint8_t *snd_buf, uint8_t cnt);

/*!
    \brief      handle I2C event interrupt request
		\param[in]  uint32_t i2c_x：第几路I2C   I2C0/I2C1/I2C2
								uint8_t *data:数据
    \param[out] none
    \retval     none
*/
void I2C_EventIRQ_Handler(uint32_t i2c_x);

/*!
    \brief      handle I2C error interrupt request
    \param[in]  uint32_t i2c_x：第几路I2C   I2C0/I2C1/I2C2
    \param[out] none
    \retval     none
*/
void I2C_ErrorIRQ_Handler(uint32_t i2c_x);



#endif /* I2C_IE_H */

