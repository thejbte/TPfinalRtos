/*!
 * *******************************************************************************
 * @file Sigfox.h
 * @author modified by julian bustamante
 * @version 1.2.0
 * @date Mar 27, 2019
 * @brief Sigfox interface for the sigfox module. Interface
 * specific for module wisol SFM11R2D.
 *********************************************************************************/

#ifndef SOURCES_SIGFOX_H_
#define SOURCES_SIGFOX_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*BOOL VALUES*/
#define SF_TRUE						1
#define SF_FALSE 					0

/*Down link definitions ---------------------------------------------------------*/
/** Each byte represented in hex */
#define DL_BYTE_SIZE 2

#define DL_PAYLOAD_SYZE 8

/** Header(3) + Payload(16) + Spaces(7) */
#define DL_PAYLOAD_LENGTH 26

/** Offset between bytes within the string frame */
#define DL_BYTES_OFFSET 3

/** Downlink frame timeout */
#define DL_TIMEOUT 45//60.0

/** Minimum report time --> 10.285 min*/
#define DL_MIN_REPORT_TIME 617

/** Downlink request period maximun*/
#define DL_REQ_PERIOD_TRANSMISSION_MAX_DAY 4

/** Downlink request period horas */
#define DL_REQ_PERIOD_H 6

/** Downlink request period horas --> cada 6 horas*/
#define DL_REQ_PERIOD_S  (DL_REQ_PERIOD_H*3600) // 360 21600

/** X time base in seconds for wakeup */
#define DL_TIMEREQUEST(X)	(uint8_t)(DL_REQ_PERIOD_S/X)

#define DL_IF_ANY_ERROR(x)		((x == DL_HEAD_ERROR) || (x == DL_TAIL_ERROR) || (x == DL_LENGTH_ERROR) )


/* Frame types -------------------------------------------------------------------*/
#define DL_FRAME_REPORT_TIME  4
#define DL_FRAME_REPORT_TIME_AND_TURN_OFF_MOTO  5

/* Numeric frame offsets ---------------------------------------------------------*/
#define DL_CTRLREG 0
#define DL_TREP 2
#define DL_P_ON_OFF  4 /*turn on off moto*/





/** Función enable/ disable pin Reset******************************************************** */
/**
 * Puntero a funcion de  tipo DigitalFcn_t:
 * Maneja el habilitar o deshabilitar el Reset
 * EJ:
 *  ******Función Reset para envolver(Wrap) con Libreria sigfox********
 * void RSTCtrl_Sigfox(uint8_t sValue){
 *		if(sValue) HAL_GPIO_WritePin(GPIOA, RST_SIGFOX_Pin, GPIO_PIN_SET);
 *		else HAL_GPIO_WritePin(GPIOA, RST_SIGFOX_Pin, GPIO_PIN_RESET); }
 * */
typedef void (*DigitalFcn_t)(uint8_t);


/** Función Transmision por uart a sigfox**************************************************** */
/**
 * Puntero a funcion de  tipo TxFnc_t:
 * Maneja Transmision por uart a sigfox
 *
 * EJ:
 ******Función TX para envolver(Wrap) con Libreria sigfox********
 *  void UART_SIGFOX_TX_STM(void * Sp, char c){
 *  		HAL_UART_Transmit(&huart1,(uint8_t*)&c,USART_TX_AMOUNT_BYTES,USART_TIMEOUT);
 *		}
 * */
typedef void (*TxFnc_t)(void*,char);


/** Función Recepcion por uart a sigfox**************************************************** */
/**
 * Puntero a funcion de  tipo RxFnc_t:
 * Maneja Recepcion por uart a sigfox
 *
 * EJ:
 *****Función RX para envolver(Wrap) con Libreria sigfox********
 *	unsigned char UART_SIGFOX_RX_STM( unsigned char * Chr){
 *			*Chr = UART_RX.Data;
 *			return WRAPER_ERR_OK;
 *		}
 * */
typedef unsigned char (*RxFnc_t)(unsigned char*);


/****Estructura Inicializar sigfox********************************************************
 * */

/**
 * @brief Return codes for downlink operation.
 */
typedef enum{
   DL_SUCCESS,      /* Downlink successfully processed */
   DL_HEAD_ERROR,   /* Could not find RX */
   DL_TAIL_ERROR,   /* Could not find \r */
   DL_LENGTH_ERROR, /* Wrong length */
   DL_TIME_OK,		/*Time success*/
   DL_UNKNOWN,
   DL_DISCRIMINATE_ERROR
} DL_Return;

typedef struct SigfoxConfig{
	DigitalFcn_t RST;
	DigitalFcn_t RST2;
	TxFnc_t TX_SIGFOX;
	RxFnc_t RX_SIGFOX;
	/*Decodificar trama numerica definida por usuario
	 * Ej : DL_Return DiscrimateFrameType(SigfoxConfig_t *obj);
	 * */
	DL_Return (*DiscrimateFrameTypeFcn)(struct SigfoxConfig* );	 /*así por que depende de la misma estructura*/
	volatile char RxFrame[100];
	volatile char TxFrame[100];
	volatile unsigned char RxReady;
	volatile uint8_t RxIndex;
	uint8_t StatusFlag;
	uint32_t Frequency;
	uint8_t DownLink;
	uint32_t UL_ReportTimeS;
	uint8_t DL_NumericFrame[DL_PAYLOAD_SYZE];
}SigfoxConfig_t;


