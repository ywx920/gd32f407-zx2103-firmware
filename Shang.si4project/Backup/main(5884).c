/*!
    \file    main.c
    \brief   with systick
    
*/


#include "gd32f4xx.h"
#include "systick.h"
#include "bsp.h"
#include <stdio.h>
#include <stdlib.h>
#include "Version_MSG.h"
#include "serial.h"

#define LED_TASK_CYCLE      (SYS_CLK_TICK / 4)      //1s/4 = 0.25s
#define TEST_TASK_CYCLE     (SYS_CLK_TICK *1)       //1s

/*!
    \brief    LED闪灯任务
    \param[in]  none
    \param[out] none
    \retval     none
*/

void Dec2Bin(uint8_t n)//整数转换2进制输出
{
    int result=0,k=1,i,temp;
    temp = n;
    while(temp)
    {
        i = temp%2;
        result = k * i + result;
        k = k*10;
        temp = temp / 2;
    }
    //printf("read_los SFP_LOS:%08d\r\n",result);


}
static void ledTask(void)
{
    static uint64_t lastTick = 0;
    
    if(systick-lastTick > LED_TASK_CYCLE)
    {
        lastTick = systick;
        dbgLedCtl(LED_D0, LED_TOG);
    }
}

/*!
    \brief    接口测试任务
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void testTask(void)
{
    uint8_t i,val1;
    char temp[3];
    //uint8_t id[3];

    uint16_t val;
    static uint64_t lastTick = 0;
    uint8_t Ureport[32]={0};
    uint8_t prove=0;
    uint16_t cnt = 0;
    uint8_t blnum=0;

    char Time[2]={0};
    char D1_num[2];
    static uint8_t LED_SD=0;

                             //D1 L3  //D3 L4 //D4 L5 //D5 L6
    const FLED_NUM LED_FS[4]={LED_FL3,LED_FL4,LED_FL5,LED_FL6};
    
    if(systick-lastTick > TEST_TASK_CYCLE)
    {       
        sec=systick/1000;
        lastTick = systick;
        //测试 
        val1 = read_los();
        
        Time[0]=(sec>>8)&0XFF;
        Time[1]=sec&0XFF;
        Ureport[cnt++] = 0x7E;      //帧头
        Ureport[cnt++] = 0xF1;      //类型F1上端机
        Ureport[cnt++] = Time[0];
        Ureport[cnt++] = Time[1];   //上电时间
        Ureport[cnt++] = val1&0xff; //SPF状态
        LED_SD++;
        for(i = 0; i < GN1406_CHIP_NUM; i++)
        {//GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
            D1_num[0]=0;
            D1_num[1]=0;
            if(STA_ERR == gn1406Read(i, GN1406_STA_REG, &val) )
            {
                printf("gn1406Read fail\r\n");
            }
            else
            {               
                //printf("gn1406Read GN1406_STA_REG:0x%04x D1 0x%04x\r\n", val, GN1406_ADDR_D1);
                D1_num[0] = (val>>8)&0xff;
                D1_num[1] = val&0xff;
                if((val & 0x1FF) == 0x1E1)          //PLL锁定;有信号;retimer;
                {
                    LEDGreen_SS(LED_FS[i],LED_SD);  //绿色闪烁;
                }
                else if((val & 0x1FF) == 0x001)     //PLL锁定;有信号;无retimer;
                {
                    fLedCtl(LED_FS[i], LED_GREEN);  //常绿;
                }
                else
                {
                    fLedCtl(LED_FS[i], LED_RED);    //常红;
                }
            }
            Ureport[cnt++] = D1_num[0];
            Ureport[cnt++] = D1_num[1];             //D1
        }
        //测试 SPI flash ID读取
        //if(STA_ERR == spiFlashReadID(SPI_FLASH_CS, id) )
        //{
        //    printf("spiFlashReadID fail\r\n");
        //}
        //else
        //{
            //printf("spiFlashReadID:0x%02x 0x%02x 0x%02x\r\n", id[0], id[1], id[2]);
        //}
        
        //测试温度传感器
        if(STA_ERR == ReadTemp(temp) )
        {
            printf("ReadCB2991Temp fail\r\n");
        }
        else
        {
            //printf("ReadCB2991Temp:%d %d %d\r\n", temp[0], temp[1], temp[2]);
        }
        
        Ureport[cnt++] = temp[0];   //Q1 TEMP1
        Ureport[cnt++] = temp[1];   //Q2 TEMP2
        Ureport[cnt++] = temp[2];   //CB2291内部温度;IndexZ@2021-9-26
        blnum=31-cnt;
        for(i=0;i<blnum;i++)
        {
            Ureport[cnt++]=0x00;
        }
        
        for(i=1;i<cnt;i++)
        {
            prove+= Ureport[i];
            prove = prove&0xff;
        }

        Ureport[cnt++] = prove;    
        serialSend(USART0, Ureport, cnt);   //RS232调试;IndexZ@2021-9-26
        //422上报;
        serialSend(USART2, Ureport, cnt);
    }
    
}
void BBXX_ouput(void)
{
    printf("\r\nZX2103 UpperTerminalARMV%d.%02d.%03d,%s,%s\r\n", VERSION_A, VERSION_CD, VERSION_EFG,__DATE__, __TIME__);
}
/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    systick_config();
    rcu_config();
    bspGpioInit();      //
    other_io_init();    //离散IO
    uartInit();
    BBXX_ouput();
    //spiFlashInit(SPI_FLASH_CS);
    
    if(STA_ERR == gn1406Init())
    {
        printf("gn1406Init fail\r\n");
    }
    if(STA_ERR == cb2991TempInit())
    {
        printf("cb2991TempInit fail\r\n");
    }
    //spi flash测试
    //testSpiFlash();
    
    while (1)
    {
        ledTask();
        testTask();
    }
}

