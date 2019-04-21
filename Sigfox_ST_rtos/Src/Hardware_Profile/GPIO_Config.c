/*
 * GPIO_Config.c
 *
 *  Created on: 01/04/2019
 *      Author: Ingeniero04
 */


#include "Hardware_Profile/GPIO_Config.h"


/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
void MX_GPIO_Init(void){

	  GPIO_InitTypeDef GPIO_InitStruct = {0};

	  /* GPIO Ports Clock Enable */
	  __HAL_RCC_GPIOF_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
	  __HAL_RCC_GPIOB_CLK_ENABLE();
	  __HAL_RCC_GPIOC_CLK_ENABLE();

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(GPIOA, RST2_SIGFOX_Pin|RST_SIGFOX_Pin, GPIO_PIN_SET);

	  /*Configure GPIO pin Output Level */
	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);

	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
	  /*Configure GPIO pin : PA0 */
//	  GPIO_InitStruct.Pin = GPIO_PIN_0;
//	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;//GPIO_MODE_IT_RISING;
//	  GPIO_InitStruct.Pull = GPIO_NOPULL;
//	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIO pins : RST2_SIGFOX_Pin RST_SIGFOX_Pin */
	  GPIO_InitStruct.Pin = RST2_SIGFOX_Pin|RST_SIGFOX_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /*Configure GPIOB pin : LED_Pin  PB1*/
	  GPIO_InitStruct.Pin = LED_Pin;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);


	  /*Configure GPIO pin : PC9 */
	  GPIO_InitStruct.Pin = GPIO_PIN_8;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /*Configure GPIO pin : PC9 */
	  GPIO_InitStruct.Pin = GPIO_PIN_9;
	  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  /* GPIO Ports Clock Enable */


//	  /*input Analog bajo consumo*/
//	  /*Configure GPIO pins : PF0 PF1 */
//	  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
//	  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//	  GPIO_InitStruct.Pull = GPIO_NOPULL;
//	  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
//
//	  /*Configure GPIO pins : PA1 PA6
//	                           PA7 PA13
//	                           PA14 */
//	  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_6
//	                          |GPIO_PIN_7|GPIO_PIN_13
//	                          |GPIO_PIN_14;
//	  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
//	  GPIO_InitStruct.Pull = GPIO_NOPULL;
//	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);






	  /* EXTI interrupt init*/
	  //HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);
	 // HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

}
