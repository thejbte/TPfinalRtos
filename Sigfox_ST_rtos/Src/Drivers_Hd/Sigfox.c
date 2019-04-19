/*!
 * *******************************************************************************
 * @file Sigfox.c
 * @author modified by julian bustamante
 * @version 1.2.0
 * @date Mar 27, 2019
 * @brief Sigfox interface for the sigfox module. Interface
 * specific for module wisol SFM11R2D.
 *********************************************************************************/

#include "../Inc/Drivers_Hd/Sigfox.h"

/** Private Prototypes************************************************************************************************************************ */
/*Funcion para transmitir mensaje*/
static void SigfoxStringTX(SigfoxConfig_t *obj, char* SigfoxString);

/*Funcion Limpia buffer y ready */
static void SigfoxResetObject(SigfoxConfig_t *obj);

/*Funcion arma trama deacuerdo a la estructura de datos 12 bytes*/
static void SigfoxBuildFrame(char* str, void* data, uint8_t size);

/*Nible(4bits) decimal to hex string*/
char NibbletoX(uint8_t value);

/*Definitions Private**************************************************************************************************************************/

/*Maximo tamaño buffer char*/
#define SIGFOX_MAX_DATA_SIZE 		25

/*Maximo tamaño trama hex string*/
#define	SIGFOX_MAX_BYTE_TX_FRAME	12

/*Maximo tamaño buffer para las frecuencias*/
#define SIGFOX_MAX_BUFF_FREQ		15

/*Wraper( envolver)*/

/*Public Functions*/

/**
 * @brief Function initialize the Sigfox module.
 * Example :
 * 		SigfoxModule.StatusFlag = SigfoxInit(&SigfoxModule, RSTCtrl_Sigfox, RST2Ctrl_Sigfox, UART_SIGFOX_TX_STM, UART_SIGFOX_RX_STM ,UL_RCZ4);
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form ULReturn.
 */
ULReturn SigfoxInit(SigfoxConfig_t *obj, DigitalFcn_t Reset, DigitalFcn_t Reset2, TxFnc_t Tx_SigFox, RxFnc_t Rx_SigFox,uint32_t Frequency_Tx, DL_Return (*DiscrimateFrameTypeFCN)(struct SigfoxConfig* ) ){
	obj->RST=Reset;
	obj->RST2=Reset2;
	obj->TX_SIGFOX=Tx_SigFox;
	obj->RX_SIGFOX=Rx_SigFox;
	obj->DiscrimateFrameTypeFcn = DiscrimateFrameTypeFCN;
	memset( (void *) obj->RxFrame,0,sizeof(obj->RxFrame));
	memset( (void *) obj->TxFrame,0,sizeof(obj->TxFrame));
	obj->RxReady=SF_FALSE;
	obj->RxIndex=0;
	obj->Frequency=Frequency_Tx;
	return SIGFOX_INIT_OK;
}


/**
 * @brief Function set low power the Sigfox module.
 * Example :
 *		SigfoxSleep(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxSleep(SigfoxConfig_t *obj){
	obj->RST(1);
	obj->RST2(1);
	SigfoxStringTX(obj,"AT$P=2\r");
}


/**
 * @brief Function wakeup from pin extern the Sigfox module.
 * Example :
 * 		SigfoxWakeUP(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxWakeUP(SigfoxConfig_t *obj){
	obj->RST(SF_FALSE);
	obj->RST(SF_TRUE);
	obj->RST2(SF_TRUE);
}


/**
 * @brief Function check module sending AT command to the Sigfox module.
 * Example :
 * 		SigfoxCheckModule(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxCheckModule(SigfoxConfig_t *obj){
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,"AT\r");
}

/**
 * @brief Function get ID from Sigfox module.
 * Example :
 * 		SigfoxGetID(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxGetID(SigfoxConfig_t *obj){
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,"AT$I=10\r");
}

/**
 * @brief Function get PAC from Sigfox module.
 * Example :
 * 		SigfoxGetPAC(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxGetPAC(SigfoxConfig_t *obj){
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,"AT$I=11\r");
}

/**
 * @brief Function ask ask channels of the transceiver.
 * Example :
 * 		SigfoxAskChannels(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxAskChannels(SigfoxConfig_t *obj){
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,"AT$GI?\r");
}

/**
 * @brief Function verificate channels of the transceiver.
 * Example :
 * 		SigfoxCheckChannels(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return Operation result in the form ULReturn.
 * 			SIGFOX_CHANN_NO_OK   : se debe resetear canales
 *			SIGFOX_CHANN_OK      : No se resetea canales
 */
