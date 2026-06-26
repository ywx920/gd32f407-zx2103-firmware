/*!
    \file  spi_flash.c
    \brief 
*/

#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H

#include "gd32f4xx.h"
#include "common.h"
#include "spi.h"

#define SPI_FLASH_CS SPI_CTLER1

//flash 扇区、PAGE、容量参数
#define SECTOR_SIZE (4*1024)
#define PAGE_SIZE (256)
#define CHIP_SIZE (16384*1024) //16M Byte
#define SECTOR_NUM (CHIP_SIZE/SECTOR_SIZE) //扇区个数
#define PAGE_NUM (CHIP_SIZE/PAGE_SIZE) //PAGE个数

/*!
    \brief      SPI flash初始化
    \param  	uint8_t spix:SPI_FLASH_CS
    \retval     void
*/
extern void spiFlashInit(uint8_t spix);

/*!
    \brief      SPI flash读取状态寄存器1
    \param  	uint8_t spix:SPI_CTLER0/SPI_CTLER1
				uint8_t *id:读到的id,三个字节
    \retval     返回:STA_OK/STA_ERR
*/
extern ERR_STA spiFlashReadID(uint8_t spix, uint8_t *id);

/*!
    \brief      SPI flash擦除
    \param  	uint8_t spix:SPI_CTLER0/SPI_CTLER1
				uint32_t sector:扇区号
    \retval     返回:STA_OK/STA_ERR
*/
extern ERR_STA spiFlashEarse(uint8_t spix, uint32_t sector);

/*!
    \brief      SPI flash页编程
    \param  	uint8_t spix:SPI_CTLER0/SPI_CTLER1
				uint32_t page:页号
				uint8_t *buf：写入的数据
				uint32 size：写入数据的大小
    \retval     返回:STA_OK/STA_ERR
*/
extern ERR_STA spiFlashPageProgram(uint8_t spix, uint32_t page, uint8_t *buf, uint32_t size);

/*!
    \brief      SPI flash页读取
    \param  	uint8_t spix:SPI_CTLER0/SPI_CTLER1
				uint32_t page:页号
				uint8_t *buf：读取的数据
				uint32 size：读取数据的大小
    \retval     返回:STA_OK/STA_ERR
*/
extern ERR_STA spiFlashPageRead(uint8_t spix, uint32_t page, uint8_t *buf, uint32_t size);

/*!
    \brief      SPI flash全片擦除编程读取并校验
    \param  	void
    \retval     void
*/
extern void testSpiFlash(void);

#endif /* __SPI_FLASH_H */

