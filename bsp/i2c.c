#include <stdio.h>
#include <string.h>
#include "gd32f4xx.h"
#include "i2c.h"
#include "led.h"

static uint8_t i2c_addr[I2C_NUM] = {
	0,0,0,
};
static uint32_t i2c_clk[I2C_NUM] = {
	0,0,0,
};
//是否发生了总线错误中断，0表示没有发生过，其他表示错误代码
uint32_t i2c_err_flag[I2C_NUM] = {
	0, 0, 0
};

I2C_RX_BUF_TYPE rxbuf[I2C_NUM];

/*驱动编号转控制器编号*/
uint32_t index_to_I2Cx(uint8_t index)
{
		switch(index)
		{
			case 0:
				return I2C0;
			case 1:
				return I2C1;
			case 2:
				return I2C2;
			default:
				return I2C0;
		}
}

/*控制器编号转驱动编号*/
uint8_t I2Cx_to_index(uint32_t I2Cx)
{
		switch(I2Cx)
		{
			case I2C0:
				return 0;
			case I2C1:
				return 1;
			case I2C2:
				return 2;
			default:
				return 0;
		}
}

/*初始化接收buffer*/
static void rx_buf_init(void)
{
	uint8_t i;
	for(i = 0; i < I2C_NUM; i++)
	{
		memset(rxbuf[i].buf, 0, I2C_RX_BUF_SIZE);
		rxbuf[i].rx_cnt = 0;
		rxbuf[i].valid = STA_ERR;
	}
}

static void delay_1ms(uint32_t cnt)
{
	uint32_t count = cnt;
  uint32_t delay = 10000;

	while(count--)
	{
    while(delay--);
	}
}

/*!
    \brief      获取自己的I2C从机地址
    \param[in]  uint32_t i2c_x：第几路I2C  I2C0/I2C1/I2C2
    \retval     自己的I2C地址
*/
uint8_t get_i2c_selfaddr(uint32_t i2c_x)
{
	return i2c_addr[I2Cx_to_index(i2c_x)];
}

/*!
    \brief      cofigure the NVIC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
void i2c_nvic_config(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
    nvic_irq_enable(I2C0_EV_IRQn, 0, 3);
    nvic_irq_enable(I2C1_EV_IRQn, 0, 4);
    nvic_irq_enable(I2C2_EV_IRQn, 0, 5);
    nvic_irq_enable(I2C0_ER_IRQn, 0, 2);
    nvic_irq_enable(I2C1_ER_IRQn, 0, 1);
    nvic_irq_enable(I2C2_ER_IRQn, 0, 6);
}

/*!
    \brief      Initialize I2C clock address and mode
    \param[in]  uint32_t i2c_x：第几路I2C  I2C0/I2C1/I2C2
								uint32_t self_addr:自己的地址
								uint32_t speed:速度
    \param[out] none
    \retval     none
*/
void init_i2c(uint32_t i2c_x, uint32_t self_addr, uint32_t speed)
{
	/* I2C clock configure */
    i2c_clock_config(i2c_x, speed, I2C_DTCY_2);
	
    /* I2C address configure */
    i2c_mode_addr_config(i2c_x, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS,self_addr);
	
	/* smbus mode device type */
	//i2c_smbus_type_config(I2C0,I2C_SMBUS_DEVICE);
	
	/* enable the I2C1 interrupt */
    //i2c_interrupt_enable(I2C0, I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);
	
	/* enable I2C0 */
    i2c_enable(i2c_x);
	
    /* enable acknowledge */
    i2c_ack_config(i2c_x, I2C_ACK_ENABLE);
	/*初始化接收buffer*/
	rx_buf_init();

	i2c_addr[I2Cx_to_index(i2c_x)] = self_addr;
	i2c_clk[I2Cx_to_index(i2c_x)] = speed;
}

/*!
    \brief      复位并重新初始化I2C硬件
    \param[in]  uint32_t i2c_x：第几路I2C  I2C0/I2C1/I2C2
    \param[out] none
    \retval     none
*/
void int_i2c_hard(uint32_t i2c_x)
{
	//i2c_deinit(i2c_x);
	
	/* I2C clock configure */
    i2c_clock_config(i2c_x, i2c_clk[I2Cx_to_index(i2c_x)], I2C_DTCY_2);
	
    /* I2C address configure */
    i2c_mode_addr_config(i2c_x, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS,i2c_addr[I2Cx_to_index(i2c_x)]);
	
	/* smbus mode device type */
	//i2c_smbus_type_config(I2C0,I2C_SMBUS_DEVICE);
	
	/* enable the I2C1 interrupt */
    //i2c_interrupt_enable(I2C0, I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);
	
	/* enable I2C0 */
    i2c_enable(i2c_x);
	
    /* enable acknowledge */
    i2c_ack_config(i2c_x, I2C_ACK_ENABLE);
}

