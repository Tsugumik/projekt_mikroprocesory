/*
 * communication_protocol.c
 *
 *  Created on: Nov 3, 2024
 *      Author: Błażej Drozd
 */

#include "communication_protocol.h"

extern RingBuffer_t UART_rx_ring_buffer;

void TEST_received_data() {
	while(!ring_buffer_is_empty(&UART_rx_ring_buffer)) {
		uint8_t received_byte;

		__disable_irq();
		ring_buffer_get(&UART_rx_ring_buffer, &received_byte);
		__enable_irq();

		char msg[100];

		snprintf(msg, sizeof(msg), "STM32 RX: %c\r\n", (char)received_byte);

		UART_SendText(msg);
	}
}

