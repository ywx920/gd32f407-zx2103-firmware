#include "gd32f4xx.h"
#include "spi_flash.h"
#include "spi.h"
#include <stdio.h>

//flash 操作命令
#define CMD_STA1 0x05 //状态寄存器1
#define CMD_STA2 0x35 //状态寄存器1
#define CMD_STA3 0x15 //状态寄存器1
#define CMD_WRITE_STA1 0x01 //写状态寄存器1
#define CMD_WRITE_STA2 0x31 //写状态寄存器2
#define CMD_WRITE_STA3 0x11 //写状态寄存器3
#define CMD_WRITE_EN 0x06 //写使能
#define CMD_ID 0x9F
#define CMD_SECTOR_EARSE 0x20
#define CMD_PAGE_PROGRAM 0x02
#define CMD_READ 0x03

#define OPRATION_BUSY 0x01 //状态寄存器1 忙状态
#define OPRATION_WRITE_EN 0x02 //状态寄存器1 写使能

/*!
    \brief      SPI flash读取状态寄存器1
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint8_t *val:读取状态寄存器1的值
    \retval     返回:STA_OK/STA_ERR
*/
static ERR_STA spiFlashReadSta1(uint8_t spix, uint8_t *val)
{
    uint8_t regVal;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    
    //CS=0
    SET_NSS_LOW(spix);
    //擦除指令
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], CMD_STA1);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //清空数据
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    spi_i2s_data_receive(spiCtler[spix]);
    //读取数据
    spi_i2s_data_transmit(spiCtler[spix], 0);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    regVal = spi_i2s_data_receive(spiCtler[spix]);
    
    //CS=1
    SET_NSS_HIGH(spix);
    
    *val = regVal;
    
    return STA_OK;
}

/*!
    \brief      SPI flash读取状态寄存器2
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint8_t *val:读取状态寄存器1的值
    \retval     返回:STA_OK/STA_ERR
*/
/* static ERR_STA spiFlashReadSta2(uint8_t spix, uint8_t *val)
{
    uint8_t regVal;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    
    //CS=0
    SET_NSS_LOW(spix);
    //擦除指令
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], CMD_STA2);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //清空数据
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    spi_i2s_data_receive(spiCtler[spix]);
    //读取数据
    spi_i2s_data_transmit(spiCtler[spix], 0);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    regVal = spi_i2s_data_receive(spiCtler[spix]);
    
    //CS=1
    SET_NSS_HIGH(spix);
    
    *val = regVal;
    
    return STA_OK;
} */

/*!
    \brief      SPI flash读取状态寄存器3
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint8_t *val:读取状态寄存器1的值
    \retval     返回:STA_OK/STA_ERR
*/
/* static ERR_STA spiFlashReadSta3(uint8_t spix, uint8_t *val)
{
    uint8_t regVal;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    
    //CS=0
    SET_NSS_LOW(spix);
    //擦除指令
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], CMD_STA3);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //清空数据
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    spi_i2s_data_receive(spiCtler[spix]);
    //读取数据
    spi_i2s_data_transmit(spiCtler[spix], 0);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    regVal = spi_i2s_data_receive(spiCtler[spix]);
    
    //CS=1
    SET_NSS_HIGH(spix);
    
    *val = regVal;
    
    return STA_OK;
} */

/*!
    \brief      SPI flash等待flash操作完成，变为空闲状态
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint32_t timeout:等待超时时间，100000;//约2秒
    \retval     返回:STA_OK/STA_ERR
*/
ERR_STA spiFlashWaitFree(uint8_t spix, uint32_t timeout)
{
    uint32_t timetmp = timeout;
    uint8_t sta;
    
    do{
        if(STA_ERR == spiFlashReadSta1(spix, &sta))
        {
            printf("spiFlashReadSta1 fail\r\n");
            return STA_ERR;
        }
        timetmp -= 1;
        if(0 == timetmp)
        {
            printf("spiFlashReadSta1 timeout\r\n");
            return STA_ERR;
        }
    }while(OPRATION_BUSY == (OPRATION_BUSY & sta) );
    
    return STA_OK;
}

/*!
    \brief      SPI flash写使能
    \param      void
    \retval     返回:STA_OK/STA_ERR
*/
static ERR_STA spiFlashWriteEn(uint8_t spix)
{
    //uint8_t sta;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    
    //CS=0
    SET_NSS_LOW(spix);
    //擦除指令
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], CMD_WRITE_EN);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    
    //CS=1
    SET_NSS_HIGH(spix);
    
    return STA_OK;
}

