/*!
    \file  spi.c
    \brief 
*/

#ifndef __SPI_H
#define __SPI_H


#include "gd32f4xx.h"

/*******************硬件信号定义*******************/
#define SPI_NUM 2

//片选操作定义
#define SET_NSS_HIGH(spix)          gpio_bit_set(spiTab[spix].port,spiTab[spix].pinCs);
#define SET_NSS_LOW(spix)           gpio_bit_reset(spiTab[spix].port,spiTab[spix].pinCs);

/*******************软件定义*******************/
//SPI 控制器编号
#define SPI_CTLER0 0
#define SPI_CTLER1 1

//定义端口和管脚的对应关系
typedef struct {
	uint32_t port;//端口
	uint32_t pinCs;
	uint32_t pinSck;
	uint32_t pinMiso;
	uint32_t pinMosi;
} SPI_PIX_TAB_TYP;

extern const SPI_PIX_TAB_TYP spiTab[SPI_NUM];
extern const uint32_t spiCtler[SPI_NUM];

/*!
    \brief      SPI GPIO初始化
    \param  	uint8_t spix:SPI_CTLER0/SPI_CTLER1
    \retval     void
*/
extern void spiInit(uint8_t spix);

#endif /* __SPI_H */

