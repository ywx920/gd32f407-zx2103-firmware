/*!
    \file  led.c
    \brief 
*/

#ifndef LED_H
#define LED_H


#include "gd32f4xx.h"

/*******************硬件信号定义*******************/
//LED端口定义
#define LED_PORT0 GPIOA
#define LED_PORT1 GPIOD
//LED管脚号定义
#define DBG_LED0 GPIO_PIN_11 //GPIOA
#define DBG_LED1 GPIO_PIN_12 //GPIOA
#define F_LED0 GPIO_PIN_8 //GPIOD
#define F_LED1 GPIO_PIN_9 //GPIOD
#define F_LED2 GPIO_PIN_10//GPIOD
#define F_LED3 GPIO_PIN_11//GPIOD
#define F_LED4 GPIO_PIN_12//GPIOD
#define F_LED5 GPIO_PIN_13//GPIOD
#define F_LED6 GPIO_PIN_14//GPIOD
#define F_LED7 GPIO_PIN_15//GPIOD
#define F_LED8 GPIO_PIN_0//GPIOD
#define F_LED9 GPIO_PIN_1//GPIOD

#define LED_NUM 12
/*******************软件定义*******************/
//LED 编号
#define LED_F0 0 //前面板LED
#define LED_F1 1 //前面板LED
#define LED_F2 2 //前面板LED
#define LED_F3 3 //前面板LED
#define LED_F4 4 //前面板LED
#define F_LED_NUM 5//前面板LED个数为4个
#define LED_D0 4 //调试LED
#define LED_D1 5 //调试LED
#define D_LED_NUM 2//调试LED个数为2个
//LED控制状态
#define LED_OFF 	0 //关闭LED
#define LED_ON 		1 //打开LED
#define LED_GREEN 	2 //关闭LED
#define LED_RED 	3 //关闭LED
#define LED_TOG 	4 //翻转LED

/*!
    \brief      LED的初始化状态
    \param  	void
    \retval     void
*/
extern void ledInitSta(void);
/*!
    \brief      控制调试LED
    \param  	uint8_t ledx:LED_D0/LED_D1
				uint8_t sta:LED_OFF/LED_ON/LED_TOG
    \retval     void
*/
extern void dbgLedCtl(uint8_t ledx, uint8_t sta);
/*!
    \brief      控制LED打开
    \param  	uint8_t ledx:LED_F0/LED_F1/LED_F2/LED_F3/LED_F4
				uint8_t sta:LED_OFF/LED_GREEN/LED_RED
    \retval     void
*/
extern void fLedCtl(uint8_t ledx, uint8_t sta);

#endif /* LED_H */

