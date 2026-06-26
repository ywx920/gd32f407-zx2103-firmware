#include "serial.h"
#include "stdio.h"

/*
 * 串口中断配置
 */
//RCT clk
static IRQn_Type serial_irq_tab[SERIAL_NUM] = \
    {USART0_IRQn,USART1_IRQn,USART2_IRQn,UART3_IRQn,\
    UART4_IRQn,USART5_IRQn};//,UART6_IRQn,UART7_IRQn};

/*
 * 串口相关配置
 */
//
static uint32_t serial_af_set[SERIAL_NUM] = \
    {GPIO_AF_7,GPIO_AF_7,GPIO_AF_7,GPIO_AF_8,\
    GPIO_AF_8,GPIO_AF_8};//,GPIO_AF_8,GPIO_AF_8};
//RCT clk

    //static rcu_periph_enum serial_clk[SERIAL_NUM] = \
    //{RCU_USART0,RCU_USART1,RCU_USART2,RCU_UART3,\
    //RCU_UART4,RCU_USART5};//,RCU_UART6,RCU_UART7};
//TX pin
static uint32_t serial_tx_pin[SERIAL_NUM] = \
    {GPIO_PIN_9,GPIO_PIN_9,GPIO_PIN_10,GPIO_PIN_10,\
    GPIO_PIN_9,GPIO_PIN_9};//,GPIO_PIN_9,GPIO_PIN_9};
//RX pin
static uint32_t serial_rx_pin[SERIAL_NUM] = {GPIO_PIN_10,GPIO_PIN_10,GPIO_PIN_11,GPIO_PIN_11,\
        GPIO_PIN_10,GPIO_PIN_10};//,GPIO_PIN_10,GPIO_PIN_10,};
//TRX port
static uint32_t serial_gpio_port[SERIAL_NUM] = {GPIOA,GPIOA,GPIOC,GPIOC,GPIOA,GPIOA};//,GPIOA,GPIOA};
//TRX port RCU clk
//static rcu_periph_enum serial_gpio_clk[SERIAL_NUM] = {RCU_GPIOA,RCU_GPIOA,RCU_GPIOC,RCU_GPIOC,RCU_GPIOA,RCU_GPIOA};//,RCU_GPIOA,RCU_GPIOA};

/*
 * 接收缓冲区
 */
static UART_RX_BUF_TYPE rxbuf[SERIAL_NUM];

/*
 * 模块地址转模块编号
 * uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * 返回:USART_INDEX0/USART_INDEX1/USART_INDEX2/USART_INDEX3/USART_INDEX4/
 *          USART_INDEX5USART_INDEX6/USART_INDEX7/USART_INDEXERR
 */
static uint32_t uartn_to_index(uint32_t uartn)
{
    switch(uartn)
    {
        case USART0:
            return USART_INDEX0;
        case USART1:
            return USART_INDEX1;
        case USART2:
            return USART_INDEX2;
        case UART3:
            return USART_INDEX3;
        case UART4:
            return USART_INDEX4;
        case USART5:
            return USART_INDEX5;
//      case UART6:
//          return USART_INDEX6;
//      case UART7:
//          return USART_INDEX7;
        default:
            return USART_INDEXERR;
    }
    
}

/*
 * 初始化缓冲区
 * uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * 返回：void
 */
/* static void initUartBuf(uint32_t uartn)
{
    uint32_t index;
    
    index = uartn_to_index(uartn);
    if(USART_INDEXERR == index){
        while(1);
        //return;
    }
    
    rxbuf[index].rx_cnt = 0;
} */
    
/*
 * 向缓冲区中写入数据
 * uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * uint8_t buf:写入缓冲区的数据指针
    * uint8_t len:写入缓冲区的数据长度
 * 返回:STA_OK/STA_ERR
 */
static ERR_STA writeUartBuf(uint32_t uartn, uint8_t *buf, uint8_t len)
{
    uint32_t index, i, offset;
    
    if ((buf == 0) && (len > 0U)){
        return STA_ERR;
    }

    index = uartn_to_index(uartn);
    if(USART_INDEXERR == index){
        return STA_ERR;
    }
    
    if(len > (UART_RX_BUF_SIZE - rxbuf[index].rx_cnt))
    {
        return STA_ERR;
    }
    
    offset = rxbuf[index].rx_cnt;
    for(i = 0; i < len; i++)
    {
        rxbuf[index].buf[offset + i] = buf[i];
    }
    
    rxbuf[index].rx_cnt += len;
    return STA_OK;
}

/*
 * 向缓冲区中写入数据
 * uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * uint8_t buf:存放数据的指针
 * 返回:成功读取数据的长度
 */
static uint8_t readUartBuf(uint32_t uartn, uint8_t *buf)
{
    uint32_t index;
    uint8_t i;
    uint8_t cnt;
    
    if (buf == 0){
        return 0;
    }

    index = uartn_to_index(uartn);
    if(USART_INDEXERR == index){
        return 0;
    }
    
    cnt = rxbuf[index].rx_cnt;
    for(i = 0; i < cnt; i++)
    {
        buf[i] = rxbuf[index].buf[i];
    }
    
    rxbuf[index].rx_cnt = 0;
    return cnt;
}

