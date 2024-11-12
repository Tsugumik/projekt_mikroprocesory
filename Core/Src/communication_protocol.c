/*
 * communication_protocol.c
 *
 *  Created on: Nov 3, 2024
 *      Author: Błażej Drozd
 */

#include "communication_protocol.h"

extern RingBuffer_t UART_rx_ring_buffer;

// Do testowania, już nieaktualne
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
	static CP_FrameStatus_t read_state = CP_FS_WAIT_FOR_START_CHAR;
	static CP_Frame_t frame;
	static uint8_t data_count = 0;
	static uint8_t temp;

	while(!ring_buffer_is_empty(&UART_rx_ring_buffer)) {
		uint8_t rx_temp;

		__disable_irq();
		ring_buffer_get(&UART_rx_ring_buffer, &rx_temp);
		__enable_irq();

		if(rx_temp == CP_START_CHAR) {
			data_count = 0;
			read_state = CP_FS_WAIT_FOR_SENDER_BYTE1;
			continue;
		} else if(rx_temp == CP_END_CHAR && read_state != CP_FS_WAIT_FOR_END_CHAR) {
			read_state = CP_FS_WAIT_FOR_START_CHAR;
			continue;
		} else if(read_state == CP_FS_WAIT_FOR_START_CHAR) {
			// Ignoruj znaki jeśli oczekuje się znaku rozpoczęcia ramki
			continue;
		} else if(rx_temp == CP_END_CHAR && read_state == CP_FS_WAIT_FOR_END_CHAR) {
			// RAMKA ODEBRANA, przetwarzaj
			if(CP_validate_frame(&frame) == V_OK) {
				CP_Command_t cmd;
				if(CP_parse_command(&frame, &cmd) == COMMAND_OK) {
					CP_CMD_execute(&cmd, frame.sender_id);
					// TODO: Dodaj obsługę błędów komendy!
					CP_send_status_frame(frame.sender_id);
				} else {
					CP_send_error_frame(COMMAND_PARSE_ERROR, frame.sender_id);
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
			} else {
				read_state = CP_FS_WAIT_FOR_START_CHAR;
			}

			// Po przetworzeniu czekaj na następną
			read_state = CP_FS_WAIT_FOR_START_CHAR;
			continue;
		}

		switch(read_state) {
			case CP_FS_WAIT_FOR_SENDER_BYTE1:
				if(CP_hex_to_byte(rx_temp, &frame.sender_id) == HEX_OK) {
					frame.sender_id <<= 4;
					read_state = CP_FS_WAIT_FOR_SENDER_BYTE2;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_SENDER_BYTE2:
				if(CP_hex_to_byte(rx_temp, &temp) == HEX_OK) {
					frame.sender_id |= temp;
					read_state = CP_FS_WAIT_FOR_RECEIVER_BYTE1;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_RECEIVER_BYTE1:
				if(CP_hex_to_byte(rx_temp, &frame.receiver_id) == HEX_OK) {
					frame.receiver_id <<= 4;
					read_state = CP_FS_WAIT_FOR_RECEIVER_BYTE2;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_RECEIVER_BYTE2:
				if(CP_hex_to_byte(rx_temp, &temp) == HEX_OK) {
					frame.receiver_id |= temp;
					read_state = CP_FS_WAIT_FOR_DATALEN_BYTE1;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_DATALEN_BYTE1:
				if(CP_hex_to_byte(rx_temp, &frame.data_length) == HEX_OK) {
					frame.data_length <<= 4;
					read_state = CP_FS_WAIT_FOR_DATALEN_BYTE2;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_DATALEN_BYTE2:
				if(CP_hex_to_byte(rx_temp, &temp) == HEX_OK) {
					frame.data_length |= temp;
					if(frame.data_length > CP_MAX_DATA_LEN) {
						read_state = CP_FS_WAIT_FOR_START_CHAR;
						break;
					}
					read_state = frame.data_length > 0 ? CP_FS_READ_DATA : CP_FS_WAIT_FOR_CRC_BYTE1;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_READ_DATA:
				if(rx_temp == CP_ENCODE_CHAR) {
					read_state = CP_FS_DECODE_DATA;
				} else {
					frame.data[data_count] = rx_temp;
					data_count++;
					if(data_count == frame.data_length) {
						read_state = CP_FS_WAIT_FOR_CRC_BYTE1;
						break;
					}
				}

				break;
			case CP_FS_DECODE_DATA:
				if(CP_decode_byte(rx_temp, &frame.data[data_count]) == DECODE_OK) {
					data_count++;
					if(data_count == frame.data_length) {
						read_state = CP_FS_WAIT_FOR_CRC_BYTE1;
						break;
					} else {
						read_state = CP_FS_READ_DATA;
					}
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_CRC_BYTE1:
				if(CP_hex_to_2bytes(rx_temp, &frame.crc) == HEX_OK) {
					frame.crc <<= 4;
					read_state = CP_FS_WAIT_FOR_CRC_BYTE2;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_CRC_BYTE2:
				if(CP_hex_to_byte(rx_temp, &temp) == HEX_OK) {
					frame.crc |= temp;
					frame.crc <<= 4;
					read_state = CP_FS_WAIT_FOR_CRC_BYTE3;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_CRC_BYTE3:
				if(CP_hex_to_byte(rx_temp, &temp) == HEX_OK) {
					frame.crc |= temp;
					frame.crc <<= 4;
					read_state = CP_FS_WAIT_FOR_CRC_BYTE4;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_WAIT_FOR_CRC_BYTE4:
				if(CP_hex_to_byte(rx_temp, &temp) == HEX_OK) {
					frame.crc |= temp;
					read_state = CP_FS_WAIT_FOR_END_CHAR;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			default:
				break;
		}
	}
}


CP_StatusCode_t CP_hex_to_byte(char ch, uint8_t* out) {
	if(!((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F'))) {
		return HEX_WRONG_CHAR;
	}

	*out = (ch >= 'A' ? ch - 'A' + 10 : ch - '0');

	return HEX_OK;
}

CP_StatusCode_t CP_hex_to_2bytes(char ch, uint16_t* out) {
	if(!((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F'))) {
		return HEX_WRONG_CHAR;
	}

	*out = (ch >= 'A' ? ch - 'A' + 10 : ch - '0');

	return HEX_OK;
}

CP_StatusCode_t CP_validate_frame(CP_Frame_t* frame) {
	uint16_t calculated_crc = crc16_ansi(frame->data, frame->data_length);
	if(calculated_crc != frame->crc) return V_CRC_ERROR;

	return V_OK;
}

void CP_byte_to_2hex(uint8_t byte, uint8_t* out) {
	sprintf((char*)out, "%02X", byte);
}

void CP_word_to_hex(uint16_t word, uint8_t* out) {
	sprintf((char*)out, "%04X", word);
}

CP_TX_StatusCode_t CP_gen_frame(const char* data, uint8_t receiver, CP_Frame_t* out) {
	size_t datalen = strlen(data);

	if(datalen != CP_MAX_DATA_LEN);

	// Kopiowanie danych do ramki
	for(size_t i=0; i<datalen; i++) {
		out->data[i] = data[i];
	}

	// Ustawianie pozostałych parametrów ramki
	out->data_length = datalen;
	out->sender_id = CP_STM_REC_ID;
	out->receiver_id = receiver;

	// Liczenie sumy kontrolnej
	out->crc = crc16_ansi(out->data, out->data_length);

	// Ramka gotowa do zakodowania i przesłania
	return GEN_OK;
}

CP_TX_StatusCode_t CP_send_frame(CP_Frame_t* frame) {
	uint8_t response_buff[CP_MAX_DATA_LEN*2 + CP_MIN_FRAME_LEN];
	uint8_t response_index = 0;

	response_buff[response_index++] = CP_START_CHAR;

	CP_byte_to_2hex(frame->sender_id, &response_buff[response_index]);
	response_index += 2;

	CP_byte_to_2hex(frame->receiver_id, &response_buff[response_index]);
	response_index += 2;

	CP_byte_to_2hex(frame->data_length, &response_buff[response_index]);
	response_index += 2;

	uint16_t data_counter = 0;
	uint8_t temp_byte;

	while(data_counter < frame->data_length) {
		temp_byte = frame->data[data_counter++];

		switch(temp_byte) {
			case CP_START_CHAR:
				response_buff[response_index++] = CP_ENCODE_CHAR;
				response_buff[response_index++] = CP_START_CODE_CHAR;
				break;
			case CP_END_CHAR:
				response_buff[response_index++] = CP_ENCODE_CHAR;
				response_buff[response_index++] = CP_END_CODE_CHAR;
				break;
			case CP_ENCODE_CHAR:
				response_buff[response_index++] = CP_ENCODE_CHAR;
				response_buff[response_index++] = CP_ENCODE_CODE_CHAR;
				break;
			default:
				response_buff[response_index++] = temp_byte;
				break;
		}
	}

	CP_word_to_hex(frame->crc, &response_buff[response_index]);
	response_index += 4;

	response_buff[response_index] = CP_END_CHAR;

	UART_SendData(response_buff, response_index+1);

	return SEND_OK;
}

void CP_send_status_frame(uint8_t receiver) {
	CP_Frame_t status_frame;
	CP_gen_frame("A0", receiver, &status_frame);
	CP_send_frame(&status_frame);
}

void CP_send_error_frame(CP_StatusCode_t status, uint8_t receiver) {
	CP_Frame_t error_frame;

	uint8_t frame_data_buff[CP_ERROR_FRAME_LEN + 1];

	frame_data_buff[0] = 'F';
	CP_byte_to_2hex(status, &frame_data_buff[1]);

	frame_data_buff[CP_ERROR_FRAME_LEN] = '\0';

	CP_gen_frame((const char*)frame_data_buff, receiver, &error_frame);
	CP_send_frame(&error_frame);
}

CP_StatusCode_t	CP_parse_command(CP_Frame_t* frame, CP_Command_t* out) {
	if(frame->data_length < 1) {
		return COMMAND_EMPTY;
	}

	// Bufor na komendę

	char command[CP_MAX_DATA_LEN + 1];

	for(uint8_t i=0; i < frame->data_length; i++) {
		command[i] = frame->data[i];
	}

	command[frame->data_length] = '\0';

	// Poszukiwanie pozycji znaku (
	const char* start_args = strchr(command, CP_ARG_START_CHAR);

	if(!start_args) {
		return COMMAND_PARSE_ERROR;
	}

	// Przekopiowanie nazwy do struktury komendy
	strncpy(out->name, command, start_args - command);
	out->name[start_args - command] = '\0';

	// Poszukiwanie pozycji znaku )
	const char* end_args = strchr(start_args, CP_ARG_END_CHAR);

	if(!end_args) {
		return COMMAND_PARSE_ERROR;
	}

	char args_copy[CP_MAX_DATA_LEN + 1];
	strncpy(args_copy, start_args + 1, end_args - start_args - 1);
	args_copy[end_args - start_args - 1] = '\0';

	char* token = strtok(args_copy, CP_ARG_SPLIT_CHAR);
	out->arg_count = 0;

	while(token && out->arg_count < CP_MAX_COMMAND_ARG) {
		out->arguments[out->arg_count++] = strdup(token);
		token = strtok(NULL, CP_ARG_SPLIT_CHAR);
	}

	return COMMAND_OK;
}

void CP_CMD_execute(CP_Command_t* cmd, uint8_t receiver) {
	if(strcmp(cmd->name, "LED") == 0) {
		if(cmd->arg_count == 0) {
			TOGGLELED();
			CP_send_status_frame(receiver);
		} else if(cmd->arg_count == 1){
			if(strcmp(cmd->arguments[0], "0") == 0) {
				SETLED(0);
				CP_send_status_frame(receiver);
			} else if(strcmp(cmd->arguments[0], "1") == 0) {
				SETLED(1);
				CP_send_status_frame(receiver);
			} else {
				CP_send_error_frame(COMMAND_ARGUMENT_TYPE_ERROR, receiver);
			}
		} else {
			CP_send_error_frame(COMMAND_ARGUMENT_ERROR, receiver);
		}
	} else if(strcmp(cmd->name, "GETOK") == 0) {
		if(cmd->arg_count != 0) {
			CP_send_error_frame(COMMAND_ARGUMENT_ERROR, receiver);
		} else {
			CP_send_status_frame(receiver);
		}
	} else {
		CP_send_error_frame(COMMAND_UNKNOWN, receiver);
	}

	// Zwalnianie przydzielonej pamięci przez strdup po jej wykonaniu
	for (uint8_t i = 0; i < cmd->arg_count; i++) {
		free(cmd->arguments[i]);
	}
}

CP_StatusCode_t CP_decode_byte(uint8_t in, uint8_t* out) {
	switch(in) {
		case CP_START_CODE_CHAR:
			*out = CP_START_CHAR;
			break;
		case CP_END_CODE_CHAR:
			*out = CP_END_CHAR;
			break;
		case CP_ENCODE_CODE_CHAR:
			*out = CP_ENCODE_CHAR;
			break;
		default:
			return DECODE_ERROR;
	}

	return DECODE_OK;
}
