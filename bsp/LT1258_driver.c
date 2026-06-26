#include "LT1258.h"
#include <stdio.h>

/*
 * 历史对照驱动：
 * main()已经停用LT1258初始化，当前高速链路由CLRD125C V1.0配置。
 * 本文件仍保留在工程中，方便后续恢复旧硬件或进行A/B参数对比。
 */
typedef struct
{
    uint8_t reg;
    uint8_t value;
} LT1258_REG_VALUE;

static const uint8_t g_lt1258DeviceAddr[] =
{
    LT1258_DEVICE_ADDR_1,
    LT1258_DEVICE_ADDR_2,
    LT1258_DEVICE_ADDR_3,
    LT1258_DEVICE_ADDR_4
};

/*
 * 保留原始LT1258信号调理结果：
 *   SRXB0/SRXB1/DRXA0/DRXA1 EQ = 17.6 dB
 *   DTXB0/DTXB1/STXA0/STXA1 VOD = 0.8 Vp-p，DEM = -4.4 dB
 *
 * 表驱动的目的，是让四颗芯片共用同一组参数，避免复制代码时漏改。
 */
static const LT1258_REG_VALUE g_lt1258Bank80Config[] =
{
    {0x38U, 0xC0U}, {0x39U, 0x70U}, /* SRXB0 EQ */
    {0x78U, 0xC0U}, {0x79U, 0x70U}, /* SRXB1 EQ */
    {0x98U, 0xC0U}, {0x99U, 0x70U}, /* DRXA0 EQ */
    {0xD8U, 0xC0U}, {0xD9U, 0x70U}, /* DRXA1 EQ */

    {0x3AU, 0x03U}, {0x3BU, 0x70U}, {0x3CU, 0x20U}, /* DTXB0 */
    {0x7AU, 0x03U}, {0x7BU, 0x70U}, {0x7CU, 0x20U}, /* DTXB1 */
    {0x9AU, 0x03U}, {0x9BU, 0x70U}, {0x9CU, 0x20U}, /* STXA0 */
    {0xDAU, 0x03U}, {0xDBU, 0x70U}, {0xDCU, 0x20U}  /* STXA1 */
};

ERR_STA lt1258Write(uint8_t deviceAddr, uint8_t regAddr, uint8_t val)
{
    return gpioI2cWrite(LT1258_I2C, deviceAddr, regAddr, &val, 1U);
}

ERR_STA lt1258Read(uint8_t deviceAddr, uint8_t regAddr, uint8_t *val)
{
    if (val == 0)
    {
        return STA_ERR;
    }

    return gpioI2cRead(LT1258_I2C, deviceAddr, regAddr, val, 1U);
}

static ERR_STA lt1258SelectBank(uint8_t deviceAddr, uint8_t bank)
{
    return lt1258Write(deviceAddr, 0xFFU, bank);
}

static ERR_STA lt1258UpdateBits(uint8_t deviceAddr,
                                uint8_t regAddr,
                                uint8_t mask,
                                uint8_t value)
{
    uint8_t oldValue;
    uint8_t newValue;
    uint8_t readValue;

    if (STA_OK != lt1258Read(deviceAddr, regAddr, &oldValue))
    {
        return STA_ERR;
    }

    /* 保留寄存器中不属于本配置项的位。 */
    newValue = (uint8_t)((oldValue & (uint8_t)(~mask)) | (value & mask));
    if (STA_OK != lt1258Write(deviceAddr, regAddr, newValue))
    {
        return STA_ERR;
    }
    if (STA_OK != lt1258Read(deviceAddr, regAddr, &readValue))
    {
        return STA_ERR;
    }

    return ((readValue & mask) == (newValue & mask)) ? STA_OK : STA_ERR;
}

static ERR_STA lt1258WriteAndVerify(uint8_t deviceAddr,
                                    uint8_t regAddr,
                                    uint8_t value)
{
    uint8_t readValue;

    if (STA_OK != lt1258Write(deviceAddr, regAddr, value))
    {
        return STA_ERR;
    }
    if (STA_OK != lt1258Read(deviceAddr, regAddr, &readValue))
    {
        return STA_ERR;
    }

    return (readValue == value) ? STA_OK : STA_ERR;
}

static ERR_STA lt1258ConfigureRoute(uint8_t deviceAddr)
{
    if (STA_OK != lt1258SelectBank(deviceAddr, 0x81U))
    {
        return STA_ERR;
    }

    /* 使INEN、SEL0、SEL1由软件寄存器控制。 */
    if (STA_OK != lt1258UpdateBits(deviceAddr, 0x60U, 0x07U, 0x07U))
    {
        return STA_ERR;
    }

    /*
     * INEN=01：输入输出持续使能并保持50 Ω端接。
     * SEL0=01：SRXB0->DTXB0，DRXA0->STXA0。
     * SEL1=01：SRXB1->DTXB1，DRXA1->STXA1。
     */
    return lt1258UpdateBits(deviceAddr, 0x63U, 0x3FU, 0x15U);
}

static ERR_STA lt1258ConfigureSignal(uint8_t deviceAddr)
{
    uint32_t i;

    /* Bank 0x80保存各高速通道的EQ、VOD和DEM参数。 */
    if (STA_OK != lt1258SelectBank(deviceAddr, 0x80U))
    {
        return STA_ERR;
    }

    for (i = 0U;
         i < (sizeof(g_lt1258Bank80Config) / sizeof(g_lt1258Bank80Config[0]));
         i++)
    {
        if (STA_OK != lt1258WriteAndVerify(deviceAddr,
                                           g_lt1258Bank80Config[i].reg,
                                           g_lt1258Bank80Config[i].value))
        {
            return STA_ERR;
        }
    }

    return STA_OK;
}

static ERR_STA lt1258ConfigureDevice(uint8_t deviceAddr)
{
    if (STA_OK != lt1258ConfigureRoute(deviceAddr))
    {
        return STA_ERR;
    }

    return lt1258ConfigureSignal(deviceAddr);
}

ERR_STA lt1258Init(void)
{
    uint32_t i;
    ERR_STA result;

    result = STA_OK;
    gpioI2cInit(LT1258_I2C);

    for (i = 0U;
         i < (sizeof(g_lt1258DeviceAddr) / sizeof(g_lt1258DeviceAddr[0]));
         i++)
    {
        if (STA_OK != lt1258ConfigureDevice(g_lt1258DeviceAddr[i]))
        {
            printf("LT1258 device 0x%02X config failed\r\n",
                   g_lt1258DeviceAddr[i]);
            result = STA_ERR;
        }
    }

    return result;
}
