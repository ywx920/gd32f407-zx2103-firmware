#include "gd32f4xx.h"
#include "cb2991.h"
#include <stdio.h>

/*******************硬件定义*******************/
//定义GN1406在哪个I2C总线上
#define CB2991_I2C GPIO_I2C0
#define CB2991_ADDR 0x4F //器件I2C地址

/*
8通道电压采集或者4通道温度采集 一路内部温度采集 一路内部VCC电压采集
电压采集 可以采集单端或者差分电压 温度可以为开尔文温度或者摄氏温度

电压格式 LSB = 
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
DV*  Sign D13  D12  D11  D10  D9   D8
D7   D6   D5   D4   D3   D2   D1   D0                   
DV*为数据有效位 Sign为符号位

温度格式 LSB = 0.0625 Degrees
Bit7 Bit6 Bit5 Bit4 Bit3 Bit2 Bit1 Bit0
DV*  X    X    D12  D11  D10  D9   D8
D7   D6   D5   D4   D3   D2   D1   D0
DV*为数据有效位  X unused
*/
#define STATUS_LOW_REG                      0x00
#define STATUS_HIGH_CHANNEL_ENABLE_REG      0x01
#define V1234_CONTROL_REG                   0x06
#define V5678_CONTROL_REG                   0x07
#define PWM_VCC_TINTERNAL_CONTROL_REG       0x08
#define PWM_MSB_REG                         0x09
#define V1_MSB_DAT_REG                      0x0A
#define V1_LSB_DAT_REG                      0x0B
#define V2_MSB_DAT_REG                      0x0C
#define V2_LSB_DAT_REG                      0x0D
#define V3_MSB_DAT_REG                      0x0E
#define V3_LSB_DAT_REG                      0x0F
#define V4_MSB_DAT_REG                      0x10
#define V4_LSB_DAT_REG                      0x11
#define V5_MSB_DAT_REG                      0x12
#define V5_LSB_DAT_REG                      0x13
#define V6_MSB_DAT_REG                      0x14
#define V6_LSB_DAT_REG                      0x15
#define V7_MSB_DAT_REG                      0x16
#define V7_LSB_DAT_REG                      0x17
#define V8_MSB_DAT_REG                      0x18
#define V8_LSB_DAT_REG                      0x19
#define TINTERNAL_MSB_REG                   0x1A
#define TINTERNAL_LSB_REG                   0x1B
#define VCC_MSB_REG                         0x1C
#define VCC_LSB_REG                         0x1D
#define CB2991_DATA_VALID_MASK              0x80U
#define CB2991_DATA_READY_RETRY             100U

static ERR_STA cb2991ReadValid(uint8_t regAddr, uint8_t *data)
{
    uint16_t retry;

    if (data == 0)
    {
        return STA_ERR;
    }

    for (retry = 0U; retry < CB2991_DATA_READY_RETRY; retry++)
    {
        if (STA_ERR == gpioI2cRead(CB2991_I2C, CB2991_ADDR, regAddr, data, 2U))
        {
            return STA_ERR;
        }
        if (0U != (data[0] & CB2991_DATA_VALID_MASK))
        {
            return STA_OK;
        }
    }

    return STA_ERR;
}

/*!
    \brief      将cb2991初始化为温度采集模式
    \param      void
    \retval     ERR_STA:STA_ERR/STA_OK
*/
ERR_STA cb2991TempInit(void)
{
    ERR_STA sta = STA_OK;
    uint8_t val;
    uint8_t lineState;
    
    gpioI2cInit(CB2991_I2C);
    lineState = gpioI2cGetLineState(CB2991_I2C);
    printf("[CB2991] bus idle: SCL=%u SDA=%u\r\n",
           (lineState >> 1) & 0x01U,
           lineState & 0x01U);
    if (lineState != 0x03U)
    {
        return STA_ERR;
    }
    if (STA_OK != gpioI2cProbe(CB2991_I2C, CB2991_ADDR))
    {
        printf("[CB2991] address NACK: 7-bit=0x%02X write=0x%02X\r\n",
               CB2991_ADDR, (uint8_t)(CB2991_ADDR << 1));
        return STA_ERR;
    }
    printf("[CB2991] address ACK: 7-bit=0x%02X\r\n", CB2991_ADDR);
    //0xF8使能全部通道 使能内部温度传感器
    val = 0xF8;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, STATUS_HIGH_CHANNEL_ENABLE_REG, &val, 1) )
    {
        return STA_ERR;
    }
    //2路温度采集通道 0x22设置为温度采集 摄氏度 不使能Filter
    val = 0x22;
    //if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, V1234_CONTROL_REG, &val, 1))
    //{
    //  return STA_ERR;
    //}
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, V5678_CONTROL_REG, &val, 1))
    {
        return STA_ERR;
    }
    //bit4=1: Repeated Mode  
    val = 0x10;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, PWM_VCC_TINTERNAL_CONTROL_REG, &val, 1))
    {
        return STA_ERR;
    }
    
    return sta;
}

