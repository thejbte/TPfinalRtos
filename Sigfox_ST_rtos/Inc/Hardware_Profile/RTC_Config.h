/*
 * RTC_Config.h
 *
 *  Created on: 01/04/2019
 *      Author: julian
 */

#ifndef RTC_CONFIG_H_
#define RTC_CONFIG_H_

#include "main.h"
#include "stm32f070x6.h"
#include "stm32f0xx.h"

#include <Common/Globals.h>

/*VALORES MAXIMOS RTC TIME*/
#define RTC_MAX_HOURS	0x23
#define RTC_MAX_MIN		0x59
#define RTC_MAX_SEG		0x59

/***************RTC*****************************/
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef TTime;
extern RTC_DateTypeDef DDate;


void MX_RTC_Init(void);
void RTC_WakeUp_Clear_Flag(RTC_HandleTypeDef *hrtc);
void Standby_Mode_Entry(void);
void RTC_Set_To_Zero_DateTime(RTC_HandleTypeDef *hrtc);
void setTime(uint8_t Hours, uint8_t Minutes, uint8_t Seconds);

#endif /* RTC_CONFIG_H_ */
