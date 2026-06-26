/*!
    \file  cb2991.c
    \brief 
*/

#ifndef CB2991_H
#define CB2991_H

#include "gd32f4xx.h"
#include "gpio_i2c.h"

/*!
    \brief      将cb2991初始化为温度采集模式
    \param  	void
    \retval     ERR_STA:STA_ERR/STA_OK
*/
extern ERR_STA cb2991TempInit(void);

/*!
    \brief      将cb2991初始化为电压采集模式
    \param  	void
    \retval     ERR_STA:STA_ERR/STA_OK
*/
extern ERR_STA cb2991VolInit(void);

/*!
    \brief      将cb2991初始化为电压采集和温度采集模式
    \param  	void
    \retval     ERR_STA:STA_ERR/STA_OK
*/
extern ERR_STA cb2991CutomInit(void);

/*!
    \brief      获取温度采样值
    \param  	char *temp:读取的四个温度采样值
    \retval     ERR_STA:STA_ERR/STA_OK
*/
extern ERR_STA ReadCB2991Temp(char *temp);

/*!
    \brief      获取电压采样值
    \param  	char *temp:读取的四个温度采样值
    \retval     ERR_STA:STA_ERR/STA_OK
*/
extern ERR_STA ReadCB2991VolCur(uint8_t *pvol_data,uint8_t *pcurrent_data);

/*!
    \brief      获取电压采样值
    \param  	uint16_t *vol:读取的四个电压采样值
    \retval     ERR_STA:STA_ERR/STA_OK
*/
extern ERR_STA ReadCB2991Vol(uint16_t *vol);


/*!
    \brief      获取板卡温度
    \param  	char *temp:获取板卡上三个温度采样点的温度
    \retval     ERR_STA:STA_ERR/STA_OK
*/
extern ERR_STA ReadTemp(char *temp);

/*!
    \brief      获取板卡电压
    \param  	uint16_t *vol:读取的四个电压采样值,单位为mv
    \retval     ERR_STA:STA_ERR/STA_OK
*/
extern ERR_STA ReadVol(uint16_t *vol);

#endif /* CB2991_H */