/*!
    \brief      SPI flash写状态寄存器
    \param      void
    \retval     返回:STA_OK/STA_ERR
*/
/* static ERR_STA spiFlashWriteSta(uint8_t spix, uint8_t reg, uint8_t val)
{
    //uint8_t sta;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    
    spiFlashWriteEn(spix);
    if(STA_ERR == spiFlashWaitFree(spix, 50000))//约1秒
    {
        return STA_ERR;
    }
    //CS=0
    SET_NSS_LOW(spix);
    //擦除指令
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], reg);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    
    spi_i2s_data_transmit(spiCtler[spix], val);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //CS=1
    SET_NSS_HIGH(spix);
    
    return STA_OK;
} */

/*!
    \brief      SPI flash接触保护
    \param      void
    \retval     返回:STA_OK/STA_ERR
*/
/* static ERR_STA spiFlashUnprotect(uint8_t spix)
{
    uint8_t sta;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    
    spiFlashReadSta3(spix, &sta);
    
    spiFlashReadSta2(spix, &sta);
    spiFlashWriteSta(spix, CMD_WRITE_STA2, sta&0xFE);
    spiFlashReadSta1(spix, &sta);
    spiFlashWriteSta(spix, CMD_WRITE_STA1, sta|0x80);

    return 0;
} */

/*!
    \brief      SPI flash初始化
    \param      uint8_t spix:SPI_FLASH_CS
    \retval     void
*/
void spiFlashInit(uint8_t spix)
{
    spiInit(spix);
    //spiFlashUnprotect(spix);
}

/*!
    \brief      SPI flash读取状态寄存器1
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint8_t *id:读到的id,三个字节
    \retval     返回:STA_OK/STA_ERR
*/
ERR_STA spiFlashReadID(uint8_t spix, uint8_t *id)
{
    uint8_t regVal;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //CS=0
    SET_NSS_LOW(spix);
    //发送ID指令
    spi_i2s_data_transmit(spiCtler[spix], CMD_ID);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //清空数据
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    spi_i2s_data_receive(spiCtler[spix]);
    //读取数据
    spi_i2s_data_transmit(spiCtler[spix], 0);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    regVal = spi_i2s_data_receive(spiCtler[spix]);
    id[0] = regVal;
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], 0);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    regVal = spi_i2s_data_receive(spiCtler[spix]);
    id[1] = regVal;
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], 0);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    regVal = spi_i2s_data_receive(spiCtler[spix]);
    id[2] = regVal;
    
    //CS=1
    SET_NSS_HIGH(spix);
        
    return STA_OK;
}

