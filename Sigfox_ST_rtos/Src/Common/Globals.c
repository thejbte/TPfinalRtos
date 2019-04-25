/*
 * Globals.c
 *
 *  Created on: 01/04/2019
 *      Author: julian
 */

#include <Common/Globals.h>


/** Antirebore */
DebounceData_t Fsm_DebounceData;
volatile Debounce_Flag_t Debounce_Flag;



/*frertos*/
SemaphoreHandle_t SemTxUart = NULL;
SemaphoreHandle_t SemFSM = NULL;
SemaphoreHandle_t SemUart = NULL ;
QueueHandle_t xQueueTx = NULL;
QueueHandle_t xQueueRx = NULL;

TaskHandle_t xTaskHandle_DL_RxNotify = NULL;
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
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

/*Tipo de datos*/
tipo_t xtypes;

/*Wrapper para enviar por tx Debug*/
void PrintString(UART_HandleTypeDef *huart,uint8_t *pData){
	HAL_UART_Transmit(huart,pData,strlen((const char *)(pData) ),500);
}

/*wrapper para enviar por tx Debug con variable ************************************************/
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

/*========================Función decodificar trama de Downlink====================================*/
DL_Return DiscrimateFrameType(SigfoxConfig_t *obj){
	uint16_t tempReg;
	/*frame test Downlink RX=50 00 00 3C 01 00 00 00  Mirar doc frame*/

	/* Discriminate the frame type */
	switch(obj->DL_NumericFrame[DL_CTRLREG] >> 4){     /* 4 most significant bits */

	case DL_FRAME_REPORT_TIME_AND_TURN_OFF_OBJECT:
		tempReg = (obj->DL_NumericFrame[DL_TREP] << 8) | obj->DL_NumericFrame[DL_TREP + 1]; 	/* junto los 2 bytes en 1*/
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,obj->DL_NumericFrame[DL_P_ON_OFF] <= 0 ? 0 :1);
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

/*========================Función Trnasmite por Debug protegida por mutex==========================*/
void UartprotectedMutex(UART_HandleTypeDef *huart, SemaphoreHandle_t Semaph, uint8_t* Str){
	xSemaphoreTake(Semaph,100/portTICK_RATE_MS);
	PrintString(huart,Str);
	xSemaphoreGive(Semaph);
}


/*====================== Maquina de estados para transmitir la secuencia de comandos===============*/
uint8_t FSM_Send_Command(uint8_t state, uint8_t *FlagEnd,uint8_t* DowlinkAux, char* x){
	char Message[24]="40c72123c29746bc";   /*Mensaje fijo Latitud y longitud Medellin, Colombia*/

	switch(state){

	case STATE_SIGFOXINIT :
		/*Espero Respuesta del comando  AT*/
		if(SIGFOX_PROCESS_SUCCED == SigfoxResponseReceived(&SigfoxModule,"OK\r") ){
			state = STATE_SIGFOXCHECKCHANNELS;

			/*Cambio valor a transmitir por la cola*/
			strcpy((char*)x,"AT$GI?\r");
		}
		break;

	case STATE_SIGFOXCHECKCHANNELS :
		/*Espero la respuesta de AT$GI?*/
		if(SIGFOX_CHANN_NO_OK == SigfoxCheckChannels(&SigfoxModule)){
			state = STATE_SIGFOXRESETCHANNELS;
			/*Cambio valor a transmitir por la cola*/
			strcpy((char*)x,"AT$RC\r");
		}else{
			state = STATE_SIGFOXSENDPAYLOAD;
			SigfoxModule.DownLink = *DowlinkAux;
			/*Cambio valor a transmitir por la cola, con Dowlink o sin el*/
			sprintf((char *)x,"AT$SF=%s,%d\r",Message,SigfoxModule.DownLink);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET); /* Led Azul Prende al transmitir*/
		}
		break;
	case STATE_SIGFOXRESETCHANNELS :
		/*Espero la respuesta de AT$RC, comando para resetear canales de Sigfox*/
		if(SIGFOX_PROCESS_SUCCED == SigfoxResponseReceived(&SigfoxModule,"OK\r") ){
			state = STATE_SIGFOXSENDPAYLOAD;
			SigfoxModule.DownLink = *DowlinkAux;
			/*AT$SF Transmite un mensaje de max 12 bytes a Sigfox*/
			sprintf((char *)x,"AT$SF=%s,%d\r",Message,SigfoxModule.DownLink);
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET); /* Led Azul Prende al transmitir*/
		}
		break;

	case STATE_SIGFOXSENDPAYLOAD :
		/*Espero la respuesta de AT$SF= XXXX*/
		if(SIGFOX_PROCESS_SUCCED == SigfoxResponseReceived(&SigfoxModule,"OK") ){

			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET); /*Apago Led Si salio el mensaje*/

			/*Si el mensaje de Salida espera un Downlink*/
			if(DowlinkAux){
				UartprotectedMutex(&huart2,SemUart,(uint8_t*)SigfoxModule.RxFrame);
				/*Notifica la llegada de la trama desde la nube de sigfox ( Downlink frame) */
				/* Uso notificación por que ocupa 4 bytes menos y 45 % mas rapido,si uso otro semaforo mas :: Overflow*/
				xTaskNotify(xTaskHandle_DL_RxNotify,0,eNoAction);
				*FlagEnd =1;  /*No dejo que se envie por la cola*/
			}else state = STATE_SLEEP;   /*Si no tengo Downlink, Tx y voy a modo bajo consumo*/
		}
		break;
	} /*End Switch*/
	return state;

}

