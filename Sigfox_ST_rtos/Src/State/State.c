/*
 * State.c
 *
 *  Created on: 1/02/2019
 *      Author: julian
 */


#include "../Inc/State/State.h"



/***********************************************************************************************/
void State_Failure(qSM_t *fsm){

	qStateMachine_Attribute(fsm, qSM_CLEAR_STATE_FIRST_ENTRY_FLAG, NULL);
	//	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
void BeforeAny(qSM_t *fsm){

}
/***********************************************************************************************/


/***********************************************************************************************/
qSM_Status_t State_Sleep(qSM_t *fsm){
	if(fsm->StateFirstEntry){
		PrintString(&huart2,(uint8_t *)"State_Sleep\r\n");
		HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);
	}

	SigfoxSleep(&SigfoxModule);

	PrintString(&huart2,(uint8_t *)"Core Sleep\r\n");
	HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);
	//if(!Flags_globals.System_Init)
	{
		/*clear flag antes de wake up, mandatory*/
		/*REINICIALIZAR RTC PARA CONTAR Y DORMIR DE NUEVO pero aca cuenta apartir que apago y no que prendo*/
		//RTC_Set_To_Zero_DateTime(&hrtc);
		//HAL_RTC_SetTime(&hrtc,&TTime,RTC_FORMAT_BIN);
		RTC_WakeUp_Clear_Flag(&hrtc);
		Standby_Mode_Entry();
	}
	fsm->NextState =State_SigfoxInit;  // State_I2C_Battery;

	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SigfoxInit(qSM_t *fsm){
	if(fsm->StateFirstEntry){
		SigfoxWakeUP(&SigfoxModule);
		//	qTraceVar(FrameConfig.time,QT_DEC);
		HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_SET);
		PrintString(&huart2,(uint8_t *)"State_SigfoxInit\r\n");
		HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);
	}

	Flags_globals.Flag_button =	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	//|| Flags_globals.System_Init
	if( Flags_globals.flag_ON_WAKEUP_TIME  || Flags_globals.Flag_button){
		Flags_globals.System_Init = 0;
		if( (Flags_globals.Flag_button ) && !Flags_globals.flag_ON_WAKEUP_TIME){  iButton_Data.periodic=0;}

		if(Flags_globals.flag_ON_WAKEUP_TIME){
			iButton_Data.periodic=1;
			Flags_globals.flag_ON_WAKEUP_TIME=0;
		}
		SigfoxWakeUP(&SigfoxModule);

#ifdef MULTI_ZONE_SELECT
		fsm->NextState = State_CheckOptions;
#else
#if defined(RC_4)
		SigfoxModule.Frequency = UL_RCZ4;
#elif defined(RC_2)
		SigfoxModule.frequency = UL_RCZ4;
#endif
		fsm->NextState =State_SigfoxChangeFrequency ; 	//State_I2C_Battery;
#endif
	}else	fsm->NextState = State_Sleep;

	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SigfoxChangeFrequency(qSM_t *fsm){
	if(fsm->StateFirstEntry){
		PrintString(&huart2,(uint8_t *)"State_SigfoxChangeFrequencyUL\r\n");
		qSTimerSet(&Timeout, 0.5);
		SigfoxChangeFrequencyUL(&SigfoxModule);
	}

	if(SigfoxResponseReceived(&SigfoxModule,"OK") == SIGFOX_PROCESS_SUCCED){
		fsm->NextState = State_SigfoxChangeFrequencyDL;
		return qSM_EXIT_SUCCESS;
	}

	if(qSTimerExpired(&Timeout))  return qSM_EXIT_FAILURE;
	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SigfoxChangeFrequencyDL(qSM_t *fsm) {
	if(fsm->StateFirstEntry) {
		PrintString(&huart2,(uint8_t *)"[STATE] : State_SigfoxChangeFrequencyDL\r\n");
		qSTimerSet(&Timeout, 2.0);

#if defined(RC_4)
		PrintString(&huart1,(uint8_t *)"AT$DR=922300000\r");
#elif defined(RC_2)
		PrintString(&huart1,(uint8_t *)"AT$DR=905200000\r");
#endif
	}

	if( SigfoxResponseReceived(&SigfoxModule,"OK") == SIGFOX_PROCESS_SUCCED ) {
#if defined(RC_4)
		PrintString(&huart2,(uint8_t *)"AT$DR=922300000\r");//qPrintString(UART_DEBUG, NULL, "AT$DR=922300000\r\n");
#elif defined(RC_2)
		PrintString(&huart2,(uint8_t *)"AT$DR=905200000\r");
#endif

		fsm->NextState = State_SifgoxSaveParameters;
		return qSM_EXIT_SUCCESS;
	}

	if(qSTimerExpired(&Timeout))  return qSM_EXIT_FAILURE;

	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SifgoxSaveParameters(qSM_t *fsm){
	if(fsm->StateFirstEntry){
		PrintString(&huart2,(uint8_t *)"[STATE] : State_SifgoxSaveParameters\r\n");
		qSTimerSet(&Timeout, 0.5);
		SigfoxSaveParameters(&SigfoxModule);
	}

	if(SigfoxResponseReceived(&SigfoxModule,"OK") == SIGFOX_PROCESS_SUCCED){
#ifdef MULTI_ZONE_SELECT
		fsm->NextState = State_Blink;
#else
		fsm->NextState = State_SigfoxCheckModule;
#endif
	}

	if(qSTimerExpired(&Timeout)) return qSM_EXIT_FAILURE;

	return EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SigfoxCheckModule(qSM_t *fsm){
	HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);
	if(fsm->StateFirstEntry){
		PrintString(&huart2,(uint8_t *)"[STATE] : State_SigfoxCheckModule\r\n");
		qSTimerSet(&Timeout, 2.0);
		SigfoxCheckModule(&SigfoxModule);
	}

	if(SigfoxResponseReceived(&SigfoxModule, "OK") == SIGFOX_PROCESS_SUCCED){
		fsm->NextState = State_SigfoxGetID;
		return qSM_EXIT_SUCCESS;
	}

	if(qSTimerExpired(&Timeout)) return qSM_EXIT_FAILURE;
	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/

qSM_Status_t State_SigfoxGetID(qSM_t *fsm){
	if(fsm->StateFirstEntry){
		PrintString(&huart2,(uint8_t *)"[STATE] : State_SigfoxGetID\r\n");
		SigfoxGetID(&SigfoxModule);
		qSTimerSet(&Timeout, 2.0);
		//LED_SetVal();
	}

	if(SigfoxModule.RxReady){
		PrintString(&huart2,(uint8_t *)"ID : "); //qPrintString(UART_DEBUG, NULL, "ID : ");
		PrintString(&huart2,(uint8_t *)SigfoxModule.RxFrame); //qPrintString(UART_DEBUG, NULL, (const char *)SigfoxModule.rxFrame);
		PrintString(&huart2,(uint8_t *)"\n");//qPrintString(UART_DEBUG, NULL, "\n");
		fsm->NextState = State_SigfoxGetPAC;
		return qSM_EXIT_SUCCESS;
	}
	if(qSTimerExpired(&Timeout)) return qSM_EXIT_FAILURE;

	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SigfoxGetPAC(qSM_t *fsm){
	if(fsm->StateFirstEntry){
		PrintString(&huart2,(uint8_t *)"[STATE] : State_SigfoxGetPAC\r\n");
		SigfoxGetPAC(&SigfoxModule);
		qSTimerSet(&Timeout, 2.0);
	}

	if(SigfoxModule.RxReady){
		PrintString(&huart2,(uint8_t *)"PAC : ");
		PrintString(&huart2,(uint8_t *)SigfoxModule.RxFrame);
		PrintString(&huart2,(uint8_t *)"\n");
		fsm->NextState=State_SigfoxCheckChannels;
		return qSM_EXIT_SUCCESS;
	}
	if(qSTimerExpired(&Timeout))  return qSM_EXIT_FAILURE;

	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SigfoxCheckChannels(qSM_t *fsm){
	//uint8_t x,y;

	if(fsm->StateFirstEntry){
		PrintString(&huart2,(uint8_t *)"[STATE] : State_SigfoxCheckChannels\r\n");
		SigfoxAskChannels(&SigfoxModule);
		qSTimerSet(&Timeout, 1.5);
	}

	if(SigfoxModule.RxReady) {
		SigfoxModule.StatusFlag=SigfoxCheckChannels(&SigfoxModule);
	}

	if(SigfoxModule.StatusFlag!=SIGFOX_DEFAULT) fsm->NextState = (SigfoxModule.StatusFlag == SIGFOX_CHANN_NO_OK) ? State_SigfoxResetChannels : State_SigfoxSendPayload;

	if(qSTimerExpired(&Timeout) || SigfoxModule.StatusFlag == SIGFOX_PROCESS_FAILED) return qSM_EXIT_FAILURE;
	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SigfoxResetChannels(qSM_t *fsm){
	if(fsm->StateFirstEntry){
		PrintString(&huart2,(uint8_t *)"[STATE] : State_SigfoxResetChannels\r\n");
		qSTimerSet(&Timeout, 2.0);
		SigfoxResetChannels(&SigfoxModule);
	}
	if(SigfoxResponseReceived(&SigfoxModule,"OK") == SIGFOX_PROCESS_SUCCED) fsm->NextState = State_SigfoxSendPayload;

	if(qSTimerExpired(&Timeout) || SigfoxModule.StatusFlag == SIGFOX_PROCESS_FAILED)  return qSM_EXIT_FAILURE;

	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/
qSM_Status_t State_SigfoxSendPayload(qSM_t *fsm){

	static uint8_t DlContRx = 0, DlCont_Tx = 0, DLError = DL_SUCCESS;
	//static uint32_t DL_ReqPeriod;
	static qSTimer_t Timeout_DL = QSTIMER_INITIALIZER;

	//uint8_t vector[32];

	if(fsm->StateFirstEntry){
		qSTimerSet(&Timeout, 2.0);
		PrintString(&huart2,(uint8_t *)"[STATE] : State_SigfoxSendPayload\r\n");
		xtypes.xtype= iButton;
		iButton_Data.Type=xtypes.xtype;


		SigfoxModule.DownLink =0;
		if(SigfoxModule.DownLink){
			DLError = DL_SUCCESS;  					/*Indicar downlink */
			qSTimerSet(&Timeout_DL,DL_TIMEOUT);
			qSTimerDisarm(&Timeout);
		}
		else  {
			if(iButton_Data.periodic) ++DlCont_Tx;
			qSTimerSet(&Timeout, 4.0);
			qSTimerDisarm(&Timeout_DL);
			//qTraceVar(DlCont_Tx,QT_DEC);
		}

		/*SendMsgToSigfox*/
		SigfoxModule.StatusFlag=SigfoxSendMessage(&SigfoxModule,&iButton_Data,MAX_SIZE_IBUTTON_DATA,SigfoxModule.DownLink);

		if(SigfoxModule.StatusFlag == SIGFOX_PROCESS_FAILED) 	//qPrintString(UART_DEBUG,NULL,"Process Failed\r\n");
			SigfoxModule.StatusFlag=SIGFOX_DEFAULT;
	}

	if(SigfoxModule.RxReady){
		if(SigfoxResponseReceived(&SigfoxModule,"OK") == SIGFOX_PROCESS_SUCCED){
			DLError = (DLError == DL_SUCCESS)? DL_DiscriminateDownLink(&SigfoxModule): DL_UNKNOWN; /*Ingresa cuando sea downlink*/
			if(DL_TIME_OK == DLError){
				//sprintf(vector,"%d",SigfoxModule.UL_ReportTimeS); PrintString(&huart2,vector);
				// Escribir en la flash............... tiempo .. probar downlink
				DlContRx++; 					/*max 4*/
				//qTraceVar(DlContRx,QT_DEC);
				SigfoxModule.DownLink = 0;
			}
			PrintString(&huart2,(uint8_t *)SigfoxModule.TxFrame);
			PrintString(&huart2,(uint8_t *)"\r\n");
			HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);
			fsm->NextState = State_Blink ;
		}
	}
	if( qSTimerExpired(&Timeout_DL) ){
		DLError = DL_HEAD_ERROR;		/*Si Timeout error y pide DL de nuevo*/
		//qPrintString(UART_DEBUG,NULL,"Downlink Timeout\r\n");
		fsm->NextState = State_Sleep;
	}
	if(qSTimerExpired(&Timeout)) return qSM_EXIT_FAILURE;
	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/

/***********************************************************************************************/

qSM_Status_t State_Blink(qSM_t *fsm){
	static uint8_t contt=0;
	uint8_t n=2;
	if(fsm->PreviousState==State_SifgoxSaveParameters) n = 8;
	else n = 2;

	if(fsm->StateFirstEntry) {
		//qPrintString(UART_DEBUG, NULL, "[STATE] : State_Blink\r\n");
		HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);
		qSTimerSet(&Timeout,0.1);
	}

	if(qSTimerExpired(&Timeout)){
		contt++;
		if(contt<2*n){
			HAL_GPIO_WritePin(GPIOB, LED_Pin, !HAL_GPIO_ReadPin(GPIOB,LED_Pin) );
			//LED_PutVal(!LED_GetVal()); //SI
			qSTimerSet(&Timeout,0.1);
		}
		else{
			HAL_GPIO_WritePin(GPIOB, LED_Pin, GPIO_PIN_RESET);
			contt=0;
			fsm->NextState = State_Sleep;
		}
	}
	return qSM_EXIT_SUCCESS;
}
/***********************************************************************************************/
#ifdef MULTI_ZONE_SELECT
qSM_Status_t State_CheckOptions(qSM_t *fsm){
	qClock_t ElapsedTimeout;

	if(fsm->StateFirstEntry){
		//qPrintString(UART_DEBUG, NULL, "CheckOptions\r\n");
		qSTimerSet(&Timeout, 10.0);
	}
	if(((GPIO_PDD_GetPortDataInput(FPTD_BASE_PTR) & GPIO_PDD_PIN_6) != 0U) ? (bool)TRUE : (bool)FALSE) {  //cuando lo presiono no entra, cuando sueltro si
		ElapsedTimeout = qSTimerElapsed(&Timeout);

		if (ElapsedTimeout<3000){
			fsm->NextState = State_SigfoxCheckModule; //State_SigfoxChangeFrequency;
			return EXIT_SUCCESS;
		}
		else if (ElapsedTimeout>=3000 && ElapsedTimeout< 8000){
			SigfoxModule.frequency = UL_RCZ2;
			fsm->NextState = State_SigfoxChangeFrequency;

		}else if (ElapsedTimeout>=8000 && ElapsedTimeout<12000){
			SigfoxModule.frequency = UL_RCZ4;
			fsm->NextState = State_SigfoxChangeFrequency;
		}
	}
	if(qSTimerExpired(&Timeout)){
		SigfoxModule.frequency = UL_RCZ4;
		fsm->NextState = State_SigfoxChangeFrequency;
	}
	return EXIT_SUCCESS;
}
/***********************************************************************************************/
#endif
