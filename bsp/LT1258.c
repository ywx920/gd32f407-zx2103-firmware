/*
 * Legacy expanded implementation retained for comparison only.
 * The Keil project now builds LT1258_driver.c.
 */
#include "gd32f4xx.h"
#include "LT1258.h"
#include <string.h>
#include "stdio.h"

/*!
    \brief      初始化LT1258芯片
    \param      void
    \retval     ERR_STA
*/

ERR_STA lt1258Init(void)
{

    gpioI2cInit(LT1258_I2C);
    lt1258Config1();
    lt1258Config2();
    lt1258Config3();
    lt1258Config4();

    return STA_OK;
}

/*!
    \brief      读取LT1258的状态
    \param      uint8_t chipx:哪个芯片，取值LT1258_CHIP0_D1/LT1258_CHIP1_D3/LT1258_CHIP2_D4/LT1258_CHIP3_D5
                uint8_t regAddr:寄存器地址
                uint16_t val:写到的寄存器值
    \retval     ERR_STA
*/
//
ERR_STA lt1258Write(uint8_t deviceAddr, uint8_t regAddr, uint8_t val)
{
    uint8_t buf[1];

    buf[0] = 0xFF & (val);
    // 调用写函数，从LT1258_I2C总线将buf中的数据写入到指定的LT1258芯片的寄存器
    if (STA_ERR == gpioI2cWrite(LT1258_I2C, deviceAddr, regAddr, buf, 1))
    {
        return STA_ERR;
    }

    return STA_OK;
}

/*!
    \brief      读取LT1258的状态
    \param      uint8_t chipx:哪个芯片，取值LT1258_CHIP0_D1/LT1258_CHIP1_D3/LT1258_CHIP2_D4/LT1258_CHIP3_D5
                uint8_t regAddr:寄存器地址
                uint16_t *val:读到的寄存器值
    \retval     ERR_STA
*/
ERR_STA lt1258Read(uint8_t deviceAddr, uint8_t regAddr, uint8_t *val)
{
    uint8_t buf[1]; // 字节数组，用于接收从芯片读取的数据
    uint16_t tmp;   // 存储合并后的寄存器值

    // 调用读函数，从指定的I2C地址读取数据，存放于buf中
    if (STA_ERR == gpioI2cRead(LT1258_I2C, deviceAddr, regAddr, buf, 1))
    {
        return STA_ERR;
    }

    // 合并字节数据并存储于tmp中
    tmp = buf[0];
    *val = tmp;

	 memset(buf, 0, sizeof(buf));

    return STA_OK;
}