/**
 * @brief Return codes for Uplink operation.
 */
typedef enum{
SIGFOX_INIT_OK = 0,
SIFOX_INIT_FAILED,			/*1*/
SIGFOX_PROCESS_SUCCED,		/*2*/
SIGFOX_PROCESS_FAILED, 		/*3*/
SIGFOX_CHANN_OK, 			/*4*/
SIGFOX_CHANN_NO_OK, 		/*5*/
SIGFOX_DEFAULT = 255
}ULReturn;


/**Centro de frecuencia Hz - Uplink********************************************************
 * */
typedef enum{
	UL_RCZ1 = 868130000,
	UL_RCZ2 = 902200000,
	UL_RCZ3 = 923200000,
	UL_RCZ4 = 920800000,
	UL_RCZ5 = 923300000,
	UL_RCZ6 = 865200000
}SigfoxFrequenciesUplink_t;

/** centro de frecuencia Hz- Downlink******************************************************
 * */
typedef enum{
	DL_RCZ1 = 869525000,
	DL_RCZ2 = 905200000,
	DL_RCZ3 = 922200000,
	DL_RCZ4 = 922300000,
	DL_RCZ5 = 922300000,
	DL_RCZ6 = 866300000
}SigfoxFrequenciesDownlink_t;


/*================================================================================
 *   prototypes
 *================================================================================*/

/**
 * @brief Function initialize the Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form ULReturn.
 */
ULReturn SigfoxInit(SigfoxConfig_t *obj, DigitalFcn_t Reset, DigitalFcn_t Reset2, TxFnc_t Tx_SigFox, RxFnc_t Rx_SigFox,uint32_t Frequency_Tx, DL_Return (*DiscrimateFrameTypeFcn)(struct SigfoxConfig* ) );

/**
 * @brief Function set low power the Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxSleep(SigfoxConfig_t *obj);

/**
 * @brief Function wakeup from pin extern the Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxWakeUP(SigfoxConfig_t *obj);

/**
 * @brief Function check module sending AT command to the Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxCheckModule(SigfoxConfig_t *obj);

/**
 * @brief Function get ID from Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxGetID(SigfoxConfig_t *obj);

/**
 * @brief Function get PAC from Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxGetPAC(SigfoxConfig_t *obj);

/**
 * @brief Function ask channels of the transceiver.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxAskChannels(SigfoxConfig_t *obj);

/**
 * @brief Function verificate channels of the transceiver.
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form ULReturn.
 *   		SIGFOX_CHANN_NO_OK   : reset channels
 *			SIGFOX_CHANN_OK      : No reset channels
 */
ULReturn SigfoxCheckChannels(SigfoxConfig_t *obj);

/**
 * @brief Function reset channels from Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxResetChannels(SigfoxConfig_t *obj);

/**
 * @brief Function change frequency uplink from Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxChangeFrequencyUL(SigfoxConfig_t *obj);

/**
 * @brief Function ask frequency uplink from Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxAskFrequencyUL(SigfoxConfig_t *obj);

/**
 * @brief Function save parameter in the flash memory from Sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxSaveParameters(SigfoxConfig_t *obj);

/**
 * @brief Function send message frame to sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param data Structure containing frame to send the Sigfox module.
 * @param size number of bytes in the payload. less 12 bytes.
 * @param eDownlink downlink enable o disable (0/1)
 * @return Operation result in the form ULReturn.
 */
ULReturn SigfoxSendMessage(SigfoxConfig_t *obj,  void* data, uint8_t size, uint8_t eDownlink);

/**
 * @brief Function ISR UART receive incoming frame to Sigfox module.
 * the buffer is stored in the structure obj->RxFrame.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxISRRX(SigfoxConfig_t *obj);

/**
 * @brief Function verificate response received from sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * @param expectedResponse expected Response from module sigfox .
 * @return Operation result in the form ULReturn.
 *				SIGFOX_PROCESS_SUCCED
 *				SIGFOX_PROCESS_FAILED
 */
ULReturn SigfoxResponseReceived(SigfoxConfig_t *obj, char *expectedResponse);


/**
 * @brief Function to discriminate downlink frames.
 * @param obj Structure containing the incoming frame from the Sigfox module.
 * @param retVal Pointer to return a value.
 * @return Operation result in the form DL_Return.
 */
DL_Return DL_DiscriminateDownLink(SigfoxConfig_t* buff);

/**
 * @brief Function Sent command to sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * * @param Str containing message to tx.
 * @return void.
 */
void SigfoxCommandSend(SigfoxConfig_t *obj, char * Str);
/**
 * @brief Function to discriminate downlink frames type.
 * @param numericFrame buffer containing the incoming frame from the Sigfox module.
 * @param Time  transmission report time .
 * @return Operation result in the form DL_Return.
 */

#endif /* SOURCES_SIGFOX_H_ */
