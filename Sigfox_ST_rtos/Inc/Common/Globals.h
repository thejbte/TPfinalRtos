/*
 * Globals.h
 *
 *  Created on: 30/01/2019
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

#include "../Inc/Kernel/QuarkTS.h"
#include "../Inc/Task/Task.h"
#include "../Inc/State/State.h"

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


/*MAX_SIZE_IBUTTON_DATA sendpayload*/

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
void PrintString(UART_HandleTypeDef *huart,uint8_t *pData);
void PrintStringVar(UART_HandleTypeDef *huart,uint8_t *pData, uint16_t * ptr);
/***************Scheduler*****************************/
qTask_t Task_ApplicationFSM;
qSM_t StateMachine_ApplicationFSM;
extern  qSTimer_t Timeout;

/*Free rtos*/
extern SemaphoreHandle_t SemTxUart ;
extern SemaphoreHandle_t SemFSM ;
extern SemaphoreHandle_t SemRxUart ;
extern QueueHandle_t xQueueTx;
extern QueueHandle_t xQueueRx;

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
	union {
		uint8_t Others[3];
		struct {
			unsigned ADC_12 :12;   /*Canal AN 12 bits*/
			unsigned DUMMY	:12; 	/*N/A*/
		};
		struct {
			unsigned ADC_0:8;	/*Canal AN 8 bits*/
			unsigned ADC_1:8;	/*Canal AN 8 bits*/
			unsigned DI:1; 						// DI  LSB en el orden que lo coloque acá  DI, BattLow, Type  hacia abajo
			unsigned BattLow:1;
			unsigned Type:5;
			unsigned periodic:1;
		};
	}Once_Bytes;
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
			unsigned DI:1; 						// DI  LSB en el orden que lo coloque acá  DI, BattLow, Type  hacia abajo
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

/**************************Flags globals*******************************************************/

typedef struct {
	union{
		uint16_t Flags;
		struct{
			unsigned GPS_NO_ADQUISITION  :1;
			unsigned GPSAttached		 :1;
			unsigned estado_actual		 :1;
			unsigned flag_FirstEntry     :1;
			unsigned System_Init	     :1;
			unsigned flag_ON_WAKEUP_TIME :1;
			unsigned Flag_button         :1;
			unsigned FLAG_Coma_OK        :1;
			unsigned flag_nibble         :1;
			unsigned DL_Power_ON		 :1;
			unsigned msgADC		         :1;
			unsigned msgGPS			     :1;
			unsigned CoordInside		 :1;
			unsigned Flag_Encendido_Moto :1;
			unsigned promedioTemp		 :1;
			unsigned Histeresis          :1;
		};
	};
}Flags_t;

extern volatile Flags_t Flags_globals;

/**************************tipos de datos a enviar*******************************************************/

typedef enum{
	iButton,
	iButton_track,
	iButton_TH,
	iButton_track_TH,

	Event_battery_cuted_moto=17,

	Event_off_on_reed_switch=25,
	Event_on_off_reed_switch=26,
	Event_off_reed_switch=27,
	Event_on_reed_switch=28,

	iButto_logger_NTC=30,
	Event_out_geofence=31,
	Event_prueba_ganado=23
}Types_t;

typedef struct{
	Types_t xtype;
}tipo_t;
extern tipo_t xtypes;
/*********************************************************************************/
DL_Return DiscrimateFrameType(SigfoxConfig_t *obj);

#endif /* GLOBALS_H_ */