static void lt1258Config1(void)
{
	uint8_t read_val[1] = {0x00};
	// Switching Bank Address
	lt1258Write(LT1258_DEVICE_ADDR_1, 0xff, 0x81);

	/***************************INEN Software Configuration********************/
	// printf("LT1258-1 INEN Software Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x60, read_val);
	// printf("LT1258-1 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x60, read_val[0] | BIT2); // 0x8160[2] = 1
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x60, read_val);
	// printf("LT1258-1 Write Done --- Read Reg 0x8160:%x \r\n\r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x63, read_val);
	// printf("LT1258-1 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x63, read_val[0] & (~BIT5));
	// lt1258Write(LT1258_DEVICE_ADDR_1, 0x63, read_val[0] & (~BIT4));//0x8163[5:4] = 2b’00
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x63, read_val[0] | BIT4); // 0x8163[5:4] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x63, read_val);
	// printf("LT1258-1 Write Done --- Read Reg 0x8163:%x \r\n\r\n\r\n", read_val[0]);

	/*********************SEL0 and SEL1 Software Configuration******************/
	// printf("LT1258-1 SEL0 and SEL1 Software Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x60, read_val);
	// printf("LT1258-1 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x60, read_val[0] | BIT0); // 0x8160[0] = 1
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x60, read_val);
	// printf("LT1258-1 Write Done --- Read Reg 0x8160:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x63, read_val);
	// printf("LT1258-1 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x63, read_val[0] & (~BIT1));
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x63, read_val[0] | BIT0); // 0x8163[1:0] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x63, read_val);
	// printf("LT1258-1 Write Done --- Read Reg 0x8163:%x \r\n", read_val[0]);
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x60, read_val);
	// printf("LT1258-1 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x60, read_val[0] | BIT1); // 0x8160[1] = 1
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x60, read_val);
	// printf("LT1258-1 Write Done --- Read Reg 0x8160:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x63, read_val);
	// printf("LT1258-1 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x63, read_val[0] & (~BIT3));
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x63, read_val[0] | BIT2); // 0x8163[3:2] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x63, read_val);
	// printf("LT1258-1 Write Done --- Read Reg 0x8163:%x \r\n\r\n\r\n", read_val[0]);

	/****************Equalizer Software Configuration*****************/
	// printf("LT1258-1 Equalizer Software Configuration \r\n\r\n");
	// Switching Bank Address
	lt1258Write(LT1258_DEVICE_ADDR_1, 0xff, 0x80);

	/************SRXB0**********/
	// printf("LT1258-1 SRXB0 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x38, read_val);
	// printf("LT1258-1 Read Reg 0x8038:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x38, 0xc0); // 0x8038=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x38, read_val);
	// printf("LT1258-1 Read Reg 0x8038:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x39, read_val);
	// printf("LT1258-1 Read Reg 0x8039:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x39, 0x70); // 0x8039=0x70
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x39, read_val);
	// printf("LT1258-1 Read Reg 0x8039:%x \r\n", read_val[0]);

	/************SRXB1**********/
	// printf("SRXB1 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x78, read_val);
	// printf("LT1258-1 Read Reg 0x8078:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x78, 0xc0); // 0x8078=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x78, read_val);
	// printf("LT1258-1 Read Reg 0x8078:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x79, read_val);
	// printf("LT1258-1 Read Reg 0x8079:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x79, 0x70); // 0x8079=0x70
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x79, read_val);
	// printf("LT1258-1 Read Reg 0x8079:%x \r\n", read_val[0]);

	/************DRXA0**********/
	// printf("LT1258-1 DRXA0 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x98, read_val);
	// printf("LT1258-1 Read Reg 0x8098:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x98, 0xc0); // 0x8098=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x98, read_val);
	// printf("LT1258-1 Read Reg 0x8098:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x99, read_val);
	// printf("LT1258-1 Read Reg 0x8099:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x99, 0x70); // 0x8099=0x70
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x99, read_val);
	// printf("LT1258-1 Read Reg 0x8099:%x \r\n", read_val[0]);

	/************DRXA1**********/
	// printf("LT1258-1 DRXA1 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xD8, read_val);
	// printf("LT1258-1 Read Reg 0x80D8:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0xD8, 0xc0); // 0x80D8=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xD8, read_val);
	// printf("LT1258-1 Read Reg 0x80D8:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xD9, read_val);
	// printf("LT1258-1 Read Reg 0x80D9:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0xD9, 0x70); // 0x80D9=0x70
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xD9, read_val);
	// printf("LT1258-1 Read Reg 0x80D9:%x \r\n", read_val[0]);

	/****************De-Emphasis and Output Swing Software Configuration*****************/
	// printf("LT1258-1 De-Emphasis and Output Swing Software Configuration \r\n\r\n");

	/************DTXB0**********/
	// printf("LT1258-1 DTXB0 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x3A, read_val);
	// printf("LT1258-1 Read Reg 0x803A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x3A, 0x03); // 0x803A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x3A, read_val);
	// printf("LT1258-1 Read Reg 0x803A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x3B, read_val);
	// printf("LT1258-1 Read Reg 0x803B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x3B, 0x70); // 0x803B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x3B, read_val);
	// printf("LT1258-1 Read Reg 0x803B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_1, 0x3C, read_val);
	// printf("LT1258-1 Read Reg 0x803C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x3C, 0x20); // 0x803C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x3C, read_val);
	// printf("LT1258-1 Read Reg 0x803C:%x \r\n", read_val[0]);

	/************DTXB1**********/
	// printf("LT1258-1 DTXB1 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x7A, read_val);
	// printf("LT1258-1 Read Reg 0x803A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x7A, 0x03); // 0x807A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x7A, read_val);
	// printf("LT1258-1 Read Reg 0x807A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x7B, read_val);
	// printf("LT1258-1 Read Reg 0x807B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x7B, 0x70); // 0x807B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x7B, read_val);
	// printf("LT1258-1 Read Reg 0x807B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_1, 0x7C, read_val);
	// printf("LT1258-1 Read Reg 0x807C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x7C, 0x20); // 0x807C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x7C, read_val);
	// printf("LT1258-1 Read Reg 0x807C:%x \r\n", read_val[0]);

	/************STXA0**********/
	// printf("LT1258-1 STXA0 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x9A, read_val);
	// printf("LT1258-1 Read Reg 0x809A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x9A, 0x03); // 0x809A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x9A, read_val);
	// printf("LT1258-1 Read Reg 0x809A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x9B, read_val);
	// printf("LT1258-1 Read Reg 0x809B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x9B, 0x70); // 0x809B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x9B, read_val);
	// printf("LT1258-1 Read Reg 0x809B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_1, 0x9C, read_val);
	// printf("LT1258-1 Read Reg 0x809C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0x9C, 0x20); // 0x809C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_1, 0x9C, read_val);
	// printf("LT1258-1 Read Reg 0x809C:%x \r\n", read_val[0]);

	/************STXA1**********/
	// printf("LT1258-1 STXA1 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xDA, read_val);
	// printf("LT1258-1 Read Reg 0x80DA:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0xDA, 0x03); // 0x80DA=0x03
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xDA, read_val);
	// printf("LT1258-1 Read Reg 0x80DA:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xDB, read_val);
	// printf("LT1258-1 Read Reg 0x80DB:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0xDB, 0x70); // 0x80DB=0x70
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xDB, read_val);
	// printf("LT1258-1 Read Reg 0x80DB:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_1, 0xDC, read_val);
	// printf("LT1258-1 Read Reg 0x80DC:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_1, 0xDC, 0x20); // 0x80DC=0x20
	lt1258Read(LT1258_DEVICE_ADDR_1, 0xDC, read_val);
	// printf("LT1258-1 Read Reg 0x80DC:%x \r\n", read_val[0]);
	// printf("LT1258-1 Config Done!!!!!! \r\n\r\n\r\n\r\n\r\n");
}


