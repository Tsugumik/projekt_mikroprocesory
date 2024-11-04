/*
 * uart_handler.h
 *
 *  Created on: Nov 3, 2024
 *      Author: drozd
 */

#ifndef INC_UART_HANDLER_H_
#define INC_UART_HANDLER_H_

#include "main.h"
#include "usart.h"
#include "ring_buffer.h"
#include "communication_protocol.h"
#include "string.h"

#define UART_RX_BUFF_LEN 750
#define UART_TX_BUFF_LEN 750

void UART_SendData(uint8_t*, uint16_t);
void UART_SendText(const char*);

#endif /* INC_UART_HANDLER_H_ */
