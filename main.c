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
#include "GN1406.h"
#include "cb2991.h"
/* LT1258已由CLRD125C替代；如需回退对比，再恢复LT1258.h和main中的初始化代码。 */
/* #include "LT1258.h" */
#include "clrd125c_v1.h"

// 定义LED任务与测试任务的周期
#define LED_TASK_CYCLE (SYS_CLK_TICK / 4)  // 1s/4 = 0.25s
#define TEST_TASK_CYCLE (SYS_CLK_TICK * 1) // 1s
#define LINK_STATUS_TASK_CYCLE (SYS_CLK_TICK * 5) // 5s
#define MAIN_LOOP_DELAY_MS 10U

/*!
    \brief    LED闪灯任务
    \param[in]  none
    \param[out] none
    \retval     none
*/

static void printBinary(const char *label, uint8_t value, uint8_t width)
{
    char bits[9];
    uint8_t i;

    if (width > 8U)
    {
        width = 8U;
    }
    /* 从最高有效位开始转换，保证前导0也能打印出来。 */
    for (i = 0U; i < width; i++)
    {
        bits[i] = (0U != (value & (uint8_t)(1U << (width - 1U - i)))) ?
                  '1' : '0';
    }
    bits[width] = '\0';
    printf("%s%s\r\n", label, bits);
}

static void appendLinkStatus(uint8_t losValue,
                             uint8_t mask,
                             FLED_NUM led,
                             uint8_t *report,
                             uint8_t *count)
{
    uint8_t linkOk;

    /*
     * LOS为低有效：一个端口组内所有LOS位均为0时认为链路正常。
     * 同一判断结果同时用于面板LED和32字节状态上报帧。
     */
    linkOk = ((losValue & mask) == 0U) ? 1U : 0U;
    fLedCtl(led, (0U != linkOk) ? LED_GREEN : LED_RED);

    if ((report != 0) && (count != 0))
    {
        report[(*count)++] = 0x01U;
        report[(*count)++] = (0U != linkOk) ? 0xE1U : 0xE0U;
    }
}