ULReturn SigfoxCheckChannels(SigfoxConfig_t *obj){
	uint8_t x,y;
	char* rspPtr;
	rspPtr=strstr((const char *)obj->RxFrame , ",");
	if(rspPtr != NULL){
		x = (*(rspPtr-1))-'0';
		y = (*(rspPtr+1))-'0';
		return (x == 0 || y < 3) ? SIGFOX_CHANN_NO_OK : SIGFOX_CHANN_OK;
	}
	return SIGFOX_PROCESS_FAILED;
}

/**
 * @brief Function reset channels from Sigfox module.
 * Example :
 * 		SigfoxResetChannels(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxResetChannels(SigfoxConfig_t *obj){
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,"AT$RC\r");
}

/**
 * @brief Function change frequency uplink from Sigfox module.
 * Example :
 * 		SigfoxChangeFrequency(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxChangeFrequencyUL(SigfoxConfig_t *obj){
	char Frequency[SIGFOX_MAX_BUFF_FREQ];
	memset(Frequency,0,sizeof(Frequency));
	sprintf(Frequency,"AT$IF=%lu\r",obj->Frequency);
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,Frequency);
}

/**
 * @brief Function ask frequency uplink from Sigfox module.
 * Example :
 * 		SigfoxAskFrequencyUL(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxAskFrequencyUL(SigfoxConfig_t *obj){
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,"AT$IF?\r");
}

/**
 * @brief Function save parameter in the flash memory from Sigfox module.
 * Example :
 * 		SigfoxSaveParameters(&SigfoxModule);
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxSaveParameters(SigfoxConfig_t *obj){
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,"AT$WR\r");
}

/*Private Functions*/

/**
 * @brief Function order the frame.
 *
 * @param str buffer.
 * @param data Structure containing frame.
 * @param size number of bytes in the payload. less or equal to 12 bytes.
  * @return void.
 */
static void SigfoxBuildFrame(char* str, void* data, uint8_t size){
	int8_t  i,j;
	uint8_t xbyte, finalsize;
	uint8_t *bdata = (uint8_t*)data;
	size = (size > SIGFOX_MAX_BYTE_TX_FRAME )? SIGFOX_MAX_BYTE_TX_FRAME : size;
	finalsize = size*2;
	str[finalsize]='\0';

	for(i = (size-1) , j = 0; i >= 0; i--){
		xbyte = bdata[i];
		str[j++]=NibbletoX(xbyte>>4);
		str[j++]=NibbletoX(xbyte);
	}
}


/**
 * @brief Function send message frame in string hexadecimal to sigfox module.
 * 	Example :
 * 		SigfoxModule.StatusFlag=SigfoxSendMessage(&SigfoxModule,&iButton_Data,MAX_SIZE_IBUTTON_DATA,SigfoxModule.DownLink);
 * @param obj Structure containing all data from the Sigfox module.
 * @param data Structure containing frame to send the Sigfox module.
 * @param size number of bytes in the payload. less o equal to 12 bytes.
 * @param eDownlink downlink enable o disable (0/1)
 * @return Operation result in the form ULReturn.
 *			 SIGFOX_PROCESS_SUCCED
 *			 SIGFOX_PROCESS_FAILED
 *
 */
ULReturn SigfoxSendMessage(SigfoxConfig_t *obj, void* data, uint8_t size, uint8_t eDownlink){
	char str[SIGFOX_MAX_DATA_SIZE] = {0};
	SigfoxResetObject(obj);

	SigfoxBuildFrame(str, data, size);
	SigfoxStringTX(obj,"AT$SF=");
	SigfoxStringTX(obj, str);
	if(!eDownlink)SigfoxStringTX(obj, "\r");
	else{
		SigfoxStringTX(obj, ",1\r");
		obj->DownLink = 1; /* Flag the downlink */
	}
	strcpy((char *)obj->TxFrame,(const char *)str);
	return SIGFOX_PROCESS_SUCCED;
}

/**
 * @brief Function ISR UART receive incoming frame to Sigfox module.
 * Example :
 * 		SigfoxISRRX(&SigfoxModule);    //call in the  interrup serial
 * the buffer is stored in the structure obj->RxFrame.
 * @param obj Structure containing all data from the Sigfox module.
 * @return void.
 */