/*!
    \brief      将cb2991初始化为电压采集模式
    \param      void
    \retval     ERR_STA:STA_ERR/STA_OK
*/
ERR_STA cb2991VolInit(void)
{
    uint8_t val;
    gpioI2cInit(CB2991_I2C);
    //0xF8使能全部通道 使能内部温度传感器
    val = 0xF8;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, STATUS_HIGH_CHANNEL_ENABLE_REG, &val, 1) )
    {
        return STA_ERR;
    }
    //8通道 0x00设置为单端电压采集 不使能Filter 1通道设置为差分电压采集
    val = 0x01;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, V1234_CONTROL_REG, &val, 1) )
    {
        return STA_ERR;
    }
    val = 0x00;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, V5678_CONTROL_REG, &val, 1) )
    {
        return STA_ERR;
    }
    //bit4=1: Repeated Mode  
    val = 0x10;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, PWM_VCC_TINTERNAL_CONTROL_REG, &val, 1) )
    {
        return STA_ERR;
    }
    
    return STA_OK;
}

/*!
    \brief      将cb2991初始化为电压采集和温度采集模式
    \param      void
    \retval     ERR_STA:STA_ERR/STA_OK
*/
ERR_STA cb2991CutomInit(void)
{
    ERR_STA sta = STA_OK;
    uint8_t val;
    
    gpioI2cInit(CB2991_I2C);
    //0xF8使能全部通道 使能内部温度传感器
    val = 0xF8;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, STATUS_HIGH_CHANNEL_ENABLE_REG, &val, 1) )
    {
        return STA_ERR;
    }
    
    val = 0x0;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, V1234_CONTROL_REG, &val, 1))
    {
        return STA_ERR;
    }
    val = 0x22;
    //1路温度采集通道 0x22设置为温度采集 摄氏度 不使能Filter
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, V5678_CONTROL_REG, &val, 1))
    {
        return STA_ERR;
    }
    //bit4=1: Repeated Mode  
    val = 0x10;
    if(STA_ERR == gpioI2cWrite(CB2991_I2C, CB2991_ADDR, PWM_VCC_TINTERNAL_CONTROL_REG, &val, 1))
    {
        return STA_ERR;
    }
    
    return sta;
}

/*!
    \brief      获取温度采样值
    \param      char *temp:读取的5个温度采样值
    \retval     ERR_STA:STA_ERR/STA_OK
*/
ERR_STA ReadCB2991Temp(char *temp)
{
    #define TEMP_REG_NUM 5
    uint8_t  pBuffer[2];
    uint8_t  i = 0;
    double temp_data[TEMP_REG_NUM];
    
    uint8_t tmpe_reg_addr[TEMP_REG_NUM] = {
                                        V1_MSB_DAT_REG,
                                        V3_MSB_DAT_REG,
                                        V5_MSB_DAT_REG,
                                        V7_MSB_DAT_REG,
                                        TINTERNAL_MSB_REG};
    if (temp == 0)
    {
        return STA_ERR;
    }

    for(i = 0; i < TEMP_REG_NUM; i++)
    {
        if (STA_OK != cb2991ReadValid(tmpe_reg_addr[i], pBuffer))
        {
            return STA_ERR;
        }
        
        if(pBuffer[0] & 0x08)
        {
            temp_data[i] = -(double)(((((uint16_t)pBuffer[1] & 0xFFU) |
                                      (((uint16_t)pBuffer[0] & 0x07U) << 8)) ^
                                     0x07FFU) + 1U) * 0.0625;
            temp[i] = temp_data[i];
        }
        else
        {
            temp_data[i] = (((uint16_t)pBuffer[1]&0xff)|(((uint16_t)pBuffer[0]&0x07) <<8))*0.0625;
            temp[i] = temp_data[i];
        }
    }
    
    return STA_OK;
}

