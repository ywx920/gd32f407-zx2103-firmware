#ifndef SERIAL_H
#define SERIAL_H

#ifdef cplusplus
 extern "C" {
#endif

#include "common.h"

//串口路数
#define SERIAL_NUM 6
//串口编号
#define USART_INDEX0 0
#define USART_INDEX1 1
#define USART_INDEX2 2
#define USART_INDEX3 3
#define USART_INDEX4 4
#define USART_INDEX5 5
//#define USART_INDEX6 6
//#define USART_INDEX7 7
#define USART_INDEXERR (~0)
//接收缓冲区大小
#define UART_RX_BUF_SIZE 16
/*
 * 接收缓冲区数据结果
 */
typedef struct UART_RX_BUF_TYPE_t{
	uint8_t buf[UART_RX_BUF_SIZE];//接收缓冲
	uint8_t rx_cnt;//接收数据的个数
	//ERR_STA valid;//是否已经收到完整的一包数据
}UART_RX_BUF_TYPE;
	 
/*
 * 串口初始化
 * 参数：uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * 备注：当前版本只有USART0/USART2可以用，其他没有调试验证
 */
void serialInit(uint32_t uartn);

/*
 * 串口数据发送
 * 参数：uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * 				uint8_t buf:发送的数据指针
 *				uint8_t len:发送数据的长度
 * 返回:成功发送数据的长度
 * 备注：当前版本只有USART0/USART2可以用，其他没有调试验证
 */
uint8_t serialSend(uint32_t uartn, uint8_t *buf, uint8_t len);

/*
 * 串口数据接收
 * 参数：uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * uint8_t buf:存放数据的指针
 * 返回:成功读取数据的长度
 * 备注：当前版本只有USART0/USART2可以用，其他没有调试验证
 */
uint8_t serialRecv(uint32_t uartn, uint8_t *buf);

/*
 * 串口接收中断
 * 参数：uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * 返回:void
 * 备注
 */
void serial_irq(uint32_t uartn);
	 
#ifdef cplusplus
}
#endif
#endif /* IPMB_H_ */