// 通过检查系统节拍定时器的中断标志位来控制LED的状态
static void ledTask(void)
{
    static uint64_t lastTick = 0;

    /* 使用节拍差值而不是阻塞延时，使LED任务和状态任务可并行轮询。 */
    if (systick - lastTick >= LED_TASK_CYCLE)
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
    uint8_t i, val1;
    char temp[3];
    uint16_t vol[4];
    // uint8_t id[3];

    // uint16_t val;
    static uint64_t lastTick = 0; // 记录系统节拍时间，控制任务的周期性执行
    uint8_t Ureport[32] = {0};
    uint8_t prove = 0;
    uint8_t cnt = 0;

    char Time[2] = {0};
    // char D1_num[2];
    // static uint8_t LED_SD = 0; // 记录LED状态;

    // D1 L3  //D3 L4 //D4 L5 //D5 L6
    // const FLED_NUM LED_FS[4] = {LED_FL3, LED_FL4, LED_FL5, LED_FL6};

    if (systick - lastTick >= TEST_TASK_CYCLE)
    {
        sec = systick / 1000;
        lastTick = systick; // 检查差值，更新systick
        // 测试
        val1 = read_los();
        /*
         * 周期调试打印已关闭，避免testTask每秒持续刷串口。
         * 状态采集、LED控制和USART2状态帧上报仍正常执行。
         * 调试离散IO时可临时恢复下面三行。
         */
        /*
        printBinary("The Read_RA val:", read_RA(), 4U);
        printBinary("The Read_GA val:", read_GA(), 6U);
        printBinary("The LOS_val:", val1, 8U);
        */
        // LOS_val = val1;
        Time[0] = (sec >> 8) & 0XFF;
        Time[1] = sec & 0XFF;

        // 构建状态报告帧
        Ureport[cnt++] = 0x7E;        // 帧头
        Ureport[cnt++] = 0xF1;        // 类型F1上端机
        Ureport[cnt++] = Time[0];     // 添加当前时间的高位和低位字节
        Ureport[cnt++] = Time[1];     // 上电时间
        Ureport[cnt++] = val1 & 0xff; // SPF状态

        // LED_SD++; // 更新LED_SD计数器，用于控制LED闪烁

        // 读取GN1406芯片状态
        //  GN1406_CHIP0_D1/GN1406_CHIP1_D3/GN1406_CHIP2_D4/GN1406_CHIP3_D5

        // printf("gn1406Read GN1406_STA_REG:0x%04x D1 0x%04x\r\n", val, GN1406_ADDR_D1);
        // val1 = 0xCF;
        appendLinkStatus(val1, 0x03U, LED_FL3, Ureport, &cnt);
        appendLinkStatus(val1, 0x0CU, LED_FL4, Ureport, &cnt);
        appendLinkStatus(val1, 0x30U, LED_FL5, Ureport, &cnt);
        appendLinkStatus(val1, 0xC0U, LED_FL6, Ureport, &cnt);
        /*  */
        // 测试 SPI flash ID读取
        /* if(STA_ERR == spiFlashReadID(SPI_FLASH_CS, id) )
        {
           printf("spiFlashReadID fail\r\n");
        }
        else
        {
            printf("spiFlashReadID:0x%02x 0x%02x 0x%02x\r\n", id[0], id[1], id[2]);
        } */

        // 测试温度传感器；读取失败时上报明确的无效值
        temp[0] = (char)0xFF;
        temp[1] = (char)0xFF;
        temp[2] = (char)0xFF;
        vol[0] = 0xFFFFU;
        vol[1] = 0xFFFFU;
        vol[2] = 0xFFFFU;
        vol[3] = 0xFFFFU;
        if (STA_ERR == ReadTemp(temp))
        {
            /* 周期错误打印关闭；无效温度仍以0xFF填入状态帧。 */
        }
        else
        {
            /* printf("\r\nReadCB2991Temp:%d %d %d\r\n",
                      temp[0], temp[1], temp[2]); */
        }
        if (STA_ERR == ReadVol(vol))
        {
            /* 周期错误打印关闭；vol保持0xFFFF无效值。 */
        }
        else
        {
            /* printf("ReadCB2991vol:%dmv %dmv %dmv %dmv\r\n",
                      vol[0], vol[1], vol[2], vol[3]); */
        }

        // 增加了温度和电压功能，将其添加到状态报告帧中
        Ureport[cnt++] = temp[0]; // Q1 TEMP1
        Ureport[cnt++] = temp[1]; // Q2 TEMP2
        Ureport[cnt++] = temp[2]; // CB2291内部温度;IndexZ@2021-9-26
        while (cnt < 31U)
        {
            Ureport[cnt++] = 0x00; // 用0x00填充Ureport直到31字节
        }

        for (i = 1; i < cnt; i++)
        {
            prove = (uint8_t)(prove + Ureport[i]);
        }

        Ureport[cnt++] = prove; // 将校验和添加到Ureport中
                                // serialSend(USART0, Ureport, cnt); // RS232调试;IndexZ@2021-9-26
                                //  422上报;
        serialSend(USART2, Ureport, cnt);
    }
}

/*
 * 上电初始化完成后只打印一次调试状态。
 * 后续业务循环仍会周期采集LOS/温度并上报USART2状态帧，但不再循环printf刷屏。
 */
static void printStartupStatusOnce(void)
{
    char temp[3];
    uint16_t vol[4];
    uint8_t losValue;

    losValue = read_los();
    printBinary("The Read_RA val:", read_RA(), 4U);
    printBinary("The Read_GA val:", read_GA(), 6U);
    printBinary("The LOS_val:", losValue, 8U);

    if (STA_ERR == ReadTemp(temp))
    {
        printf("ReadCB2991Temp fail\r\n");
    }
    else
    {
        printf("ReadCB2991Temp:%d %d %d\r\n",
               temp[0], temp[1], temp[2]);
    }

    if (STA_ERR == ReadVol(vol))
    {
        printf("ReadCB2991vol fail\r\n");
    }
    else
    {
        printf("ReadCB2991vol:%dmv %dmv %dmv %dmv\r\n",
               vol[0], vol[1], vol[2], vol[3]);
    }

    clrd125cPrintAllLosStatus();
}

