#include "gd32f4xx.h"
#include "gpio_i2c.h"

//定义端口和管脚的对应关系
typedef struct {
	uint32_t clkPort;
	uint32_t clkPin;
	uint32_t sdaPort;
	uint32_t sdaPin;
} GPIO_I2C_TAB_TYP;

const static GPIO_I2C_TAB_TYP gpioI2cTab[GPIO_I2C_NUM] = {
	{GPIOB, GPIO_PIN_6, GPIOB, GPIO_PIN_7},
	{GPIOB, GPIO_PIN_10, GPIOB, GPIO_PIN_11},
};

#define I2C_DELAY_TIME (16*10) //4的倍数
/*!
    \brief      延时函数
    \param  	uint32_t cnt:延时时间
    \retval     void
*/
void delay(uint32_t cnt)
{
	uint32_t i, j;
	
	for(i = 0; i < cnt; i++)
	{
		for(j = 0; j < 40; j++);
	}
}

/*!
    \brief      GPIO模拟I2C 启动信号
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
    \retval     void
*/
static void gpioI2cStart(uint8_t gpioI2cx)
{
	//数据线设置为输出之前拉高数据线，避免如果之前是低，置位输出后，拉低数据线
	//GPIO_BOP(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平置1
	//数据线设置为输出
	gpio_mode_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
	gpioI2cTab[gpioI2cx].sdaPin);
	
	GPIO_BOP(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平置1
	GPIO_BOP(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平置1
	
	delay(I2C_DELAY_TIME/4);
	GPIO_BC(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平清0
	delay(I2C_DELAY_TIME/4);
	GPIO_BC(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平清0
}

/*!
    \brief      GPIO模拟I2C移位数据到数据线
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
    \retval     void
*/
static void gpioI2cTxDat(uint8_t gpioI2cx, uint8_t dat)
{
	uint8_t i;
	
	//数据线设置为输出之前拉高数据线，避免如果之前是低，置位输出后，拉低数据线
	//GPIO_BOP(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平置1
	//数据线设置为输出
	gpio_mode_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
		gpioI2cTab[gpioI2cx].sdaPin);
	
	for(i = 0; i < 8; i++)
	{
		delay(I2C_DELAY_TIME/4);
		//准备数据
		if(0x01&(dat>>(7-i)))
		{
			GPIO_BOP(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平置1
		}else
		{
			GPIO_BC(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平清0
		}
		delay(I2C_DELAY_TIME/4);
		//拉高时钟
		GPIO_BOP(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平置1
		delay(I2C_DELAY_TIME/2);//等待从机采样
		GPIO_BC(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平清0
	}
}

/*!
    \brief      GPIO模拟I2C移位数据到数据线
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
    \retval     uint8_t返回读到的数据
*/
static uint8_t gpioI2cRxDat(uint8_t gpioI2cx)
{
	uint32_t regVal;
	uint8_t i, tmpVal;
	uint8_t val = 0;
	
	//数据线设置为输入
	gpio_mode_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, \
		gpioI2cTab[gpioI2cx].sdaPin);
	
	for(i = 0; i < 8; i++)
	{
		//等待从机数据准备完成
		delay(I2C_DELAY_TIME/2);
		
		//拉高时钟
		GPIO_BOP(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平置1
		
		//等待SDA稳定
		delay(I2C_DELAY_TIME/2);
		//采样
		regVal = GPIO_ISTAT(gpioI2cTab[gpioI2cx].sdaPort) & gpioI2cTab[gpioI2cx].sdaPin;
		tmpVal = (0 == regVal) ? 0 : 1;
		val |= tmpVal<<(7-i);
		
		GPIO_BC(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平清0
	}
	
	return val;
}

/*!
    \brief      发送ACK信号
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
    \retval     void
*/
static void gpioI2cAck(uint8_t gpioI2cx)
{
	
	//数据线设置为输出
	gpio_mode_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
		gpioI2cTab[gpioI2cx].sdaPin);
	//拉低数据线
	GPIO_BC(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平清0
	delay(I2C_DELAY_TIME/2);//等待
	
	//拉高时钟
	GPIO_BOP(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平置1
	delay(I2C_DELAY_TIME/2);//等待从机采样ACK信号
	
	GPIO_BC(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平清0
	//数据线设置为输入
	gpio_mode_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, \
		gpioI2cTab[gpioI2cx].sdaPin);

}

/*!
    \brief      GPIO模拟I2C移位数据到数据线
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
    \retval     uint8_t返回ACK的值
*/
static uint8_t gpioI2cWaitAck(uint8_t gpioI2cx)
{
	uint32_t regVal;
	uint8_t ackVal = 1;
	
	//数据线设置为输入
	gpio_mode_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, \
		gpioI2cTab[gpioI2cx].sdaPin);
	delay(I2C_DELAY_TIME/2);//等待从机输出ACK
	
	//拉高时钟
	GPIO_BOP(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平置1
	delay(I2C_DELAY_TIME/4);//等待SDA稳定
	//采样
	regVal = GPIO_ISTAT(gpioI2cTab[gpioI2cx].sdaPort) & gpioI2cTab[gpioI2cx].sdaPin;
	ackVal = (0 == regVal) ? 0 : 1;
	
	GPIO_BC(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平清0	
	
	return ackVal;
}

/*!
    \brief      GPIO模拟I2C 停止信号
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
    \retval     void
*/
static void gpioI2cStop(uint8_t gpioI2cx)
{
	//数据线设置为输出之前拉高数据线，避免如果之前是低，置位输出后，拉低数据线
	//GPIO_BOP(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平置1
	//数据线设置为输出
	gpio_mode_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
	gpioI2cTab[gpioI2cx].sdaPin);

	//确保数据时钟为低电平状态
	GPIO_BC(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平清0
	delay(I2C_DELAY_TIME/4);
	GPIO_BC(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平清0	
	
	//发送停止信号
	GPIO_BOP(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平置1
	delay(I2C_DELAY_TIME/4);
	GPIO_BOP(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平置1
}

/*!
    \brief      GPIO模拟I2C初始化
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
    \retval     void
*/
void gpioI2cInit(uint8_t gpioI2cx)
{
	if(gpioI2cx < GPIO_I2C_NUM)
	{
		//clk
		gpio_mode_set(gpioI2cTab[gpioI2cx].clkPort, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, \
		gpioI2cTab[gpioI2cx].clkPin);
		gpio_output_options_set(gpioI2cTab[gpioI2cx].clkPort, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, \
		gpioI2cTab[gpioI2cx].clkPin);
		//dat
		gpio_mode_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_MODE_INPUT, GPIO_PUPD_NONE, \
		gpioI2cTab[gpioI2cx].sdaPin);
		gpio_output_options_set(gpioI2cTab[gpioI2cx].sdaPort, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, \
		gpioI2cTab[gpioI2cx].sdaPin);
		
		GPIO_BOP(gpioI2cTab[gpioI2cx].clkPort) = gpioI2cTab[gpioI2cx].clkPin;//电平置1
		GPIO_BOP(gpioI2cTab[gpioI2cx].sdaPort) = gpioI2cTab[gpioI2cx].sdaPin;//电平置1
	}
}
/*!
    \brief      GPIO模拟I2C读操作
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
				uint8_t slaveAddr:从机地址
				uint8_t reg:寄存器地址
				uint8_t *buf:读取的数据
				uint8_t size:读取多少字节
    \retval     ERR_STA:STA_OK/STA_ERR
*/
ERR_STA gpioI2cRead(uint8_t gpioI2cx, uint8_t slaveAddr, uint8_t reg, uint8_t *buf, uint8_t size)
{
	ERR_STA sta = STA_OK;
	uint8_t slaveAddr_rw;
	uint8_t i;
	
	slaveAddr_rw = 0xFE & (slaveAddr<<1);//bit0 = 0表示写
	//启动信号
	gpioI2cStart(gpioI2cx);
	//从机地址和写操作位
	gpioI2cTxDat(gpioI2cx, slaveAddr_rw);
	//判断ACK
	if(1 == gpioI2cWaitAck(gpioI2cx))
	{
		sta = STA_ERR;
	}	
	//发送寄存器地址
	gpioI2cTxDat(gpioI2cx, reg);	
	//判断ACK
	if(1 == gpioI2cWaitAck(gpioI2cx))
	{
		sta = STA_ERR;
	}
	//启动信号
	gpioI2cStart(gpioI2cx);	
	//从机地址和读操作位
	slaveAddr_rw |= 0x01;//bit0 = 1表示读
	gpioI2cTxDat(gpioI2cx, slaveAddr_rw);	
	//判断ACK
	if(1 == gpioI2cWaitAck(gpioI2cx))
	{
		sta = STA_ERR;
	}
	
	delay(I2C_DELAY_TIME*10);//CLK拉宽
	//读数据
	for(i = 0; i < size; i++)
	{
		buf[i] = gpioI2cRxDat(gpioI2cx);
		//判断ACK
		if(i+1 != size)
		{//只要不是最后一个，就要回ACK信号
			gpioI2cAck(gpioI2cx);
		}else
		{//最后一个ACK
			//通过等待ACK的函数产生ACK时钟信号，但是不判断总线上的ACK值
			gpioI2cWaitAck(gpioI2cx); //Nack不用回应，靠总线上拉电阻置高
		}
	}
	//停止
	gpioI2cStop(gpioI2cx);
	
	return sta;
}
/*!
    \brief      GPIO模拟I2C读操作
    \param  	uint8_t gpioI2cx:第几路 GPIO_I2C0/GPIO_I2C1
				uint8_t slaveAddr:从机地址
				uint8_t reg:寄存器地址
				uint8_t *buf:写入的数据
				uint8_t size:写入多少字节
    \retval     ERR_STA:STA_OK/STA_ERR
*/
ERR_STA gpioI2cWrite(uint8_t gpioI2cx, uint8_t slaveAddr, uint8_t reg, uint8_t *buf, uint8_t size)
{
	ERR_STA sta = STA_OK;
	uint8_t slaveAddr_rw;
	uint8_t i;
	
	slaveAddr_rw = 0xFE & (slaveAddr<<1);//bit0 = 0表示写
	//启动信号
	gpioI2cStart(gpioI2cx);
	//从机地址和写操作位
	gpioI2cTxDat(gpioI2cx, slaveAddr_rw);	
	//判断ACK
	if(1 == gpioI2cWaitAck(gpioI2cx))
	{
		sta = STA_ERR;
	}
	//发送寄存器地址
	gpioI2cTxDat(gpioI2cx, reg);	
	//判断ACK
	if(1 == gpioI2cWaitAck(gpioI2cx))
	{
		sta = STA_ERR;
	}	
	//写数据
	for(i = 0; i < size; i++)
	{
		gpioI2cTxDat(gpioI2cx, buf[i]);	
		//判断ACK
		if(1 == gpioI2cWaitAck(gpioI2cx))
		{
				sta = STA_ERR; //每一个都需要判断ACK
		}
	}		
	//停止
	gpioI2cStop(gpioI2cx);
	
	return sta;	
}