static void lt1258Config2(void)
{
	uint8_t read_val[1] = {0x00};
	// Switching Bank Address
	lt1258Write(LT1258_DEVICE_ADDR_2, 0xff, 0x81);
	// printf("LT1258-2 INEN Software Configuration \r\n");
	/***************************INEN Software Configuration********************/
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x60, read_val);
	// printf("LT1258-2 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x60, read_val[0] | BIT2); // 0x8160[2] = 1
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x60, read_val);
	// printf("LT1258-2 Write Done --- Read Reg 0x8160:%x \r\n\r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x63, read_val);
	// printf("LT1258-2 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x63, read_val[0] & (~BIT5));
	// lt1258Write(LT1258_DEVICE_ADDR_2, 0x63, read_val[0] & (~BIT4));//0x8163[5:4] = 2b’00
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x63, read_val[0] | BIT4); // 0x8163[5:4] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x63, read_val);
	// printf("LT1258-2 Write Done --- Read Reg 0x8163:%x \r\n", read_val[0]);

	// printf("LT1258-2 SEL0 and SEL1 Software Configuration \r\n\r\n\r\n");
	/*********************SEL0 and SEL1 Software Configuration******************/
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x60, read_val);
	// printf("LT1258-2 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x60, read_val[0] | BIT0); // 0x8160[0] = 1
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x60, read_val);
	// printf("LT1258-2 Write Done --- Read Reg 0x8160:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x63, read_val);
	// printf("LT1258-2 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x63, read_val[0] & (~BIT1));
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x63, read_val[0] | BIT0); // 0x8163[1:0] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x63, read_val);
	// printf("LT1258-2 Write Done --- Read Reg 0x8163:%x \r\n", read_val[0]);
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x60, read_val);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x60, read_val[0] | BIT1); // 0x8160[1] = 1
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x63, read_val);
	// printf("LT1258-2 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x63, read_val[0] & (~BIT3));
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x63, read_val[0] | BIT2); // 0x8163[3:2] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x63, read_val);
	// printf("LT1258-2 Write Done --- Read Reg 0x8163:%x \r\n\r\n\r\n", read_val[0]);

	/****************Equalizer Software Configuration*****************/
	// printf("LT1258-2 Equalizer Software Configuration \r\n\r\n");
	// Switching Bank Address
	lt1258Write(LT1258_DEVICE_ADDR_2, 0xff, 0x80);

	/************SRXB0**********/
	// printf("LT1258-2 SRXB0 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x38, read_val);
	// printf("LT1258-2 Read Reg 0x8038:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x38, 0xc0); // 0x8038=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x38, read_val);
	// printf("LT1258-2 Read Reg 0x8038:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x39, read_val);
	// printf("LT1258-2 Read Reg 0x8039:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x39, 0x70); // 0x8039=0x70
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x39, read_val);
	// printf("LT1258-2 Read Reg 0x8039:%x \r\n", read_val[0]);

	/************SRXB1**********/
	// printf("LT1258-2 SRXB1 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x78, read_val);
	// printf("LT1258-2 Read Reg 0x8078:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x78, 0xc0); // 0x8078=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x78, read_val);
	// printf("LT1258-2 Read Reg 0x8078:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x79, read_val);
	// printf("LT1258-2 Read Reg 0x8079:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x79, 0x70); // 0x8079=0x70
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x79, read_val);
	// printf("LT1258-2 Read Reg 0x8079:%x \r\n", read_val[0]);

	/************DRXA0**********/
	// printf("LT1258-2 DRXA0 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x98, read_val);
	// printf("LT1258-2 Read Reg 0x8098:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x98, 0xc0); // 0x8098=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x98, read_val);
	// printf("LT1258-2 Read Reg 0x8098:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x99, read_val);
	// printf("LT1258-2 Read Reg 0x8099:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x99, 0x70); // 0x8099=0x70
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x99, read_val);
	// printf("LT1258-2 Read Reg 0x8099:%x \r\n", read_val[0]);

	/************DRXA1**********/
	// printf("LT1258-2 DRXA1 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xD8, read_val);
	// printf("LT1258-2 Read Reg 0x80D8:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0xD8, 0xc0); // 0x80D8=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xD8, read_val);
	// printf("LT1258-2 Read Reg 0x80D8:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xD9, read_val);
	// printf("LT1258-2 Read Reg 0x80D9:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0xD9, 0x70); // 0x80D9=0x70
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xD9, read_val);
	// printf("LT1258-2 Read Reg 0x80D9:%x \r\n", read_val[0]);

	/****************De-Emphasis and Output Swing Software Configuration*****************/
	// printf("LT1258-2 De-Emphasis and Output Swing Software Configuration \r\n\r\n");

	/************DTXB0**********/
	// printf("LT1258-2 DTXB0 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x3A, read_val);
	// printf("LT1258-2 Read Reg 0x803A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x3A, 0x03); // 0x803A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x3A, read_val);
	// printf("LT1258-2 Read Reg 0x803A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x3B, read_val);
	// printf("LT1258-2 Read Reg 0x803B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x3B, 0x70); // 0x803B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x3B, read_val);
	// printf("LT1258-2 Read Reg 0x803B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_2, 0x3C, read_val);
	// printf("LT1258-2 Read Reg 0x803C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x3C, 0x20); // 0x803C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x3C, read_val);
	// printf("LT1258-2 Read Reg 0x803C:%x \r\n", read_val[0]);

	/************DTXB1**********/
	// printf("LT1258-2 DTXB1 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x7A, read_val);
	// printf("LT1258-2 Read Reg 0x803A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x7A, 0x03); // 0x807A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x7A, read_val);
	// printf("LT1258-2 Read Reg 0x807A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x7B, read_val);
	// printf("LT1258-2 Read Reg 0x807B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x7B, 0x70); // 0x807B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x7B, read_val);
	// printf("LT1258-2 Read Reg 0x807B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_2, 0x7C, read_val);
	// printf("LT1258-2 Read Reg 0x807C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x7C, 0x20); // 0x807C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x7C, read_val);
	// printf("LT1258-2 Read Reg 0x807C:%x \r\n", read_val[0]);

	/************STXA0**********/
	// printf("LT1258-2 STXA0 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x9A, read_val);
	// printf("LT1258-2 Read Reg 0x809A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x9A, 0x03); // 0x809A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x9A, read_val);
	// printf("LT1258-2 Read Reg 0x809A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x9B, read_val);
	// printf("LT1258-2 Read Reg 0x809B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x9B, 0x70); // 0x809B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x9B, read_val);
	// printf("LT1258-2 Read Reg 0x809B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_2, 0x9C, read_val);
	// printf("LT1258-2 Read Reg 0x809C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0x9C, 0x20); // 0x809C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_2, 0x9C, read_val);
	// printf("LT1258-2 Read Reg 0x809C:%x \r\n", read_val[0]);

	/************STXA1**********/
	// printf("LT1258-2 STXA1 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xDA, read_val);
	// printf("LT1258-2 Read Reg 0x80DA:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0xDA, 0x03); // 0x80DA=0x03
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xDA, read_val);
	// printf("LT1258-2 Read Reg 0x80DA:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xDB, read_val);
	// printf("LT1258-2 Read Reg 0x80DB:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0xDB, 0x70); // 0x80DB=0x70
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xDB, read_val);
	// printf("LT1258-2 Read Reg 0x80DB:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_2, 0xDC, read_val);
	// printf("LT1258-2 Read Reg 0x80DC:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_2, 0xDC, 0x20); // 0x80DC=0x20
	lt1258Read(LT1258_DEVICE_ADDR_2, 0xDC, read_val);
	// printf("LT1258-2 Read Reg 0x80DC:%x \r\n", read_val[0]);
	// printf("LT1258-2 Config Done!!!!!! \r\n\r\n\r\n\r\n\r\n");
}


