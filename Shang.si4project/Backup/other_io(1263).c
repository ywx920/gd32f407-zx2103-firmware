#include "gd32f4xx.h"
#include "other_io.h"

/*!
    \brief      LOS信号 IO初始化
    \param  	void
    \retval     void
*/
static void los_io_init(void)
{
	//复位控制管脚
  gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, \
	GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
	GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	
  gpio_output_options_set(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, \
	GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
	GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
}

/*!
    \brief      GA信号 IO初始化
    \param  	void
    \retval     void
*/
static void GA_io_init(void)
{
	//复位控制管脚
  gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_NONE, \
	GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | \
	GPIO_PIN_9 | GPIO_PIN_10);
	
  gpio_output_options_set(GPIOE, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, \
	GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | \
	GPIO_PIN_9 | GPIO_PIN_10);
}

/*!
    \brief      RA_N0~3信号 IO初始化
    \param  	void
    \retval     void
*/
static void RA_io_init(void)
{
	//复位控制管脚
  gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_NONE, \
	GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	
  gpio_output_options_set(GPIOE, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, \
	GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
}

/*!
    \brief      NVMRO信号 IO初始化
    \param  	void
    \retval     void
*/
static void NVMRO_io_init(void)
{
	//复位控制管脚
  gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_4);
	
  gpio_output_options_set(GPIOE, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
}

/*!
    \brief      其他离散IO初始化
    \param  	void
    \retval     void
*/
void other_io_init(void)
{
	los_io_init();
	GA_io_init();
	RA_io_init();
	NVMRO_io_init();
}

/*!
    \brief      读取8个LOS信号的状态值
    \param  	void
    \retval     uint8_t 返回8个loss信号的值 
				bit0对应SFP0_LOS bit7对应SFP7_LOS
*/
uint8_t read_los(void)
{
	uint32_t regVal;
	uint8_t val = 0;
	
	regVal = GPIO_ISTAT(GPIOA) & (\
	GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
	GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7\
	);
	val = (uint8_t)(regVal >> 0);
	
	return val;
}

/*!
    \brief      读取6个VPX_GA0~4 GAP信号的状态值
    \param  	void
    \retval     uint8_t 返回6个VPX_GA0~4 GAP信号的值
				bit0对应VPX_GA0 bit4对应VPX_GA4 bit4对应VPX_GAP
*/
uint8_t read_GA(void)
{
	uint32_t regVal;
	uint8_t val = 0;
	
	regVal = GPIO_ISTAT(GPIOE) & (\
	GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | \
	GPIO_PIN_9 | GPIO_PIN_10\
	);
	val = (uint8_t)(regVal >> 5);
	
	return val;
}

/*!
    \brief      读取4个RA_N0~1信号的状态值
    \param  	void
    \retval     uint8_t 返回4个RA_N0~3信号的值
				bit0对应RA_N0 bit3对应RA_N3
*/
uint8_t read_RA(void)
{
	uint32_t regVal;
	uint8_t val = 0;
	
	regVal = GPIO_ISTAT(GPIOE) & (\
	GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15\
	);
	val = (uint8_t)(regVal >> 12);
	
	return val;
}

/*!
    \brief      读取NVMRO信号的状态值
    \param  	void
    \retval     uint8_t 返回NVMRO信号的值
				bit0对应NVMRO
*/
uint8_t read_NVMRO(void)
{
	uint32_t regVal;
	uint8_t val = 0;
	
	regVal = GPIO_ISTAT(GPIOE) & GPIO_PIN_4;
	val = (uint8_t)(regVal >> 4);
	
	return val;
}

