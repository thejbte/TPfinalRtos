/*
 * State.h
 *
 *  Created on: 1/02/2019
 *      Author: julian
 */

#ifndef STATE_STATE_H_
#define STATE_STATE_H_

#include "../Inc/Common/Globals.h"


void State_Failure(qSM_t *fsm);
qSM_Status_t State_SigfoxInit(qSM_t *fsm);
qSM_Status_t State_SigfoxCheckModule(qSM_t *fsm);
qSM_Status_t State_SigfoxCheckChannels(qSM_t *fsm);
qSM_Status_t State_SigfoxResetChannels(qSM_t *fsm);
qSM_Status_t State_SigfoxSendPayload(qSM_t *fsm);
qSM_Status_t State_SigfoxChangeFrequency(qSM_t *fsm);
qSM_Status_t State_SifgoxSaveParameters(qSM_t *fsm);
qSM_Status_t State_SigfoxGetID(qSM_t *fsm);
qSM_Status_t State_SigfoxGetPAC(qSM_t *fsm);

qSM_Status_t State_Blink(qSM_t *fsm);
qSM_Status_t State_CheckOptions(qSM_t *fsm);
qSM_Status_t State_Sleep(qSM_t *fsm);

qSM_Status_t State_GPSWaitingPosition(qSM_t *fsm);
//qSM_Status_t State_WIFIConfig(qSM_t *fsm);
qSM_Status_t State_UserConfig(qSM_t *fsm);
qSM_Status_t State_GetFlash(qSM_t *fsm);
//    qSM_Status_t State_ADC(qSM_t *fsm);
qSM_Status_t State_I2C_Battery(qSM_t *fsm);
qSM_Status_t State_SigfoxChangeFrequencyDL(qSM_t *fsm) ;
void BeforeAny(qSM_t *fsm);


#endif /* STATE_STATE_H_ */