static void lt1258Config3(void)
{
	uint8_t read_val[1] = {0x00};
	// Switching Bank Address
	lt1258Write(LT1258_DEVICE_ADDR_3, 0xff, 0x81);
	// printf("LT1258-3 INEN Software Configuration \r\n");
	/***************************INEN Software Configuration********************/
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x60, read_val);
	// printf("LT1258-3 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x60, read_val[0] | BIT2); // 0x8160[2] = 1
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x60, read_val);
	// printf("LT1258-3 Write Done --- Read Reg 0x8160:%x \r\n\r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x63, read_val);
	// printf("LT1258-3 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x63, read_val[0] & (~BIT5));
	// lt1258Write(LT1258_DEVICE_ADDR_3, 0x63, read_val[0] & (~BIT4));//0x8163[5:4] = 2b’00
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x63, read_val[0] | BIT4); // 0x8163[5:4] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x63, read_val);
	// printf("LT1258-3 Write Done --- Read Reg 0x8163:%x \r\n", read_val[0]);

	// printf("LT1258-3 SEL0 and SEL1 Software Configuration \r\n\r\n\r\n");
	/*********************SEL0 and SEL1 Software Configuration******************/
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x60, read_val);
	// printf("LT1258-3 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x60, read_val[0] | BIT0); // 0x8160[0] = 1
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x60, read_val);
	// printf("LT1258-3 Write Done --- Read Reg 0x8160:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x63, read_val);
	// printf("LT1258-3 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x63, read_val[0] & (~BIT1));
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x63, read_val[0] | BIT0); // 0x8163[1:0] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x63, read_val);
	// printf("LT1258-3 Write Done --- Read Reg 0x8163:%x \r\n", read_val[0]);
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x60, read_val);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x60, read_val[0] | BIT1); // 0x8160[1] = 1
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x63, read_val);
	// printf("LT1258-3 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x63, read_val[0] & (~BIT3));
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x63, read_val[0] | BIT2); // 0x8163[3:2] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x63, read_val);
	// printf("LT1258-3 Write Done --- Read Reg 0x8163:%x \r\n\r\n\r\n", read_val[0]);

	/****************Equalizer Software Configuration*****************/
	// printf("LT1258-3 Equalizer Software Configuration \r\n\r\n");
	// Switching Bank Address
	lt1258Write(LT1258_DEVICE_ADDR_3, 0xff, 0x80);

	/************SRXB0**********/
	// printf("LT1258-3 SRXB0 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x38, read_val);
	// printf("LT1258-3 Read Reg 0x8038:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x38, 0xc0); // 0x8038=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x38, read_val);
	// printf("LT1258-3 Read Reg 0x8038:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x39, read_val);
	// printf("LT1258-3 Read Reg 0x8039:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x39, 0x70); // 0x8039=0x70
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x39, read_val);
	// printf("LT1258-3 Read Reg 0x8039:%x \r\n", read_val[0]);

	/************SRXB1**********/
	// printf("LT1258-3 SRXB1 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x78, read_val);
	// printf("LT1258-3 Read Reg 0x8078:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x78, 0xc0); // 0x8078=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x78, read_val);
	// printf("LT1258-3 Read Reg 0x8078:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x79, read_val);
	// printf("LT1258-3 Read Reg 0x8079:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x79, 0x70); // 0x8079=0x70
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x79, read_val);
	// printf("LT1258-3 Read Reg 0x8079:%x \r\n", read_val[0]);

	/************DRXA0**********/
	// printf("LT1258-3 DRXA0 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x98, read_val);
	// printf("LT1258-3 Read Reg 0x8098:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x98, 0xc0); // 0x8098=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x98, read_val);
	// printf("LT1258-3 Read Reg 0x8098:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x99, read_val);
	// printf("LT1258-3 Read Reg 0x8099:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x99, 0x70); // 0x8099=0x70
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x99, read_val);
	// printf("LT1258-3 Read Reg 0x8099:%x \r\n", read_val[0]);

	/************DRXA1**********/
	// printf("LT1258-3 DRXA1 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xD8, read_val);
	// printf("LT1258-3 Read Reg 0x80D8:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0xD8, 0xc0); // 0x80D8=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xD8, read_val);
	// printf("LT1258-3 Read Reg 0x80D8:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xD9, read_val);
	// printf("LT1258-3 Read Reg 0x80D9:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0xD9, 0x70); // 0x80D9=0x70
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xD9, read_val);
	// printf("LT1258-3 Read Reg 0x80D9:%x \r\n", read_val[0]);

	/****************De-Emphasis and Output Swing Software Configuration*****************/
	// printf("LT1258-3 De-Emphasis and Output Swing Software Configuration \r\n\r\n");

	/************DTXB0**********/
	// printf("LT1258-3 DTXB0 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x3A, read_val);
	// printf("LT1258-3 Read Reg 0x803A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x3A, 0x03); // 0x803A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x3A, read_val);
	// printf("LT1258-3 Read Reg 0x803A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x3B, read_val);
	// printf("LT1258-3 Read Reg 0x803B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x3B, 0x70); // 0x803B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x3B, read_val);
	// printf("LT1258-3 Read Reg 0x803B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_3, 0x3C, read_val);
	// printf("LT1258-3 Read Reg 0x803C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x3C, 0x20); // 0x803C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x3C, read_val);
	// printf("LT1258-3 Read Reg 0x803C:%x \r\n", read_val[0]);

	/************DTXB1**********/
	// printf("LT1258-3 DTXB1 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x7A, read_val);
	// printf("LT1258-3 Read Reg 0x803A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x7A, 0x03); // 0x807A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x7A, read_val);
	// printf("LT1258-3 Read Reg 0x807A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x7B, read_val);
	// printf("LT1258-3 Read Reg 0x807B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x7B, 0x70); // 0x807B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x7B, read_val);
	// printf("LT1258-3 Read Reg 0x807B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_3, 0x7C, read_val);
	// printf("LT1258-3 Read Reg 0x807C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x7C, 0x20); // 0x807C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x7C, read_val);
	// printf("LT1258-3 Read Reg 0x807C:%x \r\n", read_val[0]);

	/************STXA0**********/
	// printf("LT1258-3 STXA0 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x9A, read_val);
	// printf("LT1258-3 Read Reg 0x809A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x9A, 0x03); // 0x809A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x9A, read_val);
	// printf("LT1258-3 Read Reg 0x809A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x9B, read_val);
	// printf("LT1258-3 Read Reg 0x809B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x9B, 0x70); // 0x809B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x9B, read_val);
	// printf("LT1258-3 Read Reg 0x809B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_3, 0x9C, read_val);
	// printf("LT1258-3 Read Reg 0x809C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0x9C, 0x20); // 0x809C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_3, 0x9C, read_val);
	// printf("LT1258-3 Read Reg 0x809C:%x \r\n", read_val[0]);

	/************STXA1**********/
	// printf("LT1258-3 STXA1 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xDA, read_val);
	// printf("LT1258-3 Read Reg 0x80DA:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0xDA, 0x03); // 0x80DA=0x03
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xDA, read_val);
	// printf("LT1258-3 Read Reg 0x80DA:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xDB, read_val);
	// printf("LT1258-3 Read Reg 0x80DB:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0xDB, 0x70); // 0x80DB=0x70
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xDB, read_val);
	// printf("LT1258-3 Read Reg 0x80DB:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_3, 0xDC, read_val);
	// printf("LT1258-3 Read Reg 0x80DC:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_3, 0xDC, 0x20); // 0x80DC=0x20
	lt1258Read(LT1258_DEVICE_ADDR_3, 0xDC, read_val);
	// printf("LT1258-3 Read Reg 0x80DC:%x \r\n", read_val[0]);
	// printf("LT1258-3 Config Done!!!!!! \r\n\r\n\r\n\r\n\r\n");
}


