#include "gd32f4xx.h"
#include "gn1406.h"
#include "stdio.h"

//定义GN1406在哪个I2C总线上
#define GN1406_I2C GPIO_I2C1
/*!
    \brief 芯片列表定义
*/
uint8_t slaveAddr[] = {
    GN1406_ADDR_D1,   //GN1406_CHIP0_D1 第1个芯片 位号D1
    GN1406_ADDR_D3,   //GN1406_CHIP1_D3 第2个芯片 位号D3
    GN1406_ADDR_D4,   //GN1406_CHIP2_D4 第3个芯片 位号D4
    GN1406_ADDR_D5    //GN1406_CHIP3_D5 第4个芯片 位号D5
};

//参考gn1406的芯片手册来了解寄存器的地址和功能
/*!
    \brief      gn1406 使能Bypass
    \param      void
    \retval     ERR_STA
*/
/* static void gn1406Config_byPass(void)
{
    uint8_t i;
    uint16_t val;
    
    //对于每个芯片，读取特定寄存器的值，然后将该值与0xFFFD进行AND操作，实际上清除了该寄存器的第7位，该操作用于使能bypass
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
} */

/*!
    \brief      gn1406 关闭Bypass
    \param      void
    \retval     ERR_STA
*/
static void gn1406Config_closeByPass(void)
{
    uint8_t i;
    uint16_t val;
    
    //将读取到的寄存器的值与0x0002进行or操作，设置寄存器中的第9位，关闭bypass
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
#if 0
/*!
    \brief      gn1406  速率配置
        \param      uint8_t speed: CFG_6_25G/CFG_5G/CFG_3_125G/CFG_2_5G
    \retval     ERR_STA
*/
//修改寄存器值来实现不同的速率配置
static void gn1406Config(uint8_t speed)
{
    uint8_t i;
    uint16_t val;
    
    gn1406Config_closeByPass();//关闭bypass
    
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
#else
/*!
    \brief      gn1406  速率配置
        \param      uint8_t speed: CFG_6_25G/CFG_5G/CFG_3_125G/CFG_2_5G
    \retval     ERR_STA
*/
//向指定的芯片写入一个寄存器值，并且通过读回该寄存器的值来验证是否写入成功，如果写入失败或者读回的值与写入的值不一致，则会重试直到成功或超时
/* ERR_STA 错误状态枚举；chipx 芯片编号；regAddr 寄存器地址；val 写入的值 
 函数目的：确保寄存器配置正确*/
static ERR_STA gn1406ConfigReg(uint8_t chipx, uint8_t regAddr, uint16_t val)
{
    uint16_t valRead;//存储从寄存器读到的值
		uint32_t timeOut;//跟踪尝试次数，防止无限循环
	
		timeOut = 0;
		do{
			gn1406Write(chipx, regAddr, val); 
			valRead = 0;
			if(STA_ERR == gn1406Read(chipx, regAddr, &valRead) )
			{
				printf("gn1406Read fail");
				valRead = 0;
			}
			if(val != valRead)
			{
				timeOut++;
				if(timeOut > 1000)
				{
					return STA_ERR;
				}
			}
		}while(val != valRead);
		
		return STA_OK;
}

/*!
    \brief      gn1406  速率配置
        \param      uint8_t speed: CFG_6_25G/CFG_5G/CFG_3_125G/CFG_2_5G
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
        val = 0x1818;
        if(CFG_6_25G == speed)
        {
            val = 0x1818|0x0001; //bit0  0:5G 1:6.25G
        }else if(CFG_5G == speed)
        {
            val = 0x1818&0xFFFE; //bit0  0:5G 1:6.25G
        }
				if(STA_ERR == gn1406ConfigReg(i, 0x58, val) )
				{
					printf("gn1406ConfigReg fail:%d 0x%x 0x%x\r\n", i, 0x58, val);
				}
        //时钟速率分频选择
        val = 0xcc41;
        if( (CFG_6_25G == speed) || (CFG_3_125G == speed) )
        {
            val = 0xcc41|0x0001; //bit0  0:5/2.5G  1:6.25/3.125G
        }else if( (CFG_5G == speed) || (CFG_2_5G == speed) )
        {
            val = 0xcc41&0xFFFE; //bit0  0:5/2.5G  1:6.25/3.125G
        }
				if(STA_ERR == gn1406ConfigReg(i, 0x59, val) )
				{
					printf("gn1406ConfigReg fail:%d 0x%x 0x%x\r\n", i, 0x59, val);
				}
        /*****************DDR和SDR模式选择******************/
        //ch0_reg0
        val = 0x000a;
        if( (CFG_6_25G == speed) || (CFG_5G == speed) )
        {
            val = 0x000a|0x0080; //bit0  0:5/2.5G  1:6.25/3.125G
        }else if( (CFG_3_125G == speed) || (CFG_2_5G == speed) )
        {
            val = 0x000a&0xFFF7; //bit0  0:5/2.5G  1:6.25/3.125G
        }
				if(STA_ERR == gn1406ConfigReg(i, 0x40, val) )
				{
					printf("gn1406ConfigReg fail:%d 0x%x 0x%x\r\n", i, 0x40, val);
				}
        //ch1_reg0
        val = 0x000a;
        if( (CFG_6_25G == speed) || (CFG_5G == speed) )
        {
            val = 0x000a|0x0080; //bit0  0:5/2.5G  1:6.25/3.125G
        }else if( (CFG_3_125G == speed) || (CFG_2_5G == speed) )
        {
            val = 0x000a&0xFFF7; //bit0  0:5/2.5G  1:6.25/3.125G
        }
				if(STA_ERR == gn1406ConfigReg(i, 0x46, val) )
				{
					printf("gn1406ConfigReg fail:%d 0x%x 0x%x\r\n", i, 0x46, val);
				}
        //ch2_reg0
        val = 0x000a;
        if( (CFG_6_25G == speed) || (CFG_5G == speed) )
        {
            val = 0x000a|0x0080; //bit0  0:5/2.5G  1:6.25/3.125G
        }else if( (CFG_3_125G == speed) || (CFG_2_5G == speed) )
        {
            val = 0x000a&0xFFF7; //bit0  0:5/2.5G  1:6.25/3.125G
        }
				if(STA_ERR == gn1406ConfigReg(i, 0x4C, val) )
				{
					printf("gn1406ConfigReg fail:%d 0x%x 0x%x\r\n", i, 0x4C, val);
				}
        //ch3_reg0
        val = 0x000a;
        if( (CFG_6_25G == speed) || (CFG_5G == speed) )
        {
            val = 0x000a|0x0080; //bit0  0:5/2.5G  1:6.25/3.125G
        }else if( (CFG_3_125G == speed) || (CFG_2_5G == speed) )
        {
            val = 0x000a&0xFFF7; //bit0  0:5/2.5G  1:6.25/3.125G
        }
				if(STA_ERR == gn1406ConfigReg(i, 0x52, val) )
				{
					printf("gn1406ConfigReg fail:%d 0x%x 0x%x\r\n", i, 0x52, val);
				}
    }
}
#endif

/*!
    \brief      gn1406  5G配置 最开始测试能用的版本
    \param      void
    \retval     ERR_STA
*/
/* static void gn1406Config_5G(void)
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
} */

/*!
    \brief      初始化gn1406芯片
    \param      void
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
    \param      uint8_t chipx:哪个芯片，取值GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
                uint8_t regAddr:寄存器地址
                uint16_t val:写到的寄存器值
    \retval     ERR_STA
*/
//
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
    \param      uint8_t chipx:哪个芯片，取值GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
                uint8_t regAddr:寄存器地址
                uint16_t *val:读到的寄存器值
    \retval     ERR_STA
*/
ERR_STA gn1406Read(uint8_t chipx, uint8_t regAddr, uint16_t *val)
{
    uint8_t buf[2];
    uint16_t tmp;
    
    if (val == 0)
    {
        return STA_ERR;
    }

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

