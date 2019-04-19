/*
 * RTC_Config.c
 *
 *  Created on: 30/01/2019
 *      Author: julian
 */
#include <Hardware_Profile/RTC_Config.h>
#include <Common/Globals.h>

RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef TTime = {0};
RTC_DateTypeDef DDate = {0};

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */


void MX_RTC_Init(void)
{

	  /* USER CODE BEGIN RTC_Init 0 */

	  /* USER CODE END RTC_Init 0 */

	  RTC_TimeTypeDef sTime = {0};
	  RTC_DateTypeDef sDate = {0};

	  /* USER CODE BEGIN RTC_Init 1 */

	  /* USER CODE END RTC_Init 1 */
	  /**Initialize RTC Only
	  */
	  hrtc.Instance = RTC;
	  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	  hrtc.Init.AsynchPrediv = 79;
	  hrtc.Init.SynchPrediv = 499;
	  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	  if (HAL_RTC_Init(&hrtc) != HAL_OK)
	  {
	    Error_Handler();
	  }

	  /* USER CODE BEGIN Check_RTC_BKUP */

	  /* USER CODE END Check_RTC_BKUP */

	  /**Initialize RTC and set the Time and Date
	  */
	  sTime.Hours = 0x0;
	  sTime.Minutes = 0x0;
	  sTime.Seconds = 0x0;
	  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
	  sDate.Month = RTC_MONTH_FEBRUARY;
	  sDate.Date = 0x01;
	  sDate.Year = 0x0;

	  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
	  {
	    Error_Handler();
	  }
	  /**Enable the Alarm A
	  */
	  /*H:0-23 , M: 0-59 S: 0-59*/
	   ReportTimeHour = 1;

	   setTime(ReportTimeHour,ReportTimeMinute,ReportTimeSecond); //SigfoxModule.UL_ReportTimeS);
	  /* USER CODE BEGIN RTC_Init 2 */

	  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc){

	Flags_globals.flag_ON_WAKEUP_TIME = 1;
}

void RTC_WakeUp_Clear_Flag(RTC_HandleTypeDef *hrtc){
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	//__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG (hrtc, RTC_FLAG_WUTF);
}


void Standby_Mode_Entry(void){
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnterSTANDBYMode();
}

void RTC_Set_To_Zero_DateTime(RTC_HandleTypeDef *hrtc){
	RTC_TimeTypeDef TTime = {0};
	/*RTC_DateTypeDef DDate = {0};*/

	TTime.Hours=0;
	TTime.Minutes=0;
	TTime.Seconds=0;
	HAL_RTC_SetTime(hrtc,&TTime,RTC_FORMAT_BIN);
}

/**
 * hex Bcd Hours (0-0x23  o 0-0x12) ,Minutes(0-0x59) ,  Seconds(0-0x59)
 *
 * */
void setTime(uint8_t Hours, uint8_t Minutes, uint8_t Seconds){
	  RTC_AlarmTypeDef sAlarm = {0};

	  /**Enable the Alarm A
	 	  */
	  /*Decimal to BCD*/
	  Hours = ((Hours/10)*16 +(Hours%10) );  /*((year / 10) << 4) | (year % 10)*/
	  Minutes = ((Minutes/10)*16 +(Minutes%10) );
	  Seconds = ((Seconds/10)*16 +(Seconds%10) );

	  sAlarm.AlarmTime.Hours = Hours > RTC_MAX_HOURS ? RTC_MAX_HOURS : Hours;  /**/
	  sAlarm.AlarmTime.Minutes = Minutes > RTC_MAX_MIN ? RTC_MAX_MIN : Minutes; /*0x59 = 59 min esta en hex bcd, 0x10 = 10 min = 8*/
	  sAlarm.AlarmTime.Seconds = Seconds > RTC_MAX_SEG ? RTC_MAX_MIN : Seconds;

	  sAlarm.AlarmTime.SubSeconds = 0x0;
	  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	  /*No importa la fecha mask 3, solo importa mask 0,1,2 h:m:s ver dm0025071*/
	  sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;  /*RTC_ALARMMASK_NONE; Importa fecha y hora para alarma*/
	  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
	  sAlarm.AlarmDateWeekDay = 0x1;
	  sAlarm.Alarm = RTC_ALARM_A;
	  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
	  {
		Error_Handler();
	  }
}