static void lt1258Config4(void)
{
	uint8_t read_val[1] = {0x00};
	// Switching Bank Address
	lt1258Write(LT1258_DEVICE_ADDR_4, 0xff, 0x81);
	// printf("LT1258-4 INEN Software Configuration \r\n");
	/***************************INEN Software Configuration********************/
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x60, read_val);
	// printf("LT1258-4 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x60, read_val[0] | BIT2); // 0x8160[2] = 1
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x60, read_val);
	// printf("LT1258-4 Write Done --- Read Reg 0x8160:%x \r\n\r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x63, read_val);
	// printf("LT1258-4 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x63, read_val[0] & (~BIT5));
	// lt1258Write(LT1258_DEVICE_ADDR_4, 0x63, read_val[0] & (~BIT4));//0x8163[5:4] = 2b’00
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x63, read_val[0] | BIT4); // 0x8163[5:4] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x63, read_val);
	// printf("LT1258-4 Write Done --- Read Reg 0x8163:%x \r\n", read_val[0]);

	// printf("LT1258-4 SEL0 and SEL1 Software Configuration \r\n\r\n\r\n");
	/*********************SEL0 and SEL1 Software Configuration******************/
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x60, read_val);
	// printf("LT1258-4 Read Reg 0x8160:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x60, read_val[0] | BIT0); // 0x8160[0] = 1
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x60, read_val);
	// printf("LT1258-4 Write Done --- Read Reg 0x8160:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x63, read_val);
	// printf("LT1258-4 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x63, read_val[0] & (~BIT1));
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x63, read_val[0] | BIT0); // 0x8163[1:0] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x63, read_val);
	// printf("LT1258-4 Write Done --- Read Reg 0x8163:%x \r\n", read_val[0]);
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x60, read_val);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x60, read_val[0] | BIT1); // 0x8160[1] = 1
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x63, read_val);
	// printf("LT1258-4 Read Reg 0x8163:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x63, read_val[0] & (~BIT3));
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x63, read_val[0] | BIT2); // 0x8163[3:2] = 2b’01
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x63, read_val);
	// printf("LT1258-4 Write Done --- Read Reg 0x8163:%x \r\n\r\n\r\n", read_val[0]);

	/****************Equalizer Software Configuration*****************/
	// printf("LT1258-4 Equalizer Software Configuration \r\n\r\n");
	// Switching Bank Address
	lt1258Write(LT1258_DEVICE_ADDR_4, 0xff, 0x80);

	/************SRXB0**********/
	// printf("LT1258-4 SRXB0 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x38, read_val);
	// printf("LT1258-4 Read Reg 0x8038:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x38, 0xc0); // 0x8038=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x38, read_val);
	// printf("LT1258-4 Read Reg 0x8038:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x39, read_val);
	// printf("LT1258-4 Read Reg 0x8039:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x39, 0x70); // 0x8039=0x70
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x39, read_val);
	// printf("LT1258-4 Read Reg 0x8039:%x \r\n", read_val[0]);

	/************SRXB1**********/
	// printf("LT1258-4 SRXB1 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x78, read_val);
	// printf("LT1258-4 Read Reg 0x8078:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x78, 0xc0); // 0x8078=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x78, read_val);
	// printf("LT1258-4 Read Reg 0x8078:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x79, read_val);
	// printf("LT1258-4 Read Reg 0x8079:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x79, 0x70); // 0x8079=0x70
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x79, read_val);
	// printf("LT1258-4 Read Reg 0x8079:%x \r\n", read_val[0]);

	/************DRXA0**********/
	// printf("LT1258-4 DRXA0 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x98, read_val);
	// printf("LT1258-4 Read Reg 0x8098:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x98, 0xc0); // 0x8098=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x98, read_val);
	// printf("LT1258-4 Read Reg 0x8098:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x99, read_val);
	// printf("LT1258-4 Read Reg 0x8099:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x99, 0x70); // 0x8099=0x70
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x99, read_val);
	// printf("LT1258-4 Read Reg 0x8099:%x \r\n", read_val[0]);

	/************DRXA1**********/
	// printf("LT1258-4 DRXA1 EQ Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xD8, read_val);
	// printf("LT1258-4 Read Reg 0x80D8:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0xD8, 0xc0); // 0x80D8=0xc0   11000000
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xD8, read_val);
	// printf("LT1258-4 Read Reg 0x80D8:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xD9, read_val);
	// printf("LT1258-4 Read Reg 0x80D9:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0xD9, 0x70); // 0x80D9=0x70
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xD9, read_val);
	// printf("LT1258-4 Read Reg 0x80D9:%x \r\n", read_val[0]);

	/****************De-Emphasis and Output Swing Software Configuration*****************/
	// printf("LT1258-4 De-Emphasis and Output Swing Software Configuration \r\n\r\n");

	/************DTXB0**********/
	// printf("LT1258-4 DTXB0 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x3A, read_val);
	// printf("LT1258-4 Read Reg 0x803A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x3A, 0x03); // 0x803A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x3A, read_val);
	// printf("LT1258-4 Read Reg 0x803A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x3B, read_val);
	// printf("LT1258-4 Read Reg 0x803B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x3B, 0x70); // 0x803B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x3B, read_val);
	// printf("LT1258-4 Read Reg 0x803B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_4, 0x3C, read_val);
	// printf("LT1258-4 Read Reg 0x803C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x3C, 0x20); // 0x803C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x3C, read_val);
	// printf("LT1258-4 Read Reg 0x803C:%x \r\n", read_val[0]);

	/************DTXB1**********/
	// printf("LT1258-4 DTXB1 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x7A, read_val);
	// printf("LT1258-4 Read Reg 0x803A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x7A, 0x03); // 0x807A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x7A, read_val);
	// printf("LT1258-4 Read Reg 0x807A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x7B, read_val);
	// printf("LT1258-4 Read Reg 0x807B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x7B, 0x70); // 0x807B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x7B, read_val);
	// printf("LT1258-4 Read Reg 0x807B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_4, 0x7C, read_val);
	// printf("LT1258-4 Read Reg 0x807C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x7C, 0x20); // 0x807C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x7C, read_val);
	// printf("LT1258-4 Read Reg 0x807C:%x \r\n", read_val[0]);

	/************STXA0**********/
	// printf("LT1258-4 STXA0 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x9A, read_val);
	// printf("LT1258-4 Read Reg 0x809A:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x9A, 0x03); // 0x809A=0x03
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x9A, read_val);
	// printf("LT1258-4 Read Reg 0x809A:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x9B, read_val);
	// printf("LT1258-4 Read Reg 0x809B:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x9B, 0x70); // 0x809B=0x70
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x9B, read_val);
	// printf("LT1258-4 Read Reg 0x809B:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_4, 0x9C, read_val);
	// printf("LT1258-4 Read Reg 0x809C:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0x9C, 0x20); // 0x809C=0x20
	lt1258Read(LT1258_DEVICE_ADDR_4, 0x9C, read_val);
	// printf("LT1258-4 Read Reg 0x809C:%x \r\n", read_val[0]);

	/************STXA1**********/
	// printf("LT1258-4 STXA1 De-Emphasis and Output Swing Configuration \r\n");
	// Software enable
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xDA, read_val);
	// printf("LT1258-4 Read Reg 0x80DA:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0xDA, 0x03); // 0x80DA=0x03
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xDA, read_val);
	// printf("LT1258-4 Read Reg 0x80DA:%x \r\n", read_val[0]);
	// Software Configuration
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xDB, read_val);
	// printf("LT1258-4 Read Reg 0x80DB:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0xDB, 0x70); // 0x80DB=0x70
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xDB, read_val);
	// printf("LT1258-4 Read Reg 0x80DB:%x \r\n", read_val[0]);

	lt1258Read(LT1258_DEVICE_ADDR_4, 0xDC, read_val);
	// printf("LT1258-4 Read Reg 0x80DC:%x \r\n", read_val[0]);
	lt1258Write(LT1258_DEVICE_ADDR_4, 0xDC, 0x20); // 0x80DC=0x20
	lt1258Read(LT1258_DEVICE_ADDR_4, 0xDC, read_val);
	// printf("LT1258-4 Read Reg 0x80DC:%x \r\n", read_val[0]);
	// printf("LT1258-4 Config Done!!!!!! \r\n\r\n\r\n\r\n\r\n");
}


