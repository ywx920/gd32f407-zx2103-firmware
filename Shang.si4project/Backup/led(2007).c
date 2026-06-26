#include "gd32f4xx.h"
#include "led.h"

//定义端口和管脚的对应关系
typedef struct {
	uint32_t portA;
	uint32_t pinA;
	uint32_t portB;
	uint32_t pinB;
} LED_PIX_TAB_TYP;
typedef struct {
	uint32_t port;
	uint32_t pin;
} LED_DBG_PIX_TAB_TYP;

const static LED_PIX_TAB_TYP ledTab[F_LED_NUM] = {
	{LED_PORT1, F_LED0, LED_PORT1, F_LED1},
	{LED_PORT1, F_LED2, LED_PORT1, F_LED3},
	{LED_PORT1, F_LED4, LED_PORT1, F_LED5},
	{LED_PORT1, F_LED6, LED_PORT1, F_LED7},
	{LED_PORT1, F_LED8, LED_PORT1, F_LED9},
};
const static LED_DBG_PIX_TAB_TYP dbgLedTab[D_LED_NUM] = {
	{LED_PORT0, DBG_LED0},
	{LED_PORT0, DBG_LED1},
};

/*!
    \brief      LED的初始化状态
    \param  	void
    \retval     void
*/
void ledInitSta(void)
{
	//调试LED
	GPIO_BOP(LED_PORT0) = DBG_LED0;
	GPIO_BOP(LED_PORT0) = DBG_LED1;
	//前面板
	GPIO_BOP(LED_PORT0) = F_LED0;
	GPIO_BOP(LED_PORT0) = F_LED1;
	GPIO_BOP(LED_PORT0) = F_LED2;
	GPIO_BOP(LED_PORT0) = F_LED3;
	GPIO_BOP(LED_PORT0) = F_LED4;
	//前面板
	GPIO_BOP(LED_PORT1) = F_LED5;
	GPIO_BOP(LED_PORT1) = F_LED6;
	GPIO_BOP(LED_PORT1) = F_LED7;
	GPIO_BOP(LED_PORT1) = F_LED8;
	GPIO_BOP(LED_PORT1) = F_LED9;
}
/*!
    \brief      控制调试LED
    \param  	uint8_t ledx:LED_D0/LED_D1
				uint8_t sta:LED_OFF/LED_ON/LED_TOG
    \retval     void
*/
void dbgLedCtl(uint8_t ledx, uint8_t sta)
{
	uint8_t dLedx;
	if(ledx+1-F_LED_NUM < D_LED_NUM)
	{
		dLedx = ledx+1-F_LED_NUM;
		if(LED_TOG == sta)
		{
			GPIO_TG(dbgLedTab[dLedx].port) = dbgLedTab[dLedx].pin;//翻转
		}else if(LED_OFF == sta)
		{
			GPIO_BOP(dbgLedTab[dLedx].port) = dbgLedTab[dLedx].pin;//电平置1
		}else
		{
			GPIO_BC(dbgLedTab[dLedx].port) = dbgLedTab[dLedx].pin;//电平清零
		}
	}
}
/*!
    \brief      控制LED打开
    \param  	uint8_t ledx:LED_F0/LED_F1/LED_F2/LED_F3/LED_F4
				uint8_t sta:LED_OFF/LED_GREEN/LED_RED
    \retval     void
*/
void fLedCtl(uint8_t ledx, uint8_t sta)
{
	if(ledx < F_LED_NUM)
	{
		switch(sta)
		{
			case LED_OFF://关闭
			GPIO_BOP(ledTab[ledx].portA) = ledTab[ledx].pinA;//电平置1
			GPIO_BOP(ledTab[ledx].portB) = ledTab[ledx].pinB;//电平置1
			break;
			case LED_GREEN://绿灯
			GPIO_BOP(ledTab[ledx].portA) = ledTab[ledx].pinA;//电平置1
			GPIO_BC(ledTab[ledx].portB) = ledTab[ledx].pinB;//电平清零
			break;
			case LED_RED://红灯
			GPIO_BC(ledTab[ledx].portA) = ledTab[ledx].pinA;//电平清零
			GPIO_BOP(ledTab[ledx].portB) = ledTab[ledx].pinB;//电平置1
			break;
			default:
			return;
			
		}
	}
}