/*!
    \brief      等待一包完整的数据
    \param[in]  none
    \param[out] none
    \retval     返回有完整数据包的通道
*/
uint32_t wait_rx_pack(void)
{
	uint8_t i;
	
	for(i = 0; i < I2C_NUM; i++)
	{
		if(STA_OK == rxbuf[i].valid)
		{
			return index_to_I2Cx(i);
		}
	}
	
	return I2Cx_NONE;
}

/*!
    \brief      从指定通道拷贝数据
    \param[in]  uint32_t i2c_x：第几路I2C  I2C0/I2C1/I2C2
								uint8_t *recv_buf 用户的数据空间
    \param[out] none
    \retval     返回数据包的数据个数
*/
uint8_t recv_i2c_pack(uint32_t i2c_x, uint8_t *recv_buf)
{
	uint8_t i;
	uint8_t index;
	uint8_t cnt;
	
	index = I2Cx_to_index(i2c_x);
	for(i = 0; i < rxbuf[index].rx_cnt; i++)
	{
		recv_buf[i] = rxbuf[index].buf[i];
	}
	cnt = rxbuf[index].rx_cnt;
	
	memset(rxbuf[index].buf, 0, I2C_RX_BUF_SIZE);
	rxbuf[index].rx_cnt = 0;
	rxbuf[index].valid = STA_ERR;
	
	return cnt;
}

/*!
    \brief      Completely send one byte of data from I2C1
    \param[in]  addr: addr of transmission
		\param[in]	data: data of transmission
    \param[out] none
    \retval     none
*/
#define I2C_TIMEOUT_CNT 500000
void i2c_data_send(uint32_t i2c_x,uint32_t des_addr, uint8_t *snd_buf, uint8_t cnt)
{
	uint32_t time_out;
	/* wait until I2C bus is idle */
	time_out = 0;
    while(i2c_flag_get(i2c_x, I2C_FLAG_I2CBSY))
	{
		if(I2C_TIMEOUT_CNT < time_out)
		{
			printf("I2C%d busy\r\n", I2Cx_to_index(i2c_x) );
			goto STOP_LOOP;
		}
		time_out++;
	}

    /* send a start condition to I2C bus */
    i2c_start_on_bus(i2c_x);

	time_out = 0;
    /* wait until SBSEND bit is set */
    while(!i2c_flag_get(i2c_x, I2C_FLAG_SBSEND))
		{
			if(I2C_TIMEOUT_CNT < time_out)
			{
				printf("I2C%d SBSEND timeout\r\n", I2Cx_to_index(i2c_x) );
				goto STOP_LOOP;
			}
			time_out++;
		}
	
	/* I2C address configure */
    i2c_master_addressing(i2c_x,des_addr,I2C_TRANSMITTER);

	time_out = 0;
	while(!i2c_flag_get(i2c_x, I2C_FLAG_ADDSEND))
	{
		if(I2C_TIMEOUT_CNT < time_out)
		{
			printf("I2C%d ADDSEND timeout\r\n", I2Cx_to_index(i2c_x) );
			goto STOP_LOOP;
		}
		time_out++;
	}
	/* clear ADDSEND bit */
	i2c_flag_clear(i2c_x, I2C_FLAG_ADDSEND);
	delay_1ms(1);
	while(cnt-- > 0)
	{
		i2c_data_transmit(i2c_x,*(snd_buf++));
		
		time_out = 0;
		while(!i2c_flag_get(i2c_x, I2C_FLAG_TBE))
		{
			if(I2C_TIMEOUT_CNT < time_out)
			{
				i2c_software_reset_config(i2c_x, I2C_SRESET_SET);
				i2c_software_reset_config(i2c_x, I2C_SRESET_RESET);
				int_i2c_hard(i2c_x);
				printf("I2C%d I2C_FLAG_TBE timeout\r\n", I2Cx_to_index(i2c_x) );
				//return;
				goto STOP_LOOP;
			}
			time_out++;
		}
	}	
	i2c_stop_on_bus(i2c_x);
	return;
	STOP_LOOP:
#ifdef ENABLE_FWTD_FUN
	printf("watchDog reset...\r\n");
	//fan_power(PWR_OFF);//FAN_POWER_OFF
	while(1)//等待看门狗复位，不让程序跳转到主程序喂狗
	{
		ledFlash(GPIO_PIN_15);//闪灯程序
	}
#endif
	return;
}

