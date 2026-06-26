#include "CLRD125.h"
#include "systick.h"
#include <stdio.h>

typedef struct
{
    uint8_t reg;
    uint8_t mask;
    uint8_t value;
    const char *name;
} CLRD125_CFG_ITEM;

static const CLRD125_CFG_ITEM g_clrd125BaseConfig[] =
{
    {CLRD125_REG_CHANNEL_PD,         0xFFU,                        0x00U,                        "channel power"},
    {CLRD125_REG_SMBUS_ENABLE,       CLRD125_SMBUS_ENABLE_MASK,   CLRD125_SMBUS_ENABLE_MASK,   "SMBus control"},
    {CLRD125_REG_PIN_OVERRIDE,       CLRD125_PIN_OVERRIDE_MASK,   CLRD125_PIN_OVERRIDE_MASK,   "MODE/EQ pin override"},
    {CLRD125_REG_PIN_ROUTE_OVERRIDE, CLRD125_ROUTE_OVERRIDE_MASK, CLRD125_ROUTE_OVERRIDE_MASK, "SEL/INPUT_EN override"},
    {CLRD125_REG_ROUTE_FANOUT,       0xFFU,                        CLRD125_ROUTE_FANOUT_VALUE,   "route and fanout"}
};

static const uint8_t g_clrd125CtleRegs[] =
{
    CLRD125_REG_CTLE_CH0, CLRD125_REG_CTLE_CH2,
    CLRD125_REG_CTLE_CH4, CLRD125_REG_CTLE_CH6
};

static const uint8_t g_clrd125VodRegs[] =
{
    CLRD125_REG_VOD_CH1, CLRD125_REG_VOD_CH3,
    CLRD125_REG_VOD_CH4, CLRD125_REG_VOD_CH5,
    CLRD125_REG_VOD_CH6, CLRD125_REG_VOD_CH7
};

static const uint8_t g_clrd125DemRegs[] =
{
    CLRD125_REG_DEM_CH1, CLRD125_REG_DEM_CH3,
    CLRD125_REG_DEM_CH4, CLRD125_REG_DEM_CH5,
    CLRD125_REG_DEM_CH6, CLRD125_REG_DEM_CH7
};

static ERR_STA clrd125ReadRetry(uint8_t regAddr, uint8_t *value)
{
    uint8_t retry;

    if (value == 0)
    {
        return STA_ERR;
    }

    retry = CLRD125_RETRY_CNT;
    while (retry > 0U)
    {
        if (STA_OK == clrd125Read(regAddr, value))
        {
            return STA_OK;
        }
        retry--;
        delay_1ms(2U);
    }

    printf("[CLRD125] read failed: reg=0x%02X\r\n", regAddr);
    return STA_ERR;
}

static ERR_STA clrd125WriteRetry(uint8_t regAddr, uint8_t value)
{
    uint8_t retry;

    retry = CLRD125_RETRY_CNT;
    while (retry > 0U)
    {
        if (STA_OK == clrd125Write(regAddr, value))
        {
            return STA_OK;
        }
        retry--;
        delay_1ms(2U);
    }

    printf("[CLRD125] write failed: reg=0x%02X value=0x%02X\r\n",
           regAddr, value);
    return STA_ERR;
}

static ERR_STA clrd125UpdateAndVerify(uint8_t regAddr,
                                     uint8_t mask,
                                     uint8_t value,
                                     const char *name)
{
    uint8_t oldValue;
    uint8_t writeValue;
    uint8_t readValue;

    oldValue = 0U;
    readValue = 0U;
    if (STA_OK != clrd125ReadRetry(regAddr, &oldValue))
    {
        return STA_ERR;
    }

    writeValue = (uint8_t)((oldValue & (uint8_t)(~mask)) | (value & mask));
    if (STA_OK != clrd125WriteRetry(regAddr, writeValue))
    {
        return STA_ERR;
    }
    if (STA_OK != clrd125ReadRetry(regAddr, &readValue))
    {
        return STA_ERR;
    }

    if ((readValue & mask) != (writeValue & mask))
    {
        printf("[CLRD125] verify failed: %s reg=0x%02X expected=0x%02X actual=0x%02X mask=0x%02X\r\n",
               name, regAddr, writeValue, readValue, mask);
        return STA_ERR;
    }

    printf("[CLRD125] %-22s reg=0x%02X old=0x%02X new=0x%02X OK\r\n",
           name, regAddr, oldValue, readValue);
    return STA_OK;
}

static ERR_STA clrd125CheckDevice(void)
{
    uint8_t deviceId;

    deviceId = 0U;
    if (STA_OK != clrd125ReadRetry(CLRD125_REG_DEVICE_ID, &deviceId))
    {
        return STA_ERR;
    }
    if (CLRD125_DEVICE_ID != deviceId)
    {
        printf("[CLRD125] unexpected ID: expected=0x%02X actual=0x%02X\r\n",
               CLRD125_DEVICE_ID, deviceId);
        return STA_ERR;
    }

    printf("[CLRD125] device ID 0x%02X detected\r\n", deviceId);
    return STA_OK;
}

