/*
 * Uart.h
 *
 *  Created on: 01/04/2019
 *      Author: julian
 */

#ifndef UART_H_
#define UART_H_

#include <Common/Globals.h>
#include "main.h"
#include "stm32f070x6.h"
#include "stm32f0xx.h"

#define MAX_BUFFER_SIZE_RX	32
/***************UART*****************************/
typedef struct {
uint8_t Buffer[MAX_BUFFER_SIZE_RX];
uint8_t Index;
uint8_t Data;
uint8_t Ready;
uint8_t Downlink;
}UART_BufferData_t;

extern UART_BufferData_t UART_RX;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);



#endif /* UART_H_ */
