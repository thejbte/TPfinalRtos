/* USER CODE BEGIN Header */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include <Common/Globals.h>


#define TIME_DELAY_MS_SEMAPHORE		200/portTICK_RATE_MS

osThreadId defaultTaskHandle;

/* USER CODE BEGIN 0 */
void TaskFSM( void* taskParmPtr);
void TaskDL_DecoFrame ( void* taskParmPtr);
void TaskTxUart ( void* taskParmPtr);
typedef char Stringtx[32];  /*Tipo de Dato para La cola */
/* USER CODE END 0 */

int main(void)
{

	HAL_Init();
	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN 2 */
	MX_GPIO_Init();   		 /*Inicializo GPIO STM32*/
	MX_RTC_Init();   		 /*Inicializo tiempos del RTC para despertar por tiempos ->> ReportTimeSecond*/
	MX_USART1_UART_Init();	  /*Iincializo Uart1 para Sigfox , 9600 baudios*/
	MX_USART2_UART_Init();   /*Iincializo Uart2 para Debug, 9600 baudios*/
	HAL_UART_Receive_IT(&huart1,(uint8_t *)&UART_RX.Data,USART_RX_AMOUNT_BYTES);



	/*ANTIREBOTE 40 ms*/
	Debounce_Init(&Fsm_DebounceData,40, PULL_DOWN);

	/*Task Create*/

	/*Tarea para la maquina de estados, envia la secuencia de comandos*/
	xTaskCreate(TaskFSM, (const char *)"TaskFSM",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);

	/*Tarea para transmitir por la Uart*/
	xTaskCreate(TaskTxUart, (const char *)"TaskTxUart",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);

	/*Tarea para decodificar el mensaje de bajada de sigfox*/
	xTaskCreate(TaskDL_DecoFrame, (const char *)"DecoFrame",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, &xTaskHandle_DL_RxNotify);

	/* Configure the sigfox library  */
	SigfoxModule.StatusFlag = SigfoxInit(&SigfoxModule, RSTCtrl_Sigfox, RST2Ctrl_Sigfox, UART_SIGFOX_TX_STM, UART_SIGFOX_RX_STM ,UL_RCZ4, DiscrimateFrameType);

	/*Despierto modulo Wisol*/
	SigfoxWakeUP(&SigfoxModule);

	/*habilitar salir del modo debajo consumo con interrupción en PA0*/
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);


	/* USER CODE BEGIN RTOS_SEMAPHORES */

	/*Semaforo binario para sincronización por Uart*/
	SemTxUart = xSemaphoreCreateBinary();

	/*Semaforo binario para sincronización  entre maquina de estados y respuesta de cada comando*/
	SemFSM = xSemaphoreCreateBinary();

	/*Semaforo mutex para proteger la escritura de la uart desde otras tareas*/
	SemUart = xSemaphoreCreateMutex();
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_QUEUES */
	/*Cola para transferir datos entre FSM y UArt*/
	xQueueTx = xQueueCreate(1 , sizeof(Stringtx));
	/* USER CODE END RTOS_QUEUES */

	/* Start scheduler */
	vTaskStartScheduler();

	while (1){ }
}

/* USER CODE BEGIN 4 */
/*===============================================================================================
									Tarea maquina de estados
  ===============================================================================================*/
void TaskFSM( void* taskParmPtr){

	static uint8_t state = STATE_SIGFOXINIT;
	static uint8_t FlagEnd = 0;

	char  x[32] ="AT\r" ;
	/*Downlink modificar en 1 si quiere pedir un mensaje de bajada, tiempo de espera alrededor de 45s*/
	static uint8_t DowlinkAux = 0 ;
	for(;;){

		if(state == STATE_SLEEP  ){
			FlagEnd = 1;
			/*Retardo para el led de transmisión, ya que es muy rapido*/
			vTaskDelay(3000/ portTICK_RATE_MS);
			/*Bajo consumo UART DEBUG*/
			UartprotectedMutex(&huart2,SemUart,(uint8_t*)"Sleep\r\n");

			/*Led Azul, indicador de transmisión de mensaje*/
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);

			/*Entra en modo de bajo consumo*/
			RTC_WakeUp_Clear_Flag(&hrtc);
			Standby_Mode_Entry();
			UartprotectedMutex(&huart2,SemUart,(uint8_t*)"Si paso esto el uC no entro en modo Sleep");
		}
		/*Si el mensaje No ha salido Envio datos Por la cola  ala Uart de SIgfox*/
		if(!FlagEnd){
			if(pdTRUE != xQueueSend(xQueueTx , &x,portMAX_DELAY) ){ 	/*Fail send to queue*/	}
			/*Libero el semaforo para que TaskReceiveUart reciba el dato por la  cola xQueueTx*/
			xSemaphoreGive(SemTxUart);
		}

		/*Se libera Semaphore cuando llega un mensaje por Interrupción por  uart*/
		if( pdTRUE == xSemaphoreTake(SemFSM,portMAX_DELAY) ){
			/*Maquina de estados  UPLINK SIGFOX*/
			state = FSM_Send_Command(state, &FlagEnd,&DowlinkAux,x);
			/*Permite llenar el buffer de nuevoCuando Llega \r la llego una trama, entonces RxReady = 1*/
			SigfoxModule.RxReady = 0;
		}
	}
}

/*===============================================================================================
									Tarea Decodificar trama de Downlink Sigfox
  ===============================================================================================*/
void TaskDL_DecoFrame ( void* taskParmPtr){
	for(;;){
		xTaskNotifyWait(0,0,NULL,portMAX_DELAY);
		if(DL_TIME_OK == DL_DiscriminateDownLink(&SigfoxModule)){
			/* tIEMPO DE SALIDA  ON CON DOWNLINK*/
			vTaskDelay(10000/ portTICK_RATE_MS);
			UartprotectedMutex(&huart2,SemUart,(uint8_t*)"Sleep\r\n");

			/*Entro en modo de bajo consumo*/
			RTC_WakeUp_Clear_Flag(&hrtc);
			Standby_Mode_Entry();
			UartprotectedMutex(&huart2,SemUart,(uint8_t*)"Si paso esto el uC no entro en modo Sleep\r\n");
		}
		else{
			UartprotectedMutex(&huart2,SemUart, (uint8_t*)"Downlink Timeout  or Non Downlink \r\n");
			/*Entro en modo de bajo consumo, si la trama de Downlink esta mal*/
			RTC_WakeUp_Clear_Flag(&hrtc);
			Standby_Mode_Entry();
		}
	}
}

/*===============================================================================================
							Tarea Transmitir por UART al modulo  Wisol SIGFOX
  ===============================================================================================*/
void TaskTxUart ( void* taskParmPtr){
	char RxQueue[32] ={ 0};
	for(;;){
		/*Tomo semaforo, lo libera cuando se llega mensaje por uart int*/
		if( pdTRUE == xSemaphoreTake(SemTxUart,portMAX_DELAY) ){
			if( pdTRUE == xQueueReceive(xQueueTx , &RxQueue,1000) ){
				UartprotectedMutex(&huart2,SemUart,(uint8_t*)RxQueue);
				UartprotectedMutex(&huart1,SemUart,(uint8_t*)RxQueue);
			};
		}
	} /*End For*/
} /*End Task*/



/* USER CODE END 4 */
/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(char *file, uint32_t line)
{ 
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
