#include "gd32f4xx.h"
#include "gn1406.h"

//定义GN1406在哪个I2C总线上
#define GN1406_I2C GPIO_I2C1
/*!
    \brief 芯片列表定义
*/
uint8_t slaveAddr[] = {
	GN1406_ADDR0,//GN1406_CHIP0_D1 第1个芯片 位号D1
	GN1406_ADDR1,//GN1406_CHIP1_D3 第2个芯片 位号D3
	GN1406_ADDR2,//GN1406_CHIP2_D4 第3个芯片 位号D4
	GN1406_ADDR3,//GN1406_CHIP3_D5 第4个芯片 位号D5
};

/*!
    \brief      gn1406 使能Bypass
    \param  	void
    \retval     ERR_STA
*/
static void gn1406Config_byPass(void)
{
	uint8_t i;
	uint16_t val;
	
	for(i = 0; i < GN1406_CHIP_NUM; i++)
	{//GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
		//ch0_reg0
		gn1406Read(i, 0x40, &val);
		gn1406Write(i, 0x40, val&0xFFFD);
		//ch1_reg0
		gn1406Read(i, 0x46, &val);
		gn1406Write(i, 0x46, val&0xFFFD);
		//ch2_reg0
		gn1406Read(i, 0x4C, &val);
		gn1406Write(i, 0x4C, val&0xFFFD);
		//ch3_reg0
		gn1406Read(i, 0x52, &val);
		gn1406Write(i, 0x52, val&0xFFFD);
	}
}

/*!
    \brief      gn1406 关闭Bypass
    \param  	void
    \retval     ERR_STA
*/
static void gn1406Config_closeByPass(void)
{
	uint8_t i;
	uint16_t val;
	
	for(i = 0; i < GN1406_CHIP_NUM; i++)
	{//GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
		//ch0_reg0
		gn1406Read(i, 0x40, &val);
		gn1406Write(i, 0x40, val|0x0002);
		//ch1_reg0
		gn1406Read(i, 0x46, &val);
		gn1406Write(i, 0x46, val|0x0002);
		//ch2_reg0
		gn1406Read(i, 0x4C, &val);
		gn1406Write(i, 0x4C, val|0x0002);
		//ch3_reg0
		gn1406Read(i, 0x52, &val);
		gn1406Write(i, 0x52, val|0x0002);
	}
}

/*!
    \brief      gn1406  速率配置
		\param  	uint8_t speed: CFG_6_25G/CFG_5G/CFG_3_125G/CFG_2_5G
    \retval     ERR_STA
*/
static void gn1406Config(uint8_t speed)
{
	uint8_t i;
	uint16_t val;
	
	gn1406Config_closeByPass();
	
	for(i = 0; i < GN1406_CHIP_NUM; i++)
	{//GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
		//DDR config:5/6.25G
		gn1406Read(i, 0x58, &val);
		if(CFG_6_25G == speed)
		{
			gn1406Write(i, 0x58, val|0x0001); //bit0  0:5G 1:6.25G
		}else if(CFG_5G == speed)
		{
			gn1406Write(i, 0x58, val&0xFFFE); //bit0  0:5G 1:6.25G
		}
		//时钟速率分频选择
		gn1406Read(i, 0x59, &val);
		if( (CFG_6_25G == speed) || (CFG_3_125G == speed) )
		{
			gn1406Write(i, 0x59, val|0x0001); //bit0  0:5/2.5G  1:6.25/3.125G
		}else if( (CFG_5G == speed) || (CFG_2_5G == speed) )
		{
			gn1406Write(i, 0x59, val&0xFFFE); //bit0  0:5/2.5G  1:6.25/3.125G
		}
		/*****************DDR和SDR模式选择******************/
		//ch0_reg0
		gn1406Read(i, 0x40, &val);
		if( (CFG_6_25G == speed) || (CFG_5G == speed) )
		{
			gn1406Write(i, 0x40, val|0x0080); //bit0  0:5/2.5G  1:6.25/3.125G
		}else if( (CFG_3_125G == speed) || (CFG_2_5G == speed) )
		{
			gn1406Write(i, 0x40, val&0xFFF7); //bit0  0:5/2.5G  1:6.25/3.125G
		}
		//ch1_reg0
		gn1406Read(i, 0x46, &val);
		if( (CFG_6_25G == speed) || (CFG_5G == speed) )
		{
			gn1406Write(i, 0x46, val|0x0080); //bit0  0:5/2.5G  1:6.25/3.125G
		}else if( (CFG_3_125G == speed) || (CFG_2_5G == speed) )
		{
			gn1406Write(i, 0x46, val&0xFFF7); //bit0  0:5/2.5G  1:6.25/3.125G
		}
		//ch2_reg0
		gn1406Read(i, 0x4C, &val);
		if( (CFG_6_25G == speed) || (CFG_5G == speed) )
		{
			gn1406Write(i, 0x4C, val|0x0080); //bit0  0:5/2.5G  1:6.25/3.125G
		}else if( (CFG_3_125G == speed) || (CFG_2_5G == speed) )
		{
			gn1406Write(i, 0x4C, val&0xFFF7); //bit0  0:5/2.5G  1:6.25/3.125G
		}
		//ch3_reg0
		gn1406Read(i, 0x52, &val);
		if( (CFG_6_25G == speed) || (CFG_5G == speed) )
		{
			gn1406Write(i, 0x52, val|0x0080); //bit0  0:5/2.5G  1:6.25/3.125G
		}else if( (CFG_3_125G == speed) || (CFG_2_5G == speed) )
		{
			gn1406Write(i, 0x52, val&0xFFF7); //bit0  0:5/2.5G  1:6.25/3.125G
		}
	}
}