/*!
    \brief      handle I2C event interrupt request
		\param[in]  uint32_t i2c_x：第几路I2C   I2C0/I2C1/I2C2
    \param[out] none
    \retval     none
*/
void I2C_EventIRQ_Handler(uint32_t i2c_x)
{
	uint8_t data;
	uint8_t i2c_index;
	switch(i2c_x)
	{
		case I2C0:
			i2c_index = 0;
			break;
		case I2C1:
			i2c_index = 1;
			break;
		case I2C2:
			i2c_index = 2;
			break;
		default:
			return;
	}
	
	if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_ADDSEND))
	{
        /* clear the ADDSEND bit */
        i2c_interrupt_flag_clear(i2c_x, I2C_INT_FLAG_ADDSEND);
    }
	if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_RBNE))
	{
		/* if reception data register is not empty ,I2C1 will read a data from I2C_DATA */
		data = i2c_data_receive(i2c_x);
		if(STA_ERR == rxbuf[i2c_index].valid)//如果数据还不是完整包, 才填充数据
		{
			if(rxbuf[i2c_index].rx_cnt >= I2C_RX_BUF_SIZE)//数据超过缓冲区大小
			{
				rxbuf[i2c_index].rx_cnt = 0;
			}
			rxbuf[i2c_index].buf[rxbuf[i2c_index].rx_cnt] = data;
			rxbuf[i2c_index].rx_cnt++;
		}
	}
	if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_BTC))//看数据手册500页，详细描述
	{
	/* if reception data register is not empty ,I2C1 will read a data from I2C_DATA */
	data = i2c_data_receive(i2c_x);
    }
	if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_STPDET))
	{
		  data = i2c_data_receive(i2c_x);
			if(STA_ERR == rxbuf[i2c_index].valid)//如果数据还不是完整包, 才填充数据
			{
				if(rxbuf[i2c_index].rx_cnt >= I2C_RX_BUF_SIZE)//数据超过缓冲区大小
				{
					rxbuf[i2c_index].rx_cnt = 0;
				}
				rxbuf[i2c_index].buf[rxbuf[i2c_index].rx_cnt] = data;
				rxbuf[i2c_index].rx_cnt++;
			}
			
	rxbuf[i2c_index].valid = STA_OK;
	/* clear the STPDET bit */
#if 0
	i2c_enable(i2c_x);
	/* disable I2C1 interrupt */
	i2c_interrupt_disable(i2c_x, I2C_INT_EV | I2C_INT_BUF);
#else
	//orc1910用的是该分支 20191218 凌晨0:05
		//reset_and_int_i2c(i2c_x);
	i2c_software_reset_config(i2c_x, I2C_SRESET_SET);
	i2c_software_reset_config(i2c_x, I2C_SRESET_RESET);
	int_i2c_hard(i2c_x);
#endif
    }
}

/*!
    \brief      handle I2C error interrupt request
    \param[in]  uint32_t i2c_x：第几路I2C   I2C0/I2C1/I2C2
    \param[out] none
    \retval     none
*/
void I2C_ErrorIRQ_Handler(uint32_t i2c_x)
{
		uint8_t index;
		uint32_t errSta = 0;
		
		index = I2Cx_to_index(i2c_x);
	
    /* no acknowledge received */
    if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_AERR)){
        i2c_interrupt_flag_clear(i2c_x, I2C_INT_FLAG_AERR);
				errSta |= ((uint32_t)1)<<0;
    }

    /* SMBus alert */
    if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_SMBALT)){
        i2c_interrupt_flag_clear(i2c_x, I2C_INT_FLAG_SMBALT);
				errSta |= ((uint32_t)1)<<1;
    }

    /* bus timeout in SMBus mode */
    if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_SMBTO)){
        i2c_interrupt_flag_clear(i2c_x, I2C_INT_FLAG_SMBTO);
				errSta |= ((uint32_t)1)<<2;
    }

    /* over-run or under-run when SCL stretch is disabled */
    if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_OUERR)){
        i2c_interrupt_flag_clear(i2c_x, I2C_INT_FLAG_OUERR);
				errSta |= ((uint32_t)1)<<3;
    }

    /* arbitration lost */
    if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_LOSTARB)){
        i2c_interrupt_flag_clear(i2c_x, I2C_INT_FLAG_LOSTARB);
				errSta |= ((uint32_t)1)<<4;
    }

    /* bus error */
    if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_BERR)){
        i2c_interrupt_flag_clear(i2c_x, I2C_INT_FLAG_BERR);
				errSta |= ((uint32_t)1)<<5;
    }

    /* CRC value doesn't match */
    if(i2c_interrupt_flag_get(i2c_x, I2C_INT_FLAG_PECERR)){
        i2c_interrupt_flag_clear(i2c_x, I2C_INT_FLAG_PECERR);
				errSta |= ((uint32_t)1)<<6;
    }

		if(0 == errSta)
		{
			errSta = ((uint32_t)1)<<31;//表示出现了未知错误
		}
		i2c_err_flag[index] |= errSta;
		
    /* disable the error interrupt */
    // i2c_interrupt_disable(i2c_x,I2C_CTL1_ERRIE | I2C_CTL1_BUFIE | I2C_CTL1_EVIE);
}