/*
 * 每5秒循环打印一次板级LOS和CLRD125C内部通道状态。
 * 板级LOS：bit0~bit7对应SFP0_LOS~SFP7_LOS，通常0=有光/OK，1=LOS。
 * CLRD125C losb：bit0~bit7对应CH0~CH7，通常1=信号OK，0=LOS。
 */
static void linkStatusTask(void)
{
    static uint64_t lastTick = 0;
    uint8_t losValue;

    if (systick - lastTick >= LINK_STATUS_TASK_CYCLE)
    {
        lastTick = systick;

        losValue = read_los();
        printf("\r\n[LINK-STAT] LOS_val[7:0]=%u%u%u%u%u%u%u%u LOS=0x%02X SFP_OK[7:0]=%u%u%u%u%u%u%u%u  note: SFP_OK=1 means board LOS pin is 0\r\n",
               (unsigned int)((losValue >> 7) & 0x01U),
               (unsigned int)((losValue >> 6) & 0x01U),
               (unsigned int)((losValue >> 5) & 0x01U),
               (unsigned int)((losValue >> 4) & 0x01U),
               (unsigned int)((losValue >> 3) & 0x01U),
               (unsigned int)((losValue >> 2) & 0x01U),
               (unsigned int)((losValue >> 1) & 0x01U),
               (unsigned int)((losValue >> 0) & 0x01U),
               losValue,
               (unsigned int)(((losValue >> 7) & 0x01U) == 0U),
               (unsigned int)(((losValue >> 6) & 0x01U) == 0U),
               (unsigned int)(((losValue >> 5) & 0x01U) == 0U),
               (unsigned int)(((losValue >> 4) & 0x01U) == 0U),
               (unsigned int)(((losValue >> 3) & 0x01U) == 0U),
               (unsigned int)(((losValue >> 2) & 0x01U) == 0U),
               (unsigned int)(((losValue >> 1) & 0x01U) == 0U),
               (unsigned int)(((losValue >> 0) & 0x01U) == 0U));
        clrd125cPrintAllLosStatus();
    }
}

void BBXX_ouput(void)
{
    printf("\r\nZX2103 UpperTerminalARMV%d.%02d.%03d,%s,%s\r\n", VERSION_A, VERSION_CD, VERSION_EFG, __DATE__, __TIME__);
}

/*!
    \brief    main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    ERR_STA clrd125Sta;
    ERR_STA cb2991Sta;

    systick_config();
    rcu_config();
    bspGpioInit();
    other_io_init();
    uartInit();
    BBXX_ouput();
    printf("BOOT: basic init done\r\n");

    printf("BOOT: start CLRD125C direct write\r\n");
    clrd125Sta = clrd125cLt1258StyleTest();
    if (STA_ERR == clrd125Sta)
    {
        printf("CLRD125C V1.0 init fail\r\n");
    }

    cb2991Sta = cb2991TempInit();
    if (STA_ERR == cb2991Sta)
    {
        printf("cb2991TempInit fail\r\n");
    }

    printf("Init summary: LT1258=DISABLED, CB2991=%s, CLRD125C=%s(%s)\r\n",
           (STA_OK == cb2991Sta) ? "OK" : "FAIL",
           (STA_OK == clrd125Sta) ? "OK" : "FAIL",
           clrd125cGetConfigModeName());

    printStartupStatusOnce();

    while (1)
    {
        ledTask();
        testTask();
        linkStatusTask();
        delay_1ms(MAIN_LOOP_DELAY_MS);
    }
}
