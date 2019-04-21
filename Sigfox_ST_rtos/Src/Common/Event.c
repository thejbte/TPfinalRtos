/*
 * Event.c
 *
 *  Created on: 01/04/2019
 *      Author: julian
 */

#include <Common/Event.h>


void HAL_SYSTICK_Callback(void)
{
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_SYSTICK_Callback could be implemented in the user file
           	   Se debe incluir en stm32l0xx_it  HAL_SYSTICK_IRQHandler
   */
	//qSchedulerSysTick();
	Debounce_Update(&Fsm_DebounceData, HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13));

}