/*
 * 串口初始化
 * 参数：uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * 备注：当前版本只有USART0/USART2可以用，其他没有调试验证
 */
void serialInit(uint32_t uartn)
{
    uint32_t com_id = 0U;
    com_id = uartn_to_index(uartn);
    if(USART_INDEXERR == com_id){
            while(1);
      //return;
    }
    if((USART_INDEX2 != com_id)
        && (USART_INDEX0 != com_id) )//只有uart2和uart0经过验证，其他没有验证
    {
        while(1);
    }
    /* USART interrupt configuration */
    nvic_irq_enable(serial_irq_tab[com_id], 0, 0);
        
    /* enable GPIO clock 已经在rcu_config中调用*/
    //rcu_periph_clock_enable(serial_gpio_clk[com_id]);

    /* enable USART clock 已经在rcu_config中调用*/
    //rcu_periph_clock_enable(serial_clk[com_id]);
    
    /* connect port to USARTx_Tx */
    gpio_af_set(serial_gpio_port[com_id], serial_af_set[com_id], serial_tx_pin[com_id]);
    /* connect port to USARTx_Rx */
    gpio_af_set(serial_gpio_port[com_id], serial_af_set[com_id], serial_rx_pin[com_id]);
    
    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(serial_gpio_port[com_id], GPIO_MODE_AF, GPIO_PUPD_PULLUP,serial_tx_pin[com_id]);
    gpio_output_options_set(serial_gpio_port[com_id], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,serial_tx_pin[com_id]);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(serial_gpio_port[com_id], GPIO_MODE_AF, GPIO_PUPD_PULLUP,serial_rx_pin[com_id]);
    gpio_output_options_set(serial_gpio_port[com_id], GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,serial_rx_pin[com_id]);

    /* USART configure */
    usart_deinit(uartn);
    usart_baudrate_set(uartn, 115200U);
    usart_receive_config(uartn, USART_RECEIVE_ENABLE);
    usart_transmit_config(uartn, USART_TRANSMIT_ENABLE);
    usart_enable(uartn);
                
    /* enable USART RBNE interrupt */  
    usart_interrupt_enable(uartn, USART_INT_RBNE);
        
    //initUartBuf(uartn);
}

/*
 * 串口数据发送
 * 参数：uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 *              uint8_t buf:发送的数据指针
 *              uint8_t len:发送数据的长度
 * 返回:成功发送数据的长度
 * 备注：当前版本只有USART0/USART2可以用，其他没有调试验证
 */
uint8_t serialSend(uint32_t uartn, uint8_t *buf, uint8_t len)
{
    uint32_t index;
    uint8_t i;
    
    if ((buf == 0) && (len > 0U)){
        return 0;
    }

    index = uartn_to_index(uartn);
    if(USART_INDEXERR == index){
        return 0;
    }
    
    for(i = 0; i < len; i++)
    {
        usart_data_transmit(uartn, buf[i]);
    while (RESET == usart_flag_get(uartn, USART_FLAG_TBE));//判断是否发送成功
    }
    
    return len;
}

/*
 * 串口数据接收
 * 参数：uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * uint8_t buf:存放数据的指针
 * 返回:成功读取数据的长度
 * 备注：当前版本只有USART0/USART2可以用，其他没有调试验证
 */
uint8_t serialRecv(uint32_t uartn, uint8_t *buf)
{
    uint8_t size;
    
    size = readUartBuf(uartn, buf);
    return size;
}

/*
 * 串口接收中断
 * 参数：uint32_t uartn：USART0/USART1/USART2/UART3/UART4/USART5/UART6/UART7
 * 返回:void
 * 备注
 */
void serial_irq(uint32_t uartn)
{
    uint8_t dat;
    if((RESET != usart_interrupt_flag_get(uartn, USART_INT_FLAG_RBNE)) && 
       (RESET != usart_flag_get(uartn, USART_FLAG_RBNE)))
    {
      /* receive data */
      //dat = (usart_data_receive(uartn) & 0x7F);
        dat = (usart_data_receive(uartn) & 0xFF);
        writeUartBuf(uartn, &dat, sizeof(dat));
    }
    if((RESET != usart_flag_get(uartn, USART_FLAG_TBE)) && 
       (RESET != usart_interrupt_flag_get(uartn, USART_INT_FLAG_TBE)))
    {
        /* transmit data */
        //usart_data_transmit(uartn, 'A');
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    uint32_t timeout = 0;

    (void)f;
    
    usart_data_transmit(USART0, (uint8_t)ch);
    while (RESET == usart_flag_get(USART0, USART_FLAG_TC))
        {
            timeout++;
            if(timeout > 10000)//大概0.1s超时
            {
                break;
            }
        }
    return ch;
}


