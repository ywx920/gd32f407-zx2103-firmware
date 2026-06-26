/*!
    \file  bsp.c
    \brief 主要实现板级资源管理，单片机的GPIO，RCU相关初始化实现
*/
#include "gd32f4xx.h"
#include "bsp.h"
#include "led.h"
#include "serial.h"


/*!
    \brief      enable the peripheral clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
void rcu_config(void)
{
	//LED RCU
    rcu_periph_clock_enable(RCU_GPIOA); //LED USART 离散IO要用
    rcu_periph_clock_enable(RCU_GPIOB); //LED
    rcu_periph_clock_enable(RCU_GPIOC); //LED USART
    rcu_periph_clock_enable(RCU_GPIOD); //LED
    rcu_periph_clock_enable(RCU_GPIOE); //LED 离散IO要用
	
    rcu_periph_clock_enable(RCU_USART0);//USART0控制器
    rcu_periph_clock_enable(RCU_USART2);//USART2控制器
    
    rcu_periph_clock_enable(RCU_I2C0);//I2C0控制器
    rcu_periph_clock_enable(RCU_I2C1);//I2C1控制器
}

/*!
    \brief      板卡的上电和复位序列
    \param  	void
    \retval     void
*/
static void pwrRstInit(void)
{
	//复位控制管脚
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
	GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, \
	GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

	//拉高
	GPIO_BOP(GPIOD) = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	//拉低
	//GPIO_BOP(GPIOD) = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
}

/*!
    \brief      板卡的所有GPIO初始化
    \param  	void
    \retval     void
*/
void bspGpioInit(void)
{
	//LED PORTA
    gpio_mode_set(LED_PORT0, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
	DBG_LED0 | DBG_LED1);
    gpio_output_options_set(LED_PORT0, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, \
	DBG_LED0 | DBG_LED1);
	//LED PORTD
    gpio_mode_set(LED_PORT1, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
	F_LED0 | F_LED1 | F_LED2 | F_LED3 | F_LED4 | F_LED5 | F_LED6 | F_LED7 | F_LED8 | F_LED9);
    gpio_output_options_set(LED_PORT1, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, \
	F_LED0 | F_LED1 | F_LED2 | F_LED3 | F_LED4 | F_LED5 | F_LED6 | F_LED7 | F_LED8 | F_LED9);
	
	//上电和复位管脚
	pwrRstInit();
	
	//LED 初始状态
	dbgLedCtl(LED_D0, LED_ON);
	dbgLedCtl(LED_D1, LED_ON);
	fLedCtl(LED_F0, LED_GREEN);//灯L5
	fLedCtl(LED_F1, LED_GREEN);//灯L6
	fLedCtl(LED_F2, LED_GREEN);//灯L3
	fLedCtl(LED_F3, LED_GREEN);//灯L4 初始状态为绿
	fLedCtl(LED_F4, LED_GREEN);//灯L2 上电为绿
}
	//F0 灯L5     F1 灯L6     F2 灯L3     F3 灯L4
void LEDGreen_SS(uint8_t ledx,uint8_t i)
{


	if(i%2==1)
	{
		fLedCtl(ledx, LED_GREEN);
	}
	//delay_1ms(150);
	else
	{
		fLedCtl(ledx, LED_OFF);

	}

}
/*!
    \brief      UART初始化
    \param  	void
    \retval     void
*/
void uartInit(void)
{
	serialInit(USART0);
	serialInit(USART2);
}