void SigfoxISRRX(SigfoxConfig_t *obj){
	unsigned char rxChar_Sigfox;

	obj->RX_SIGFOX(&rxChar_Sigfox);
	if(obj->RxReady) return; // B_uffer reveived
	obj->RxFrame[obj->RxIndex++] = rxChar_Sigfox;
	if (obj->RxIndex>=sizeof(obj->RxFrame)-1) obj->RxIndex=0;
	obj->RxFrame[obj->RxIndex] = 0;
	if (rxChar_Sigfox=='\r'){
		/*  Check if there is a downlink request */
		if(!obj->DownLink){
		    obj->RxIndex = 0;
		    obj->RxReady = SF_TRUE; // Framed completed
		}else
			obj->DownLink = 0; /* Clear the downlink request */
	}
}


/**
 * @brief Function verificate response received from sigfox module.
 * Example :
 * 		if(SigfoxResponseReceived(&SigfoxModule,"OK") == SIGFOX_PROCESS_SUCCED) {}
 * @param obj Structure containing all data from the Sigfox module.
 * @param expectedResponse expected Response from module sigfox .
 * @return Operation result in the form ULReturn.
 *				SIGFOX_PROCESS_SUCCED
 *				SIGFOX_PROCESS_FAILED
 */
ULReturn SigfoxResponseReceived(SigfoxConfig_t *obj, char *expectedResponse){
	uint8_t Rprocess;
	if(strstr((const char *) obj->RxFrame,expectedResponse) != NULL) Rprocess = SIGFOX_PROCESS_SUCCED;
	else Rprocess = SIGFOX_PROCESS_FAILED;
	return Rprocess;
}

/**
 * @brief Function to discriminate downlink frames.
 * @param obj Structure containing the incoming frame from the Sigfox module.
 * @param retVal Pointer to return a value.
 * @return Operation result in the form DL_Return.
 */
DL_Return DL_DiscriminateDownLink(SigfoxConfig_t* obj){
    uint8_t* payLoadHead;
    uint8_t* payLoadTail;
    uint8_t byteIndex = DL_BYTES_OFFSET;
   // uint8_t numericFrame[DL_PAYLOAD_SYZE];
    uint8_t byteStr[DL_BYTE_SIZE + 1] = {0};

    /* Get payload offset */
    payLoadHead = (uint8_t *)strstr((const char*)obj->RxFrame, "RX");

    if(!payLoadHead)
        return DL_HEAD_ERROR;

    /* Check payload length */
    payLoadTail = (uint8_t *)strstr((const char*)payLoadHead, "\r");

    if(!payLoadTail)
        return DL_TAIL_ERROR;

    if((payLoadTail - payLoadHead) != DL_PAYLOAD_LENGTH)
        return DL_LENGTH_ERROR;

    /* Convert frame to numeric values */
    for(uint8_t i = 0; i < DL_PAYLOAD_SYZE; i++){

        /* Copy byte strings an convert them to numbers */
        strncpy((char *)byteStr, (const char *)(payLoadHead + byteIndex), DL_BYTE_SIZE);
        obj->DL_NumericFrame[i] = (uint8_t)strtol((const char *)byteStr, NULL, 16);
        byteIndex += DL_BYTES_OFFSET;
    }


    return ( NULL != obj->DiscrimateFrameTypeFcn )? obj->DiscrimateFrameTypeFcn(obj) : DL_DISCRIMINATE_ERROR;
}

/**
 * @brief Function Sent command to sigfox module.
 * @param obj Structure containing all data from the Sigfox module.
 * * @param Str containing message to tx.
 * @return void.
 */
void SigfoxCommandSend(SigfoxConfig_t *obj, char * Str){
	SigfoxResetObject(obj);
	SigfoxStringTX(obj,Str);
}

/*Private Functions ********************************************************************************************************************************/
void SigfoxStringTX(SigfoxConfig_t *obj, char* SigfoxString){
	while(*SigfoxString) {obj->TX_SIGFOX(NULL,*SigfoxString);SigfoxString++;}
}

/*Private Functions ***********************************************************************************************************************************************/
void SigfoxResetObject(SigfoxConfig_t *obj){
	memset((void *)obj->RxFrame,0,sizeof(obj->RxFrame));
	memset((void *)obj->TxFrame,0,sizeof(obj->TxFrame));
	obj->RxReady=SF_FALSE;
	obj->RxIndex=0;
	obj->StatusFlag=SIGFOX_DEFAULT;
}


/*Nible(4bits) decimal to hex string***********************************************************************************************************************************************/

char NibbletoX(uint8_t value){
    char ch;
    ch = (char)(value & 0x0F) + '0';
    return (ch > '9')? ch+7u : ch;
}

