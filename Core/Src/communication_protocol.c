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

void CP_receive_frame() {
	static uint8_t frame_buffer[412];
	static uint8_t frame_index = 0;

	uint8_t temp_byte;

	while(!ring_buffer_is_empty(&UART_rx_ring_buffer)) {
		__disable_irq();
		ring_buffer_get(&UART_rx_ring_buffer, &temp_byte);
		__enable_irq();

		if(temp_byte == CP_START_CHAR) {
			frame_index = 0;
		}

		if(frame_index < sizeof(frame_buffer)) {
			frame_buffer[frame_index++] = temp_byte;
		}

		if(temp_byte == CP_END_CHAR && frame_index >= CP_MIN_DATA_LEN) {
			Frame_t decoded_frame;
			if(CP_decode_received_frame(frame_buffer, frame_index, &decoded_frame)) {
				// TODO: Dekodowanie ramki przebiegło pomyślnie, napisać przetwarzanie
			}
		}
	}
}

uint8_t CP_decode_received_frame(uint8_t* frame_buffer, uint8_t frame_length, Frame_t* output) {

	output->sender_id = CP_2hex_to_byte((char)frame_buffer[1], (char)frame_buffer[2]);
	output->receiver_id = CP_2hex_to_byte((char)frame_buffer[3], (char)frame_buffer[4]);
	output->data_length = CP_2hex_to_byte((char)frame_buffer[5], (char)frame_buffer[6]);

	if(output->data_length > CP_MAX_DATA_LEN || output->data_length != frame_length - CP_MIN_DATA_LEN) {
		UART_SendText("INVALID DATA LENGTH\r\n");
		return 0;
	}

	for(uint8_t data_index = 0, decode_index = 7; data_index < output->data_length; data_index++) {
		output->data[data_index] = frame_buffer[decode_index++];
	}

	uint8_t crc_start_index = 6 + output->data_length;

	output->crc = CP_hex_to_word(frame_buffer[crc_start_index+1], frame_buffer[crc_start_index+2], frame_buffer[crc_start_index+3], frame_buffer[crc_start_index+4]);

	UART_SendText("DECODED FRAME\r\n");
	return 1;
}

uint8_t	CP_2hex_to_byte(char high, char low) {
	high = toupper(high);
	low = toupper(low);

	return ((high >= 'A' ? high - 'A' + 10 : high - '0') << 4) |
			(low >= 'A' ? low - 'A' + 10 : low - '0');
}

uint16_t CP_hex_to_word(char high1, char low1, char high2, char low2) {
	return (CP_2hex_to_byte(high1, low1) << 8) | CP_2hex_to_byte(high2, low2);
}

