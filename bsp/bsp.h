/*!
    \file  bsp.h
    \brief 主要实现板级资源管理，单片机的GPIO，RCU相关初始化实现
*/

#ifndef __BSP__H
#define __BSP__H


#include "gd32f4xx.h"
#include "led.h"
#include "gn1406.h"
#include "cb2991.h"
#include "other_io.h"
#include "spi_flash.h"

/*!
    \brief      enable the peripheral clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern void rcu_config(void);

/*!
    \brief      板卡的所有GPIO初始化
    \param  	void
    \retval     void
*/
extern void bspGpioInit(void);

void LEDGreen_SS(uint8_t ledx,uint8_t i);
/*!
    \brief      UART初始化
    \param  	void
    \retval     void
*/
extern void uartInit(void);

#endif /* __BSP__H */

