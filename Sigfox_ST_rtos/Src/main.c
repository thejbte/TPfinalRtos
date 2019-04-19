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
void TaskReceiveUart ( void* taskParmPtr);
void TaskTxUartDebug ( void* taskParmPtr);

//SemaphoreHandle_t SemTxUart = NULL ;
//QueueHandle_t xQueueTx = NULL;
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
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
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
	//xTaskCreate(TaskReceiveUart, (const char *)"TaskFSM",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 2, NULL);
	xTaskCreate(TaskTxUartDebug, (const char *)"TaskTxUartDebug",configMINIMAL_STACK_SIZE*2, NULL, tskIDLE_PRIORITY + 1, NULL);


	/* Configure the sigfox library  */
	SigfoxModule.StatusFlag = SigfoxInit(&SigfoxModule, RSTCtrl_Sigfox, RST2Ctrl_Sigfox, UART_SIGFOX_TX_STM, UART_SIGFOX_RX_STM ,UL_RCZ4, DiscrimateFrameType);
	SigfoxWakeUP(&SigfoxModule);



	//PrintString(&huart2, (uint8_t *)"Inicio del sistema\r\n");

	// qSetDebugFcn(UART_DEBUG);

	/*Enable pin wakeup PA0 Button*/
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

	/*Scheduler cooperaTivo Tecrea SAS*/

	//qSchedulerSetup(0.001, IdleTask_Callback, 0);
	//qSchedulerAddSMTask(&Task_ApplicationFSM, MEDIUM_Priority, 0.1, &StateMachine_ApplicationFSM, State_SigfoxInit, NULL, NULL, State_Failure, NULL, qEnabled, NULL);
	//qSchedulerRun();
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	SemTxUart = xSemaphoreCreateBinary();
	//SemFSM = xSemaphoreCreateBinary();
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  //osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
 // defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
	xQueueTx = xQueueCreate(1 , sizeof(uint8_t));
	//xQueueRx = xQueueCreate(1 , sizeof(uint8_t));
  /* USER CODE END RTOS_QUEUES */
 

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */
		//PrintString(&huart2, "Hola\r\n");
		for(volatile int i=0; i<100000;i++){}
    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}







/* USER CODE BEGIN 4 */
void TaskFSM( void* taskParmPtr){

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount ();
	uint8_t x = 2;
	for(;;){

		 vTaskDelayUntil( &xLastWakeTime, 1000/ portTICK_RATE_MS);
		if(pdTRUE != xQueueSend(xQueueTx , &x,1000) ){
			/*Fail send to queue*/
		}
		x++;
		/*PrintString(&huart2, SigfoxModule.RxFrame);
		SigfoxModule.RxReady = 0;*/
		xSemaphoreGive(SemTxUart);
	}
}

void TaskReceiveUart ( void* taskParmPtr){
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount ();
	for(;;){
		vTaskDelayUntil( &xLastWakeTime, 100/ portTICK_RATE_MS);
	}
}
void TaskTxUartDebug ( void* taskParmPtr){
	uint8_t vec[32];
	uint8_t RxQueue = 0;
	for(;;){
		HAL_GPIO_WritePin(GPIOB, LED_Pin, !HAL_GPIO_ReadPin(GPIOB, LED_Pin));

		if( pdTRUE == xSemaphoreTake(SemTxUart,portMAX_DELAY) ){ 			// 3000/portTICK_RATE_MS
			if( pdTRUE == xQueueReceive(xQueueTx , &RxQueue,1000) ){
				sprintf((char*) vec,"data Receive %d\r\n",RxQueue);
				PrintString(&huart2, vec);
			}else PrintString(&huart2, (uint8_t *)"fail to receive\r");
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