/*!
    \brief      SPI flash擦除
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint32_t sector:扇区号
    \retval     返回:STA_OK/STA_ERR
*/
ERR_STA spiFlashEarse(uint8_t spix, uint32_t sector)
{
    uint32_t addr32;
    uint8_t addr8;
    //uint8_t sta;
    //uint32_t timeout;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    if(sector >= SECTOR_NUM)
    {
        return STA_ERR;
    }
    
    addr32 = sector*SECTOR_SIZE;
    
    spiFlashWriteEn(spix);
    
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //CS=0
    SET_NSS_LOW(spix);
    //擦除指令
    spi_i2s_data_transmit(spiCtler[spix], CMD_SECTOR_EARSE);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    
    //ADDR23~16
    addr8 = (uint8_t)((addr32 >> 16) & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    //ADDR15~8
    addr8 = (uint8_t)((addr32 >> 8) & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    //ADDR7~0
    addr8 = (uint8_t)(addr32 & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    
    //CS=1
    SET_NSS_HIGH(spix);
    
    //判断操作是否完成
    if(STA_ERR == spiFlashWaitFree(spix, 100000))//约2秒
    {
        return STA_ERR;
    }
    
    return STA_OK;
}

/*!
    \brief      SPI flash页编程
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint32_t page:页号
                uint8_t *buf：写入的数据
                uint32 size：写入数据的大小
    \retval     返回:STA_OK/STA_ERR
*/
ERR_STA spiFlashPageProgram(uint8_t spix, uint32_t page, uint8_t *buf, uint32_t size)
{
    uint32_t addr32, i;
    uint8_t addr8;
    //uint8_t sta;
    //uint32_t timeout;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    if(page > PAGE_NUM)
    {
        return STA_ERR;
    }
    
    spiFlashWriteEn(spix);
    
    addr32 = page*PAGE_SIZE;
    
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //CS=0
    SET_NSS_LOW(spix);
    //擦除指令
    spi_i2s_data_transmit(spiCtler[spix], CMD_PAGE_PROGRAM);
    
    //ADDR23~16
    addr8 = (uint8_t)((addr32 >> 16) & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    //ADDR15~8
    addr8 = (uint8_t)((addr32 >> 8) & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    //ADDR7~0
    addr8 = (uint8_t)(addr32 & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    //发送数据
    for(i = 0; i < size; i++)
    {
        while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
        spi_i2s_data_transmit(spiCtler[spix], buf[i]);
    }
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    
    //CS=1
    SET_NSS_HIGH(spix);
    
    //判断操作是否完成
    if(STA_ERR == spiFlashWaitFree(spix, 50000))//约1秒
    {
        return STA_ERR;
    }
    
    return STA_OK;
}

/*!
    \brief      SPI flash页读取
    \param      uint8_t spix:SPI_CTLER0/SPI_CTLER1
                uint32_t page:页号
                uint8_t *buf：读取的数据
                uint32 size：读取数据的大小
    \retval     返回:STA_OK/STA_ERR
*/
ERR_STA spiFlashPageRead(uint8_t spix, uint32_t page, uint8_t *buf, uint32_t size)
{
    uint32_t addr32, i;
    uint8_t addr8;
    //uint8_t sta;
    //uint32_t timeout;
    if(spix >= SPI_NUM)
    {
        return STA_ERR;
    }
    if(page > PAGE_NUM)
    {
        return STA_ERR;
    }
    
    addr32 = page*PAGE_SIZE;
    
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //CS=0
    SET_NSS_LOW(spix);
    //读指令
    spi_i2s_data_transmit(spiCtler[spix], CMD_READ);
    
    //ADDR23~16
    addr8 = (uint8_t)((addr32 >> 16) & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    //ADDR15~8
    addr8 = (uint8_t)((addr32 >> 8) & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    //ADDR7~0
    addr8 = (uint8_t)(addr32 & 0xFF);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    spi_i2s_data_transmit(spiCtler[spix], addr8);
    while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
    //清空数据
    while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
    spi_i2s_data_receive(spiCtler[spix]);
    //接收数据
    for(i = 0; i < size; i++)
    {
        spi_i2s_data_transmit(spiCtler[spix], 0);
        while(SET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_TRANS));
        while(RESET == spi_i2s_flag_get(spiCtler[spix], SPI_FLAG_RBNE));
        buf[i] = spi_i2s_data_receive(spiCtler[spix]);
    }
    
    //CS=1
    SET_NSS_HIGH(spix);
    
    return STA_OK;
}

/*!
    \brief      SPI flash全片擦除编程读取并校验
    \param      void
    \retval     void
*/
void testSpiFlash(void)
{
    uint8_t buf[PAGE_SIZE];
    uint32_t *dat = (uint32_t *)buf;
    
    uint32_t page, sector, i;
    uint32_t testdat = 0;
    
    printf("spiFlashEarse...\r\n");
    //擦除和写入数据
    for(sector = 0; sector < SECTOR_NUM; sector++)
    {
        //printf("\r spiFlashEarse:%d/%d", sector, SECTOR_NUM);
        if(STA_ERR == spiFlashEarse(SPI_FLASH_CS, sector))
        {
            printf("spiFlashEarse fail\r\n");
            return;
        }
    }
    
    if(STA_ERR == spiFlashPageRead(SPI_FLASH_CS, 0, buf, PAGE_SIZE))
    {
        printf("spiFlashPageRead fail\r\n");
        return;
    }
    printf("spiFlashPageProgram...\r\n");
    testdat = 0;
    //编程
    for(page = 0; page < PAGE_NUM; page++)
    {
        //准备测试数据
        for(i = 0; i < PAGE_SIZE/4; i++, testdat++)
        {
            dat[i] = testdat;
        }
        if(STA_ERR == spiFlashPageProgram(SPI_FLASH_CS, page, buf, PAGE_SIZE))
        {
            printf("spiFlashPageProgram fail\r\n");
            return;
        }
    }
    
    printf("spiFlashPageRead check...\r\n");
    testdat = 0;
    //读取数据并校验
    for(page = 0; page < PAGE_NUM; page++)
    {
        if(STA_ERR == spiFlashPageRead(SPI_FLASH_CS, page, buf, PAGE_SIZE))
        {
            printf("spiFlashPageRead fail\r\n");
            return;
        }
            
        
        for(i = 0; i < PAGE_SIZE/4; i++, testdat++)
        {
            if( testdat != dat[i] )
            {
                printf("check error:dat[%d] = 0x%08x, testdat = 0x%08x\r\n", i, dat[i], testdat);
            }
        }
    }
    
    printf("test over\r\n");
}