static ERR_STA clrd125ApplyBaseConfig(void)
{
    uint32_t i;

    for (i = 0U; i < (sizeof(g_clrd125BaseConfig) / sizeof(g_clrd125BaseConfig[0])); i++)
    {
        if (STA_OK != clrd125UpdateAndVerify(g_clrd125BaseConfig[i].reg,
                                             g_clrd125BaseConfig[i].mask,
                                             g_clrd125BaseConfig[i].value,
                                             g_clrd125BaseConfig[i].name))
        {
            return STA_ERR;
        }
    }
    return STA_OK;
}

static ERR_STA clrd125ApplyRegisterList(const uint8_t *regs,
                                        uint32_t count,
                                        uint8_t mask,
                                        uint8_t value,
                                        const char *name)
{
    uint32_t i;

    for (i = 0U; i < count; i++)
    {
        if (STA_OK != clrd125UpdateAndVerify(regs[i], mask, value, name))
        {
            return STA_ERR;
        }
    }
    return STA_OK;
}

ERR_STA clrd125Init(void)
{
    uint8_t rxDetect;
    uint8_t rxLos;

    printf("\r\n[CLRD125] initialization start\r\n");
    gpioI2cInit(CLRD125_I2C);

    if (STA_OK != clrd125CheckDevice())
    {
        return STA_ERR;
    }
    if (STA_OK != clrd125ApplyBaseConfig())
    {
        return STA_ERR;
    }
    if (STA_OK != clrd125ApplyRegisterList(g_clrd125CtleRegs,
                                           sizeof(g_clrd125CtleRegs),
                                           CLRD125_CTLE_MASK,
                                           CLRD125_CTLE_VALUE,
                                           "receiver CTLE"))
    {
        return STA_ERR;
    }

    /* Change VOD[2:0] only; preserve MODE and protection fields. */
    if (STA_OK != clrd125ApplyRegisterList(g_clrd125VodRegs,
                                           sizeof(g_clrd125VodRegs),
                                           CLRD125_VOD_MASK,
                                           CLRD125_VOD_VALUE,
                                           "transmitter VOD"))
    {
        return STA_ERR;
    }
    if (STA_OK != clrd125ApplyRegisterList(g_clrd125DemRegs,
                                           sizeof(g_clrd125DemRegs),
                                           CLRD125_DEM_MASK,
                                           CLRD125_DEM_VALUE,
                                           "transmitter DEM"))
    {
        return STA_ERR;
    }

    rxDetect = 0U;
    rxLos = 0U;
    if (STA_OK == clrd125ReadStatus(&rxDetect, &rxLos))
    {
        printf("[CLRD125] status: RX_DETECT=0x%02X RX_LOS=0x%02X\r\n",
               rxDetect, rxLos);
    }

    printf("[CLRD125] initialization complete\r\n");
    return STA_OK;
}

ERR_STA clrd125InitWithRetry(void)
{
    uint8_t attempt;

    /* Datasheet tPOR requires 500 ms after power-on. */
    delay_1ms(CLRD125_POWER_ON_DELAY_MS);

    for (attempt = 1U; attempt <= CLRD125_RETRY_CNT; attempt++)
    {
        if (STA_OK == clrd125Init())
        {
            printf("[CLRD125] initialized on attempt %u\r\n", attempt);
            return STA_OK;
        }
        printf("[CLRD125] attempt %u/%u failed\r\n",
               attempt, CLRD125_RETRY_CNT);
        delay_1ms(CLRD125_RETRY_DELAY_MS);
    }

    printf("[CLRD125] initialization failed\r\n");
    return STA_ERR;
}

ERR_STA clrd125Read(uint8_t regAddr, uint8_t *val)
{
    if (val == 0)
    {
        return STA_ERR;
    }
    return gpioI2cRead(CLRD125_I2C, CLRD125_DEVICE_ADDR, regAddr, val, 1U);
}

ERR_STA clrd125Write(uint8_t regAddr, uint8_t val)
{
    return gpioI2cWrite(CLRD125_I2C, CLRD125_DEVICE_ADDR, regAddr, &val, 1U);
}

ERR_STA clrd125ReadStatus(uint8_t *rxDetect, uint8_t *rxLos)
{
    if ((rxDetect == 0) || (rxLos == 0))
    {
        return STA_ERR;
    }
    if (STA_OK != clrd125ReadRetry(CLRD125_REG_RX_DETECT, rxDetect))
    {
        return STA_ERR;
    }
    return clrd125ReadRetry(CLRD125_REG_RX_LOS, rxLos);
}