/*!
    \brief      gn1406  5G配置 最开始测试能用的版本
    \param  	void
    \retval     ERR_STA
*/
static void gn1406Config_5G(void)
{
	uint8_t i;
	uint16_t val;
	
	gn1406Config_closeByPass();
	
	for(i = 0; i < GN1406_CHIP_NUM; i++)
	{//GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
		gn1406Read(i, 0x58, &val);
		gn1406Write(i, 0x58, val&0xFFFE); 
		gn1406Read(i, 0x59, &val);
		gn1406Write(i, 0x59, val&0xFFFE);
#if 0
		//ch0_reg0
		gn1406Read(i, 0x40, &val);
		gn1406Write(i, 0x40, val|0x0080);
		//ch1_reg0
		gn1406Read(i, 0x46, &val);
		gn1406Write(i, 0x46, val|0x0080);
		//ch2_reg0
		gn1406Read(i, 0x4C, &val);
		gn1406Write(i, 0x4C, val|0x0080);
		//ch3_reg0
		gn1406Read(i, 0x52, &val);
		gn1406Write(i, 0x52, val|0x0080);
#endif
	}
}

/*!
    \brief      初始化gn1406芯片
    \param  	void
    \retval     ERR_STA
*/
ERR_STA gn1406Init(void)
{
	gpioI2cInit(GN1406_I2C);
	//gn1406Config_byPass();
	gn1406Config(CFG_5G);
	return STA_OK;
}

/*!
    \brief      读取gn1406的状态
    \param  	uint8_t chipx:哪个芯片，取值GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
				uint8_t regAddr:寄存器地址
				uint16_t val:写到的寄存器值
    \retval     ERR_STA
*/
ERR_STA gn1406Write(uint8_t chipx, uint8_t regAddr, uint16_t val)
{
	uint8_t buf[2];
	
	if(chipx < GN1406_CHIP_NUM)
	{
		buf[0] = 0xFF&(val>>8);
		buf[1] = 0xFF&(val);
		if(STA_ERR == gpioI2cWrite(GN1406_I2C, slaveAddr[chipx], regAddr, buf, 2))
		{
			return STA_ERR;
		}
	}else
	{
		return STA_ERR;
	}
	
	return STA_OK;
}

/*!
    \brief      读取gn1406的状态
    \param  	uint8_t chipx:哪个芯片，取值GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
				uint8_t regAddr:寄存器地址
				uint16_t *val:读到的寄存器值
    \retval     ERR_STA
*/
ERR_STA gn1406Read(uint8_t chipx, uint8_t regAddr, uint16_t *val)
{
	uint8_t buf[2];
	uint16_t tmp;
	
	if(chipx < GN1406_CHIP_NUM)
	{
		if(STA_ERR == gpioI2cRead(GN1406_I2C, slaveAddr[chipx], regAddr, buf, 2))
		{
			return STA_ERR;
		}
	}else
	{
		return STA_ERR;
	}
	
	tmp = buf[0];
	tmp = (tmp << 8) | buf[1];
	*val = tmp;
	
	return STA_OK;
}

