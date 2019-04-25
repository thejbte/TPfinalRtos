/*
 * Globals.h
 *
 *  Created on: 01/04/2019
 *      Author: julian
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_


#include <Drivers_Hd/Debounce.h>
#include <Hardware_Profile/RTC_Config.h>
#include <Hardware_Profile/Uart.h>

#include "stm32f0xx.h"

#include "stdio.h"
#include "string.h"
#include "stdint.h"


#include "../Inc/SystemClock_Config.h"
#include "../Inc/Common/Event.h"
#include "../Inc/Hardware_Profile/GPIO_Config.h"

#include "../Inc/Drivers_Hd/Sigfox.h"
#include "main.h"
#include "cmsis_os.h"

/*Definition */

//#define Once_Bytes
#define Doce_Bytes

/*Definition Others*/
#define MAX_BUFFER_SIZE
#define VERBOSE

#define RC_4

/*Wrap */
#define iButton_Data DataFrame


#ifdef _11_BYTES
#define MAX_SIZE_IBUTTON_DATA	11
#else
#define	MAX_SIZE_IBUTTON_DATA	12
#endif


/** Error */
#define WRAPER_ERR_OK	0X00U /* OK*/

/** USART 1 */
#define USART_TIMEOUT		500
#define USART_RX_AMOUNT_BYTES	1
#define USART_TX_AMOUNT_BYTES	1

/** *UASRT2  */
#define UART_SIGFOX_TX_DEBUG_STM	UART_DEBUG

/*Prototipos defunciones para transmitir a la uart*/
void PrintString(UART_HandleTypeDef *huart,uint8_t *pData);
void PrintStringVar(UART_HandleTypeDef *huart,uint8_t *pData, uint16_t * ptr);

/***************Scheduler*****************************/


/*Free rtos*/
extern SemaphoreHandle_t SemTxUart ;
extern SemaphoreHandle_t SemFSM ;
extern SemaphoreHandle_t SemUart ;
extern QueueHandle_t xQueueTx;
extern QueueHandle_t xQueueRx;
extern TaskHandle_t xTaskHandle_DL_RxNotify;
BaseType_t xHigherPriorityTaskWoken;


/*fsm rtos*/
typedef enum{
	STATE_SLEEP = 0,
	STATE_SIGFOXINIT,
	STATE_SIGFOXCHECKMODULE,
	STATE_SIGFOXCHECKCHANNELS,
	STATE_SIGFOXRESETCHANNELS,
	STATE_SIGFOXSENDPAYLOAD,
	STATE_SIGFOXCHANGEFREQUENCY,
	STATE_SIFGOXSAVEPARAMETERS,
	STATE_SIGFOXGETID,
	STATE_SIGFOXGETPAC,
	STATE_BLINK,
	STATE_SIGFOXCHANGEFREQUENCYDL,
	STATE_SIGFOXGETVOLTBATERRY,
}FSM_States_t;
/********************Other Variables ***********************/
extern volatile uint8_t wakeup;
extern volatile uint8_t FlagCont;
extern uint8_t ReportTimeHour;
extern uint8_t ReportTimeMinute;
extern uint8_t ReportTimeSecond;
/*******************Sigfox Library**************************/
extern  SigfoxConfig_t SigfoxModule;
void RSTCtrl_Sigfox(uint8_t sValue);
void RST2Ctrl_Sigfox(uint8_t sValue);
void UART_SIGFOX_TX_STM(void * Sp, char c); /*Wrapper function Tx*/
unsigned char UART_SIGFOX_RX_STM( unsigned char * Chr); /*Wrapper function Rx*/


/***************Debounce*****************************/
typedef struct {
	uint8_t State;
}Debounce_Flag_t;
extern volatile Debounce_Flag_t Debounce_Flag;
extern DebounceData_t Fsm_DebounceData;


/**************************** Data Sigfox *****************************/
typedef struct {

#ifdef Once_Bytes
#endif

#ifdef Doce_Bytes
	union {
		uint8_t Others[4];

		/** Si solo es un canal de 12 bi*/
		struct {
			unsigned ADC_12 :12;	/*Canal AN 12 bits*/
			unsigned DUMMY	:20;	/*N/A*/
		};
		struct {
			unsigned ADC_0:12;		/*Canal AN 12 bits*/
			unsigned ADC_1:12;		/*Canal AN 12 bits*/
			unsigned DI:1; 		     // DI  LSB en el orden que lo coloque acá  DI, BattLow, Type  hacia abajo
			unsigned BattLow:1;
			unsigned Type:5;
			unsigned periodic:1;
		};
	}Doce_Bytes;
	float Longitud;
	float Latitud;

#endif
}DataFrame_t;

extern DataFrame_t DataFrame;

/**************************tipos de datos a enviar*******************************************************/

typedef enum{
	iButton,
	iButton_track,
}Types_t;

typedef struct{
	Types_t xtype;
}tipo_t;
extern tipo_t xtypes;

/***************************Prototipo Dfunción decodificar Frame******************************************************/
DL_Return DiscrimateFrameType(SigfoxConfig_t *obj);

/*Prototipo Función Trnasmite por Debug protegida por mutex**********************************/
void UartprotectedMutex(UART_HandleTypeDef *huart, SemaphoreHandle_t Semaph, uint8_t* Str);


uint8_t FSM_Send_Command(uint8_t state, uint8_t *FlagEnd,uint8_t* DowlinkAux, char* x);
#endif /* GLOBALS_H_ */