/****************************Test Code**************************/
/* void configureInen(uint8_t device_addr) {
    uint8_t read_val[1] = {0x00};
    printf("LT1258-%d INEN Software Configuration \r\n", device_addr - 0x50 + 1);
    
    // Software enable
    lt1258Read(device_addr, 0x60, read_val);
    printRegValue(device_addr, 0x60, "Read Reg 0x8160:");
    lt1258Write(device_addr, 0x60, read_val[0] | BIT2);
    lt1258Read(device_addr, 0x60, read_val);
    printRegValue(device_addr, 0x60, "Write Done --- Read Reg 0x8160:");

    // Software Configuration
    lt1258Read(device_addr, 0x63, read_val);
    printRegValue(device_addr, 0x63, "Read Reg 0x8163:");
    lt1258Write(device_addr, 0x63, read_val[0] & (~BIT5));
    lt1258Write(device_addr, 0x63, read_val[0] | BIT4);
    lt1258Read(device_addr, 0x63, read_val);
    printRegValue(device_addr, 0x63, "Write Done --- Read Reg 0x8163:");
}

void configureSel0Sel1(uint8_t device_addr) {
    uint8_t read_val[1] = {0x00};
    printf("LT1258-%d SEL0 and SEL1 Software Configuration \r\n", device_addr - 0x50 + 1);
    
    // Software enable
    lt1258Read(device_addr, 0x60, read_val);
    printRegValue(device_addr, 0x60, "Read Reg 0x8160:");
    lt1258Write(device_addr, 0x60, read_val[0] | BIT0);
    lt1258Read(device_addr, 0x60, read_val);
    printRegValue(device_addr, 0x60, "Write Done --- Read Reg 0x8160:");

    // Software Configuration
    lt1258Read(device_addr, 0x63, read_val);
    printRegValue(device_addr, 0x63, "Read Reg 0x8163:");
    lt1258Write(device_addr, 0x63, read_val[0] & (~BIT1));
    lt1258Write(device_addr, 0x63, read_val[0] | BIT0);
    lt1258Read(device_addr, 0x63, read_val);
    printRegValue(device_addr, 0x63, "Write Done --- Read Reg 0x8163:");

    // Software enable
    lt1258Read(device_addr, 0x60, read_val);
    printRegValue(device_addr, 0x60, "Read Reg 0x8160:");
    lt1258Write(device_addr, 0x60, read_val[0] | BIT1);
    lt1258Read(device_addr, 0x60, read_val);
    printRegValue(device_addr, 0x60, "Write Done --- Read Reg 0x8160:");

    // Software Configuration
    lt1258Read(device_addr, 0x63, read_val);
    printRegValue(device_addr, 0x63, "Read Reg 0x8163:");
    lt1258Write(device_addr, 0x63, read_val[0] & (~BIT3));
    lt1258Write(device_addr, 0x63, read_val[0] | BIT2);
    lt1258Read(device_addr, 0x63, read_val);
    printRegValue(device_addr, 0x63, "Write Done --- Read Reg 0x8163:");
}

void configureEqualizer(uint8_t device_addr) {
    uint8_t read_val[1] = {0x00};
    printf("LT1258-%d Equalizer Software Configuration \r\n\r\n", device_addr - 0x50 + 1);

    // Switching Bank Address
    lt1258Write(device_addr, 0xff, 0x80);

    // SRXB0
    printf("LT1258-%d SRXB0 EQ Configuration \r\n", device_addr - 0x50 + 1);
    lt1258Read(device_addr, 0x38, read_val);
    printRegValue(device_addr, 0x38, "Read Reg 0x8038:");
    lt1258Write(device_addr, 0x38, 0xc0);
    lt1258Read(device_addr, 0x38, read_val);
    printRegValue(device_addr, 0x38, "Read Reg 0x8038:");

    lt1258Read(device_addr, 0x39, read_val);
    printRegValue(device_addr, 0x39, "Read Reg 0x8039:");
    lt1258Write(device_addr, 0x39, 0x70);
    lt1258Read(device_addr, 0x39, read_val);
    printRegValue(device_addr, 0x39, "Read Reg 0x8039:");

    // SRXB1
    printf("LT1258-%d SRXB1 EQ Configuration \r\n", device_addr - 0x50 + 1);
    lt1258Read(device_addr, 0x78, read_val);
    printRegValue(device_addr, 0x78, "Read Reg 0x8078:");
    lt1258Write(device_addr, 0x78, 0xc0);
    lt1258Read(device_addr, 0x78, read_val);
    printRegValue(device_addr, 0x78, "Read Reg 0x8078:");

    lt1258Read(device_addr, 0x79, read_val);
    printRegValue(device_addr, 0x79, "Read Reg 0x8079:");
    lt1258Write(device_addr, 0x79, 0x70);
    lt1258Read(device_addr, 0x79, read_val);
    printRegValue(device_addr, 0x79, "Read Reg 0x8079:");

    // DRXA0
    printf("LT1258-%d DRXA0 EQ Configuration \r\n", device_addr - 0x50 + 1);
    lt1258Read(device_addr, 0x98, read_val);
    printRegValue(device_addr, 0x98, "Read Reg 0x8098:");
    lt1258Write(device_addr, 0x98, 0xc0);
    lt1258Read(device_addr, 0x98, read_val);
    printRegValue(device_addr, 0x98, "Read Reg 0x8098:");

    lt1258Read(device_addr, 0x99, read_val);
    printRegValue(device_addr, 0x99, "Read Reg 0x8099:");
    lt1258Write(device_addr, 0x99, 0x70);
    lt1258Read(device_addr, 0x99, read_val);
    printRegValue(device_addr, 0x99, "Read Reg 0x8099:");

    // DRXA1
    printf("LT1258-%d DRXA1 EQ Configuration \r\n", device_addr - 0x50 + 1);
    lt1258Read(device_addr, 0xD8, read_val);
    printRegValue(device_addr, 0xD8, "Read Reg 0x80D8:");
    lt1258Write(device_addr, 0xD8, 0xc0);
    lt1258Read(device_addr, 0xD8, read_val);
    printRegValue(device_addr, 0xD8, "Read Reg 0x80D8:");

    lt1258Read(device_addr, 0xD9, read_val);
    printRegValue(device_addr, 0xD9, "Read Reg 0x80D9:");
    lt1258Write(device_addr, 0xD9, 0x70);
    lt1258Read(device_addr, 0xD9, read_val);
    printRegValue(device_addr, 0xD9, "Read Reg 0x80D9:");
}

void configureDeEmphasisAndOutputSwing(uint8_t device_addr) {
    uint8_t read_val[1] = {0x00};
    printf("LT1258-%d De-Emphasis and Output Swing Software Configuration \r\n\r\n", device_addr - 0x50 + 1);

    // DTXB0
    printf("LT1258-%d DTXB0 De-Emphasis and Output Swing Configuration \r\n", device_addr - 0x50 + 1);
    lt1258Read(device_addr, 0x3A, read_val);
    printRegValue(device_addr, 0x3A, "Read Reg 0x803A:");
    lt1258Write(device_addr, 0x3A, 0x03);
    lt1258Read(device_addr, 0x3A, read_val);
    printRegValue(device_addr, 0x3A, "Read Reg 0x803A:");

    lt1258Read(device_addr, 0x3B, read_val);
    printRegValue(device_addr, 0x3B, "Read Reg 0x803B:");
    lt1258Write(device_addr, 0x3B, 0x70);
    lt1258Read(device_addr, 0x3B, read_val);
    printRegValue(device_addr, 0x3B, "Read Reg 0x803B:");

    lt1258Read(device_addr, 0x3C, read_val);
    printRegValue(device_addr, 0x3C, "Read Reg 0x803C:");
    lt1258Write(device_addr, 0x3C, 0x20);
    lt1258Read(device_addr, 0x3C, read_val);
    printRegValue(device_addr, 0x3C, "Read Reg 0x803C:");

    // DTXB1
    printf("LT1258-%d DTXB1 De-Emphasis and Output Swing Configuration \r\n", device_addr - 0x50 + 1);
    lt1258Read(device_addr, 0x7A, read_val);
    printRegValue(device_addr, 0x7A, "Read Reg 0x807A:");
    lt1258Write(device_addr, 0x7A, 0x03);
    lt1258Read(device_addr, 0x7A, read_val);
    printRegValue(device_addr, 0x7A, "Read Reg 0x807A:");

    lt1258Read(device_addr, 0x7B, read_val);
    printRegValue(device_addr, 0x7B, "Read Reg 0x807B:");
    lt1258Write(device_addr, 0x7B, 0x70);
    lt1258Read(device_addr, 0x7B, read_val);
    printRegValue(device_addr, 0x7B, "Read Reg 0x807B:");

    lt1258Read(device_addr, 0x7C, read_val);
    printRegValue(device_addr, 0x7C, "Read Reg 0x807C:");
    lt1258Write(device_addr, 0x7C, 0x20);
    lt1258Read(device_addr, 0x7C, read_val);
    printRegValue(device_addr, 0x7C, "Read Reg 0x807C:");

    // STXA0
    printf("LT1258-%d STXA0 De-Emphasis and Output Swing Configuration \r\n", device_addr - 0x50 + 1);
    lt1258Read(device_addr, 0x9A, read_val);
    printRegValue(device_addr, 0x9A, "Read Reg 0x809A:");
    lt1258Write(device_addr, 0x9A, 0x03);
    lt1258Read(device_addr, 0x9A, read_val);
    printRegValue(device_addr, 0x9A, "Read Reg 0x809A:");

    lt1258Read(device_addr, 0x9B, read_val);
    printRegValue(device_addr, 0x9B, "Read Reg 0x809B:");
    lt1258Write(device_addr, 0x9B, 0x70);
    lt1258Read(device_addr, 0x9B, read_val);
    printRegValue(device_addr, 0x9B, "Read Reg 0x809B:");

    lt1258Read(device_addr, 0x9C, read_val);
    printRegValue(device_addr, 0x9C, "Read Reg 0x809C:");
    lt1258Write(device_addr, 0x9C, 0x20);
    lt1258Read(device_addr, 0x9C, read_val);
    printRegValue(device_addr, 0x9C, "Read Reg 0x809C:");

    // STXA1
    printf("LT1258-%d STXA1 De-Emphasis and Output Swing Configuration \r\n", device_addr - 0x50 + 1);
    lt1258Read(device_addr, 0xDA, read_val);
    printRegValue(device_addr, 0xDA, "Read Reg 0x80DA:");
    lt1258Write(device_addr, 0xDA, 0x03);
    lt1258Read(device_addr, 0xDA, read_val);
    printRegValue(device_addr, 0xDA, "Read Reg 0x80DA:");

    lt1258Read(device_addr, 0xDB, read_val);
    printRegValue(device_addr, 0xDB, "Read Reg 0x80DB:");
    lt1258Write(device_addr, 0xDB, 0x70);
    lt1258Read(device_addr, 0xDB, read_val);
    printRegValue(device_addr, 0xDB, "Read Reg 0x80DB:");

    lt1258Read(device_addr, 0xDC, read_val);
    printRegValue(device_addr, 0xDC, "Read Reg 0x80DC:");
    lt1258Write(device_addr, 0xDC, 0x20);
    lt1258Read(device_addr, 0xDC, read_val);
    printRegValue(device_addr, 0xDC, "Read Reg 0x80DC:");
}

void printRegValue(uint8_t device_addr, uint8_t reg, const char* message) {
    uint8_t read_val[1] = {0x00};
    lt1258Read(device_addr, reg, read_val);
    printf("LT1258-%d %s:%x \r\n", device_addr - 0x50 + 1, message, read_val[0]);
}

void configureAllChips() {
    uint8_t device_addrs[] = {LT1258_DEVICE_ADDR_1, LT1258_DEVICE_ADDR_2, LT1258_DEVICE_ADDR_3, LT1258_DEVICE_ADDR_4};
    int num_devices = sizeof(device_addrs) / sizeof(device_addrs[0]);

    for (int i = 0; i < num_devices; i++) {
        uint8_t device_addr = device_addrs[i];
        
        // Switching Bank Address
        lt1258Write(device_addr, 0xff, 0x81);

        configureInen(device_addr);
        configureSel0Sel1(device_addr);
        configureEqualizer(device_addr);
        configureDeEmphasisAndOutputSwing(device_addr);

        printf("LT1258-%d Config Done!!!!!! \r\n\r\n\r\n\r\n\r\n", i + 1);
    }
} */
