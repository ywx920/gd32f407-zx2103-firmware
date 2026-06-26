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
    rcu_periph_clock_enable(RCU_GPIOA);     //USART 离散IO要用
    rcu_periph_clock_enable(RCU_GPIOB);     //IC SPI
    rcu_periph_clock_enable(RCU_GPIOC);     //USART
    rcu_periph_clock_enable(RCU_GPIOD);     //LED 离散IO
    rcu_periph_clock_enable(RCU_GPIOE);     //离散IO要用
    
    rcu_periph_clock_enable(RCU_USART0);    //USART0控制器
    rcu_periph_clock_enable(RCU_USART2);    //USART2控制器
    
    rcu_periph_clock_enable(RCU_I2C0);      //I2C0控制器
    rcu_periph_clock_enable(RCU_I2C1);      //I2C1控制器
}

/*!
    \brief      板卡的上电和复位序列
    \param      void
    \retval     void
*/
/* static void pwrRstInit(void)
{
    //4片GN1406复位控制管脚;IndexZ@2021-9-26
    gpio_mode_set(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
    GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, \
    GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    //拉高，结束复位，进入工作状态;IndexZ@2021-9-26
    GPIO_BOP(GPIOD) = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    //拉低
    //GPIO_BOP(GPIOD) = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
} */

/*!
    \brief      板卡的所有GPIO初始化
    \param      void
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
    
    //GN1406上电和复位管脚
    //pwrRstInit();
    
//    //LED 初始状态
    dbgLedCtl(LED_D0, LED_ON);
    dbgLedCtl(LED_D1, LED_ON);
    fLedCtl(LED_FL5, LED_GREEN);    //灯L5
    fLedCtl(LED_FL6, LED_GREEN);    //灯L6
    fLedCtl(LED_FL3, LED_GREEN);    //灯L3
    fLedCtl(LED_FL4, LED_GREEN);    //灯L4 初始状态为绿
    fLedCtl(LED_FL2, LED_GREEN);    //灯L2 上电为绿,MCU异常为红色;
//LQ测试 LED 20240726	
/****************8
		dbgLedCtl(LED_D0, LED_OFF);
		dbgLedCtl(LED_D0, LED_ON);
		dbgLedCtl(LED_D0, LED_OFF);
		dbgLedCtl(LED_D1, LED_OFF);
		dbgLedCtl(LED_D1, LED_ON);
		dbgLedCtl(LED_D1, LED_OFF);
		//单向  D2-1 down
		fLedCtl(LED_FL5, LED_GREEN);    //灯L5
		fLedCtl(LED_FL5, LED_RED);    //灯L5
		fLedCtl(LED_FL5, LED_OFF);    //灯L5
		//双向  D2-2 up
		fLedCtl(LED_FL6, LED_GREEN);    //灯L6
		fLedCtl(LED_FL6, LED_RED);    //灯L6
		fLedCtl(LED_FL6, LED_OFF);    //灯L6
		//单向	D5-1 down
    fLedCtl(LED_FL3, LED_GREEN);    //灯L3
		fLedCtl(LED_FL3, LED_RED);    //灯L3
		fLedCtl(LED_FL3, LED_OFF);    //灯L3
		//双向	D5-2 up
    fLedCtl(LED_FL4, LED_GREEN);    //灯L4 初始状态为绿
		fLedCtl(LED_FL4, LED_RED);    //灯L4 初始状态为绿
		fLedCtl(LED_FL4, LED_OFF);    //灯L4 初始状态为绿		
		
		//双向	D1-2 up
    fLedCtl(LED_FL2, LED_GREEN);    //灯L2 上电为绿,MCU异常为红色;
		fLedCtl(LED_FL2, LED_RED);    //灯L2 上电为绿,MCU异常为红色;
		fLedCtl(LED_FL2, LED_OFF);    //灯L2 上电为绿,MCU异常为红色;
		***************/
}

void LEDGreen_SS(FLED_NUM ledx, uint8_t i)
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
    \param      void
    \retval     void
*/
void uartInit(void)
{
    serialInit(USART0);
    //serialInit(USART5);
    serialInit(USART2);
}

