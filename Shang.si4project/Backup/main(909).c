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

uint16_t Ch_los=0x00;
uint16_t Ch_retimer=0X0F;
uint8_t LED_FS[4]={LED_F2,LED_F3,LED_F0,LED_F1};//F2 灯L3     F3 灯L4 F0 灯L5     F1 灯L6   
/*!
    \brief    LED闪灯任务
    \param[in]  none
    \param[out] none
    \retval     none
*/
#define LED_TASK_CYCLE (SYS_CLK_TICK / 4) //1s/4 = 0.25s
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
#define TEST_TASK_CYCLE (SYS_CLK_TICK *1) //1s
static void testTask(void)
{
	uint8_t i,val1;
	static volatile uint8_t reg = 0x62;
	char temp[3];
	uint8_t id[3];
	//char str[8];
	uint16_t val;
	static uint64_t lastTick = 0;
	//static uint8_t overFlowFlag = 0;
	uint8_t Ureport[32]={0};
	uint8_t prove=0;
  uint16_t cnt = 0;
	uint8_t blnum=0;

	char Time[2]={0};
	char D1_num[2];
	static uint8_t LED_SD=0;
	//char *itoa(int value, char *string, int);
	
	if(systick-lastTick > TEST_TASK_CYCLE)
	{		
		sec=systick/1000;
		lastTick = systick;
		//测试 
		val1 = read_los();
		Dec2Bin(val1);
		//itoa(val1, str, 2);//整数转换为二进制数
		//printf("read_los SFP_LOS: %d\r\n", val1);
		
		Time[0]=(sec>>8)&0XFF;
		Time[1]=sec&0XFF;
		Ureport[cnt++] = 0x7E;//帧头
		Ureport[cnt++] = 0xF1;//类型F1上端机
		Ureport[cnt++] = Time[0];
		Ureport[cnt++] = Time[1];//上电时间
		Ureport[cnt++] = val1&0xff;//SPF状态
			LED_SD++;		
		for(i = 0; i < GN1406_CHIP_NUM; i++)
		{//GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5
			//if(STA_ERR == gn1406Read(i, GN1406_STA_REG, &val) )
			D1_num[0]=0;
			D1_num[1]=0;
			if(STA_ERR == gn1406Read(i, reg, &val) )
			{
				printf("gn1406Read fail\r\n");
			}else
			{				
				//printf("gn1406Read GN1406_STA_REG:0x%04x D1 0x%04x\r\n", val, GN1406_ADDR0);
				D1_num[0] = (val>>8)&0xff;
				D1_num[1] = val&0xff;
				if (((val >> 1) & 0x0F)== Ch_retimer)
				{
					
					fLedCtl(LED_FS[i], LED_RED);//L3红
					
				}
				else if((((val >> 1) & 0x0F)== Ch_los) && (((val >> 5) & 0x0F)==Ch_los))
				{
					fLedCtl(LED_FS[i], LED_GREEN);//chang lv
				}
				else
				{
					LEDGreen_SS(LED_FS[i],LED_SD);
				}
			
			}
			Ureport[cnt++] = D1_num[0];
		    Ureport[cnt++] = D1_num[1];//D1
			
		}
		//测试 SPI flash ID读取
		if(STA_ERR == spiFlashReadID(SPI_FLASH_CS, id) )
		{
			printf("spiFlashReadID fail\r\n");
		}else
		{
			//printf("spiFlashReadID:0x%02x 0x%02x 0x%02x\r\n", id[0], id[1], id[2]);
		}
		
		//测试温度传感器
		if(STA_ERR == ReadTemp(temp) )
		{
			printf("ReadCB2991Temp fail\r\n");
		}else
		{
			//printf("ReadCB2991Temp:%d %d %d\r\n", temp[0], temp[1], temp[2]);
		}
		
		Ureport[cnt++] = temp[0];//TEMP1
		Ureport[cnt++] = temp[1];//TEMP2
		Ureport[cnt++] = temp[2];//TEMP3
		blnum=31-cnt;
		for(i=0;i<blnum;i++)
		{
			Ureport[cnt++]=0x00;
		}
		
		for(i=1;i<cnt;i++)
       {
           prove^=Ureport[i];
		   prove = prove&0xff;
        }
	//cnt+= sprintf(Ureport+cnt,"*%02X\r\n",prove);
		Ureport[cnt++] = prove;    
		serialSend(USART0, Ureport, cnt);	
		//422测试
		serialSend(USART2, Ureport, cnt);
	}
	
}
void BBXX_ouput(void)
{
		printf("orc2103 upTerminalARMV%d.%d,%d,%s,%s\r\n", VERSION_A, VERSION_CD, VERSION_EFG,__DATE__, __TIME__);

}/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
	
	systick_config();
	rcu_config();
	bspGpioInit();//
	other_io_init();//离散IO
	uartInit();
	BBXX_ouput();
	spiFlashInit(SPI_FLASH_CS);
	
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
	
	while (1){
		ledTask();
		testTask();

	}
}
