/*
 * Globals.c
 *
 *  Created on: 30/01/2019
 *      Author: julian
 */

#include <Common/Globals.h>


/** Antirebore */
DebounceData_t Fsm_DebounceData;
volatile Debounce_Flag_t Debounce_Flag;

/** Scheduler */
qSTimer_t Timeout = QSTIMER_INITIALIZER;

/*frertos*/
SemaphoreHandle_t SemTxUart = NULL;
SemaphoreHandle_t SemFSM = NULL;
SemaphoreHandle_t SemRxUart = NULL ;
QueueHandle_t xQueueTx = NULL;
QueueHandle_t xQueueRx = NULL;


 /** otros */
volatile uint8_t FlagCont=0;
uint8_t ReportTimeHour = 0;
uint8_t ReportTimeMinute = 0;
uint8_t ReportTimeSecond = 0;
/******************Sigfox library*******************************/
SigfoxConfig_t SigfoxModule;

void RSTCtrl_Sigfox(uint8_t sValue){
	if(sValue) HAL_GPIO_WritePin(GPIOA, RST_SIGFOX_Pin, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOA, RST_SIGFOX_Pin, GPIO_PIN_RESET);
}
void RST2Ctrl_Sigfox(uint8_t sValue){
	if(sValue) HAL_GPIO_WritePin(GPIOA, RST2_SIGFOX_Pin, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOA, RST2_SIGFOX_Pin, GPIO_PIN_RESET);
}

/*****Función TX DEBUG para envolver(Wrap) con Libreria sigfox********/
void UART_SIGFOX_TX_DEBUG_STM(void * Sp, char c){
	//HAL_UART_Transmit(&huart1,(uint8_t*)&c,USART_TX_AMOUNT_BYTES,USART_TIMEOUT);
}

/*****Función TX para envolver(Wrap) con Libreria sigfox********/
void UART_SIGFOX_TX_STM(void * Sp, char c){
	HAL_UART_Transmit(&huart1,(uint8_t*)&c,USART_TX_AMOUNT_BYTES,USART_TIMEOUT);
}

/*****Función RX para envolver(Wrap) con Libreria sigfox********/
unsigned char UART_SIGFOX_RX_STM( unsigned char * Chr){
	*Chr = UART_RX.Data;
	return WRAPER_ERR_OK;
}

/************************Data Sigfox *************************************/
DataFrame_t DataFrame;

/*flags globales*/
volatile Flags_t Flags_globals;

/*Tipo de datos*/
tipo_t xtypes;

/*Enviar por tx Debug*/
void PrintString(UART_HandleTypeDef *huart,uint8_t *pData){

	HAL_UART_Transmit(huart,pData,strlen((const char *)(pData) ),500);
}

/*Enviar por tx Debug var ************************************************/
void PrintStringVar(UART_HandleTypeDef *huart,uint8_t *pData, uint16_t * ptr){
	uint8_t length;
	length = strlen((const char *)(pData) );
	uint8_t Buf[length];
	memset((void *)&Buf,0,sizeof(Buf));
	if(ptr != NULL){
		sprintf((char *)&Buf,(const char *)(pData),*ptr);
		HAL_UART_Transmit(huart,(uint8_t *)&Buf,sizeof(Buf),USART_TIMEOUT);
	}
}

DL_Return DiscrimateFrameType(SigfoxConfig_t *obj){
	uint16_t tempReg;

	/* Discriminate the frame type */
    switch(obj->DL_NumericFrame[DL_CTRLREG] >> 4){ /* 4 most significant bits */

        case DL_FRAME_REPORT_TIME_AND_TURN_OFF_MOTO:
            tempReg = (obj->DL_NumericFrame[DL_TREP] << 8) | obj->DL_NumericFrame[DL_TREP + 1]; 	/* junto los 2 bytes en 1*/
            //Flags_globals.DL_Power_ON = numericFrame[DL_P_ON_OFF] <= 0 ? 0 :1;
            if(tempReg >= DL_MIN_REPORT_TIME){
              if(tempReg != obj->UL_ReportTimeS){ /*Si el dato es diferente*/
            	  obj->UL_ReportTimeS = tempReg;
               	}
            	return DL_TIME_OK;
                /* TODO: Add callback here */
            }

        break;

        default:
        break;
    }/* End switch */

    return DL_SUCCESS;
}