/*!
    \brief      获取电压采样值
    \param      char *temp:读取的四个温度采样值
    \retval     ERR_STA:STA_ERR/STA_OK
*/
ERR_STA ReadCB2991VolCur(uint8_t *pvol_data,uint8_t *pcurrent_data)
{
    #define DATA_REG_NUM 2
    uint8_t  pBuffer[2];
    uint8_t  i = 0;
    uint16_t vol_data[DATA_REG_NUM];
    double current_data = 0;
    double middle_val = 0;
    uint8_t vol_reg_addr[DATA_REG_NUM] = {
                                        V3_MSB_DAT_REG, //1.8V
//                                      V4_MSB_DAT_REG,
//                                      V5_MSB_DAT_REG,
//                                      V6_MSB_DAT_REG,
//                                      V7_MSB_DAT_REG,
                                        V8_MSB_DAT_REG}; //3.3V
    if ((pvol_data == 0) || (pcurrent_data == 0))
    {
        return STA_ERR;
    }

    /*6路电压*/                                      
    for(i = 0;i<DATA_REG_NUM;i++)
    {
        if (STA_OK != cb2991ReadValid(vol_reg_addr[i], pBuffer))
        {
            return STA_ERR;
        }
        vol_data[i] = ((uint16_t)pBuffer[1]&0xff)|(((uint16_t)pBuffer[0]&0x3f) <<8);//bit0~13有效 bit14符号位 不考虑负电压
    }
    for(i = 0;i<DATA_REG_NUM;i++)
    {
        middle_val = vol_data[i];
        vol_data[i] = (middle_val * 305.18)/1000;//mV
    }
    
    *(pvol_data+0) = (uint8_t)(vol_data[0]/9);      //1800mV/9
    *(pvol_data+1) = (uint8_t)(vol_data[1]/15);     //3300mV/15
    
    /*1路电流*/
    if (STA_OK != cb2991ReadValid(V2_MSB_DAT_REG, pBuffer))
    {
        return STA_ERR;
    }
    middle_val = ((uint16_t)pBuffer[1]&0xff)|(((uint16_t)pBuffer[0]&0x3f) <<8);
    current_data = middle_val * 15.54;//mA 
    *pcurrent_data = current_data;
    
    return STA_OK;
}

/*!
    \brief      获取电压采样值
    \param      uint16_t *vol:读取的四个电压采样值
    \retval     ERR_STA:STA_ERR/STA_OK
*/
ERR_STA ReadCB2991Vol(uint16_t *vol)
{
    #define VOL_REG_NUM 4
    uint8_t  pBuffer[2];
    uint8_t  i = 0;
    uint16_t vol_data[VOL_REG_NUM];
    double middle_val = 0;
    uint8_t vol_reg_addr[VOL_REG_NUM] = {
                                        V1_MSB_DAT_REG, //1.0V
                                        V2_MSB_DAT_REG, //1.0 MGTV
                                        V3_MSB_DAT_REG, //1.8V
                                        V4_MSB_DAT_REG, //3.3V
        }; 
    if (vol == 0)
    {
        return STA_ERR;
    }

    /*4路电压*/                                      
    for(i = 0;i<VOL_REG_NUM;i++)
    {
        if (STA_OK != cb2991ReadValid(vol_reg_addr[i], pBuffer))
        {
            return STA_ERR;
        }
        vol_data[i] = ((uint16_t)pBuffer[1]&0xff)|(((uint16_t)pBuffer[0]&0x3f) <<8);//bit0~13有效 bit14符号位 不考虑负电压
    }
    for(i = 0;i<VOL_REG_NUM;i++)
    {
        middle_val = vol_data[i];
        vol_data[i] = (middle_val * 305.18)/1000;//mV
    }
    
    vol[0] = vol_data[0]/1;     //1000mV/1
    vol[1] = vol_data[1]/1;     //1000mV/1
    vol[2] = vol_data[2]/1;     //1800mV/1
    vol[3] = vol_data[3]/1;     //3300mV/1
    
    return STA_OK;

}

/*!
    \brief      获取板卡温度
    \param      char *temp:获取板卡上三个温度采样点的温度
    \retval     ERR_STA:STA_ERR/STA_OK
*/
ERR_STA ReadTemp(char *temp)
{
    char tmp[5];
    if (temp == 0)
    {
        return STA_ERR;
    }
    if(STA_ERR ==  ReadCB2991Temp(tmp) )
    {
        return STA_ERR;
    }
    
    temp[0] = tmp[2];
    temp[1] = tmp[3];
    temp[2] = tmp[4];
    return STA_OK;
}

/*!
    \brief      获取板卡电压
    \param      uint16_t *vol:读取的四个电压采样值,单位为mv
    \retval     ERR_STA:STA_ERR/STA_OK
*/
ERR_STA ReadVol(uint16_t *vol)
{
    return ReadCB2991Vol(vol);
}
