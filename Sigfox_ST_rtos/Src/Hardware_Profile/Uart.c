/*
 * Uart.c
 *
 *  Created on: 01/04/2019
 *      Author: julian
 */


#include <Hardware_Profile/Uart.h>
#include <Common/Globals.h>

UART_BufferData_t UART_RX;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
void MX_USART1_UART_Init(void)
{

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 9600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 9600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */
}
/*
 ===================================================================================
		 	 ### Funcion recibe por interrupcion de recepcion
 ===================================================================================
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	UBaseType_t uxSavedInterruptStatus;

	if(huart->Instance == USART1){
		/*Se proteje la escritura de los datos en el buffer*/
		uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR() ;
		SigfoxISRRX(&SigfoxModule); /*almacena datos en el buffer recepcion*/
		taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);

		HAL_UART_Receive_IT( &huart1,(uint8_t *)&UART_RX.Data,USART_RX_AMOUNT_BYTES);
		if(SigfoxModule.RxReady ) xSemaphoreGiveFromISR( SemFSM, &xHigherPriorityTaskWoken );
	}
	/* If xHigherPriorityTaskWoken was set to true you we should yield.  The actual macro used here is    port specific. */
	if(xHigherPriorityTaskWoken) portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}
/*
 ===================================================================================
		 	 ###
 ===================================================================================
 */

