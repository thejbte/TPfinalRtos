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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <Common/Globals.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIME_DELAY_MS_SEMAPHORE		200/portTICK_RATE_MS
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/


osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void StartDefaultTask(void const * argument);



/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void TaskFSM( void* taskParmPtr);
void TaskDL_DecoFrame ( void* taskParmPtr);
void TaskTxUart ( void* taskParmPtr);
typedef char Stringtx[32];  /*Tipo de Dato para La cola */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */

	/* USER CODE BEGIN 2 */
	MX_GPIO_Init();
	MX_RTC_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	HAL_UART_Receive_IT(&huart1,(uint8_t *)&UART_RX.Data,USART_RX_AMOUNT_BYTES);



	/*ANTIREBOTE*/
	Debounce_Init(&Fsm_DebounceData,40, PULL_DOWN);
	/*change tiempo transmision*/
	//setTime(0,0,8);  // 0x10 = 8


	/*Task Create*/
	xTaskCreate(TaskFSM, (const char *)"TaskFSM",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(TaskTxUart, (const char *)"TaskTxUart",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);
	xTaskCreate(TaskDL_DecoFrame, (const char *)"DecoFrame",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, &xTaskHandle_DL_RxNotify);

	/* Configure the sigfox library  */
	SigfoxModule.StatusFlag = SigfoxInit(&SigfoxModule, RSTCtrl_Sigfox, RST2Ctrl_Sigfox, UART_SIGFOX_TX_STM, UART_SIGFOX_RX_STM ,UL_RCZ4, DiscrimateFrameType);
	SigfoxWakeUP(&SigfoxModule);
	// qSetDebugFcn(UART_DEBUG);

	/*Enable pin wakeup PA0 Button*/
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

	/*Scheduler cooperaTivo*/

	//qSchedulerSetup(0.001, IdleTask_Callback, 0);
	//qSchedulerAddSMTask(&Task_ApplicationFSM, MEDIUM_Priority, 0.1, &StateMachine_ApplicationFSM, State_SigfoxInit, NULL, NULL, State_Failure, NULL, qEnabled, NULL);
	//qSchedulerRun();
	/* USER CODE END 2 */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	SemTxUart = xSemaphoreCreateBinary();
	SemFSM = xSemaphoreCreateBinary();
	SemUart = xSemaphoreCreateMutex();
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the thread(s) */
	/* definition and creation of defaultTask */

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_QUEUES */

	xQueueTx = xQueueCreate(1 , sizeof(Stringtx));

	/* USER CODE END RTOS_QUEUES */

	xPortGetMinimumEverFreeHeapSize ();
	xPortGetFreeHeapSize();
	/* Start scheduler */
	//osKernelStart();
	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */
/*===============================================================================================
									Tarea maquina de estados
  ===============================================================================================*/
void TaskFSM( void* taskParmPtr){

	static uint8_t state = STATE_SIGFOXINIT;
	static uint8_t FlagEnd = 0;
	char Message[24]="40c72123c29746bc";  /*test lat y long medellin*/

	/*frame test Downlink RX=50 00 00 3C 01 00 00 00  Mirar doc frame*/

	char  x[32] ="AT\r" ;
	static uint8_t DowlinkAux = 0 ;

	for(;;){
		if(state == STATE_SLEEP  ){
			FlagEnd = 1;
			vTaskDelay(3000/ portTICK_RATE_MS);

			xSemaphoreTake(SemUart,100/portTICK_RATE_MS);
			PrintString(&huart2, (uint8_t*)"Sleep");  /*UART DEBUG*/
			xSemaphoreGive(SemUart);

			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
			RTC_WakeUp_Clear_Flag(&hrtc);
			Standby_Mode_Entry();

			xSemaphoreTake(SemUart,100/portTICK_RATE_MS);
			PrintString(&huart2, (uint8_t*)"Si paso esto el uC no entro en modo Sleep"); /*UART DEBUG*/
			xSemaphoreGive(SemUart);
		}

		if(!FlagEnd){
			if(pdTRUE != xQueueSend(xQueueTx , &x,portMAX_DELAY) ){ 	/*Fail send to queue*/	}

			/*Libero el semaforo para que TaskReceiveUart reciba el dato por la  cola xQueueTx*/
			xSemaphoreGive(SemTxUart);
		}


		/*Se libera cuando llega un mensaje por Int uart*/
		if( pdTRUE == xSemaphoreTake(SemFSM,portMAX_DELAY) ){

			/*FSM UPLINK SIGFOX*/
			switch(state){
			case STATE_SIGFOXINIT :
				/*Respuesta del AT*/
				if(SIGFOX_PROCESS_SUCCED == SigfoxResponseReceived(&SigfoxModule,"OK\r") ){
					state = STATE_SIGFOXCHECKCHANNELS;
					strcpy((char*)x,"AT$GI?\r");  /*Cambio valor a agregar en la cola*/
				}
				break;

			case STATE_SIGFOXCHECKCHANNELS :
				/*Espero la respuesta de AT$GI?*/
				if(SIGFOX_CHANN_NO_OK == SigfoxCheckChannels(&SigfoxModule)){
					state = STATE_SIGFOXRESETCHANNELS;
					strcpy((char*)x,"AT$RC\r");   /*Cambio valor a agregar en la cola*/
				}else{
					state = STATE_SIGFOXSENDPAYLOAD;
					SigfoxModule.DownLink = DowlinkAux;
					sprintf((char *)x,"AT$SF=%s,%d\r",Message,SigfoxModule.DownLink);  /*Cambio valor a agregar en la cola*/
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET); /*PRENDE AL TRANSMITIR*/
				}
				break;

			case STATE_SIGFOXRESETCHANNELS :
				/*Espero la respuesta de AT$RC*/
				if(SIGFOX_PROCESS_SUCCED == SigfoxResponseReceived(&SigfoxModule,"OK\r") ){
					state = STATE_SIGFOXSENDPAYLOAD;
					SigfoxModule.DownLink = DowlinkAux;
					sprintf((char *)x,"AT$SF=%s,%d\r",Message,SigfoxModule.DownLink);  /*Cambio valor a agregar en la cola*/
					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET); /*PRENDE AL TRANSMITIR*/
				}
				break;

			case STATE_SIGFOXSENDPAYLOAD :
				/*Espero la respuesta de AT$SF= XXXX*/
				if(SIGFOX_PROCESS_SUCCED == SigfoxResponseReceived(&SigfoxModule,"OK") ){

					HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET); /*APAGA SI SALIO EL MSJ*/
					if(DowlinkAux){

						//xSemaphoreTake(SemUart,100/portTICK_RATE_MS);
						PrintString(&huart2,(uint8_t*)SigfoxModule.RxFrame);
						//xSemaphoreGive(SemUart);

						xTaskNotify(xTaskHandle_DL_RxNotify,0,eNoAction);  /* Uso notificación por overflow si uso otro semaforo mas*/
						//xSemaphoreGive(SemUart);
						FlagEnd =1;  /*No envio por la cola*/
					}else state = STATE_SLEEP;   /*Si es sin Downlink, Tx y modo bajo consumo*/
				}
				break;
			}
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
		//if( pdTRUE == xSemaphoreTake(SemUart,portMAX_DELAY))
		{

			if(DL_TIME_OK == DL_DiscriminateDownLink(&SigfoxModule)){
				vTaskDelay(10000/ portTICK_RATE_MS);  /* tIEMPO DE SALIDA  ON CON DOWNLINK*/

				xSemaphoreTake(SemUart,100/portTICK_RATE_MS);
				PrintString(&huart2, (uint8_t*)"Sleep\r\n");
				xSemaphoreGive(SemUart);

				RTC_WakeUp_Clear_Flag(&hrtc);
				Standby_Mode_Entry();

				xSemaphoreTake(SemUart,100/portTICK_RATE_MS);
				PrintString(&huart2, (uint8_t*)"Si paso esto el uC no entro en modo Sleep\r\n");
				xSemaphoreGive(SemUart);
			}
			else{
				xSemaphoreTake(SemUart,100/portTICK_RATE_MS);
				PrintString(&huart2, (uint8_t*)"Downlink Timeout  or bad Frame\r\n");
				xSemaphoreGive(SemUart);

				RTC_WakeUp_Clear_Flag(&hrtc);
				Standby_Mode_Entry();
			}
		}
	}
}

/*===============================================================================================
							Tarea Transmitir por UART al modulo  Wisol SIGFOX
  ===============================================================================================*/
void TaskTxUart ( void* taskParmPtr){
	//uint8_t vec[32];
	char RxQueue[32] ={ 0};
	for(;;){
		/*tomo semaforo, lo libera cuando se llega mensaje por uart int*/
		if( pdTRUE == xSemaphoreTake(SemTxUart,portMAX_DELAY) ){
			if( pdTRUE == xQueueReceive(xQueueTx , &RxQueue,1000) ){

				xSemaphoreTake(SemUart,100/portTICK_RATE_MS);
				PrintString(&huart2,(uint8_t*)RxQueue);
				xSemaphoreGive(SemUart);

				xSemaphoreTake(SemUart,100/portTICK_RATE_MS);
				PrintString(&huart1,(uint8_t*)RxQueue);
				xSemaphoreGive(SemUart);
			};
		}
	}
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
	/* USER CODE END 5 */
}

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
