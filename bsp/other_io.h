/*!
    \file  other_io.c
    \brief 
*/

#ifndef _OTHER_IO_H
#define _OTHER_IO_H


#include "gd32f4xx.h"

/*!
    \brief      其他离散IO初始化
    \param  	void
    \retval     void
*/
extern void other_io_init(void);

/*!
    \brief      读取8个LOS信号的状态值
    \param  	void
    \retval     uint8_t 返回8个loss信号的值 
				bit0对应SFP0_LOS bit7对应SFP7_LOS
*/
extern uint8_t read_los(void);

/*!
    \brief      读取6个VPX_GA0~4 GAP信号的状态值
    \param  	void
    \retval     uint8_t 返回6个VPX_GA0~4 GAP信号的值
				bit0对应VPX_GA0 bit4对应VPX_GA4 bit4对应VPX_GAP
*/
extern uint8_t read_GA(void);

/*!
    \brief      读取4个RA_N0~1信号的状态值
    \param  	void
    \retval     uint8_t 返回4个RA_N0~3信号的值
				bit0对应RA_N0 bit3对应RA_N3
*/
extern uint8_t read_RA(void);

/*!
    \brief      读取NVMRO信号的状态值
    \param  	void
    \retval     uint8_t 返回NVMRO信号的值
				bit0对应NVMRO
*/
extern uint8_t read_NVMRO(void);

#endif /* _OTHER_IO_H */

