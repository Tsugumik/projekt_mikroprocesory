/*
 * communication_protocol.c
 *
 *  Created on: Nov 3, 2024
 *      Author: Błażej Drozd
 */

#include "communication_protocol.h"

extern RingBuffer_t UART_rx_ring_buffer;
extern RingBufferSensor_RawData_t SENSOR_ring_buffer;
extern uint32_t read_interval;
extern I2C_HandleTypeDef hi2c1;
extern SCREEN_TempUnits_t tempUnit;

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
			// Otrzymano znak początku ramki
			data_count = 0;
			read_state = CP_FS_WAIT_FOR_SENDER_BYTE1;
			continue;
		} else if(rx_temp == CP_END_CHAR && read_state != CP_FS_WAIT_FOR_END_CHAR) {
			// Otrzymano znak końca ramki, który nie był oczekiwany
			read_state = CP_FS_WAIT_FOR_START_CHAR;
			continue;
		} else if(read_state == CP_FS_WAIT_FOR_START_CHAR) {
			// Ignoruj znaki jeśli oczekuje się znaku rozpoczęcia ramki
			continue;
		} else if(rx_temp == CP_END_CHAR && read_state == CP_FS_WAIT_FOR_END_CHAR) {
			// RAMKA ODEBRANA, przetwarzaj
			if(CP_validate_frame(&frame) == V_OK) {
				// Sprawdzanie CRC
				CP_Command_t cmd;
				if(CP_parse_command(&frame, &cmd) == COMMAND_OK) {
					// Parsowanie komendy

					// I na końcu jej wykonanie
					CP_CMD_execute(&cmd, frame.sender_id);
					// TODO: Dodaj obsługę błędów komendy!
				} else {
					CP_send_error_frame(RFS_CMD_PARSE_ERROR, frame.sender_id);
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

					// Sprawdzanie czy odbiorca się zgadza
					if(frame.receiver_id != CP_STM_REC_ID) {
						read_state = CP_FS_WAIT_FOR_START_CHAR;
						break;
					}

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

					// Jeśli zadeklarowano 0 danych, odrzuć ramkę
					read_state = frame.data_length > 0 ? CP_FS_READ_DATA : CP_FS_WAIT_FOR_CRC_BYTE1;
				} else {
					read_state = CP_FS_WAIT_FOR_START_CHAR;
				}
				break;
			case CP_FS_READ_DATA:
				// Odbiór danych
				if((rx_temp >= 0x41 && rx_temp <= 0x5A) || (rx_temp >= 0x30 && rx_temp <= 0x39) || (rx_temp == 0x28 || rx_temp == 0x29 || rx_temp == 0x2C)) {
					/*
					 * Dopuszczone są tylko znaki
					 * - A do Z (od 0x41 do 0x5A)
					 * - 0 do 9 (od 0x30 do 0x39)
					 * - "(" (0x28)
					 * - ")" (0x29)
					 * - "," (0x2C)
					 *
					 * Jeśli pojawi się bajt o innej wartości, ramka jest
					 * odrzucana
					 */
					frame.data[data_count] = rx_temp;
					data_count++;

					if(data_count == frame.data_length) read_state = CP_FS_WAIT_FOR_CRC_BYTE1;
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

	if(datalen > CP_MAX_DATA_LEN) {
		return GEN_ERROR;
	}

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

CP_TX_StatusCode_t CP_createFrame_measurement_interval(uint8_t receiver, CP_Frame_t* out) {
	char buff[10];
	sprintf(buff, "0%08lX", read_interval);

	CP_gen_frame((char*)buff, receiver, out);

	return GEN_OK;
}

CP_TX_StatusCode_t CP_createFrame_oldest_index(uint8_t receiver, CP_Frame_t* out) {
	char buff[6];
	sprintf(buff, "0%03X", ring_bufferSensor_get_oldest_index(&SENSOR_ring_buffer));

	CP_gen_frame((char*)buff, receiver, out);

	return GEN_OK;
}

CP_TX_StatusCode_t	CP_createFrame_latest_sensor_data(uint8_t receiver, CP_ReturnSensorData_t dataType, CP_Frame_t* out) {
	char buff[9];

	Sensor_RawData_t rawData;

	switch(dataType) {
		case TEMPERATURE:
			__disable_irq();
			if(ring_bufferSensor_get_latest(&SENSOR_ring_buffer, &rawData)) {
				__enable_irq();
				sprintf(buff, "0%01X%05lX", (uint8_t)dataType, rawData.temperature);
				CP_gen_frame((char*)buff, receiver, out);
				return GEN_OK;
			} else {
				__enable_irq();
				return GEN_ERROR;
			}
		case HUMIDITY:
			__disable_irq();
			if(ring_bufferSensor_get_latest(&SENSOR_ring_buffer, &rawData)) {
				__enable_irq();
				sprintf(buff, "0%01X%05lX", (uint8_t)dataType, rawData.humidity);
				CP_gen_frame((char*)buff, receiver, out);
				return GEN_OK;
			} else {
				__enable_irq();
				return GEN_ERROR;
			}
		default:
			return GEN_ERROR;
	}

}

CP_TX_StatusCode_t	CP_createFrame_archive_sensor_data(uint8_t receiver, CP_ReturnSensorData_t dataType, uint16_t index, CP_Frame_t* out) {
	char buff[13];

	Sensor_RawData_t rawData;

	switch(dataType) {
		case TEMPERATURE:
			__disable_irq();
			if(ring_bufferSensor_get_at_index(&SENSOR_ring_buffer, index, &rawData)) {
				__enable_irq();
				sprintf(buff, "0%03X%01X%05lX", index, (uint8_t)dataType, rawData.temperature);
				CP_gen_frame((char*)buff, receiver, out);
				return GEN_OK;
			} else {
				__enable_irq();
				return GEN_ERROR;
			}
		case HUMIDITY:
			__disable_irq();
			if(ring_bufferSensor_get_at_index(&SENSOR_ring_buffer, index, &rawData)) {
				__enable_irq();
				sprintf(buff, "0%03X%01X%05lX", index, (uint8_t)dataType, rawData.humidity);
				CP_gen_frame((char*)buff, receiver, out);
				return GEN_OK;
			} else {
				__enable_irq();
				return GEN_ERROR;
			}
		default:
			return GEN_ERROR;
	}

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

		response_buff[response_index++] = temp_byte;
	}

	CP_word_to_hex(frame->crc, &response_buff[response_index]);
	response_index += 4;

	response_buff[response_index] = CP_END_CHAR;

	UART_SendData(response_buff, response_index+1);

	return SEND_OK;
}

void CP_send_status_frame(uint8_t receiver) {
	CP_Frame_t status_frame;
	CP_gen_frame("10", receiver, &status_frame);
	CP_send_frame(&status_frame);
}

void CP_send_error_frame(CP_ReturnFrameStatus_t status, uint8_t receiver) {
	CP_Frame_t error_frame;

	uint8_t frame_data_buff[CP_ERROR_FRAME_LEN + 1];

	frame_data_buff[0] = '2';
	CP_byte_to_2hex(status, &frame_data_buff[1]);

	frame_data_buff[CP_ERROR_FRAME_LEN] = '\0';

	CP_gen_frame((const char*)frame_data_buff, receiver, &error_frame);
	CP_send_frame(&error_frame);
}

CP_StatusCode_t CP_is_command_name_valid(const char* name) {

	for(size_t i = 0; i < strlen(name); i++) {
		if(!(name[i] >= 0x41 && name[i] <= 0x5A)) return COMMAND_NAME_ERROR;
	}

	return COMMAND_NAME_OK;
}

CP_StatusCode_t	CP_parse_command(CP_Frame_t* frame, CP_Command_t* out) {
	if(frame->data_length < 1) {
		return COMMAND_EMPTY;
	}

	// Konwertowanie danych na string
	char command[CP_MAX_DATA_LEN + 1];
	memcpy(command, frame->data, frame->data_length);
	command[frame->data_length] = '\0';

	// Poszukiwanie pozycji znaku (
	const char* start_args = strchr(command, CP_ARG_START_CHAR);

	if(!start_args) {
		// Jeśli nie znaleziono - komenda niepoprawna
		return COMMAND_PARSE_ERROR;
	}

	// Sprawdzanie długości komendy
	size_t name_len = start_args - command;
	if(name_len > CP_MAX_CMD_LEN || name_len == 0) {
		return COMMAND_NAME_ERROR;
	}

	// Przekopiowanie nazwy do struktury komendy
	strncpy(out->name, command, name_len);
	out->name[start_args - command] = '\0';

	// Sprawdzanie nazwy (wielkości liter)
	if(!CP_is_command_name_valid(out->name)) {
		return COMMAND_NAME_ERROR;
	}

	// Poszukiwanie pozycji znaku )
	const char* end_args = strchr(start_args, CP_ARG_END_CHAR);

	if(!end_args) {
		return COMMAND_PARSE_ERROR;
	}

	size_t args_len = end_args - start_args - 1;
	if(args_len > CP_MAX_DATA_LEN) {
		return COMMAND_PARSE_ERROR;
	}

	char args_copy[CP_MAX_DATA_LEN + 1];
	strncpy(args_copy, start_args + 1, args_len);

	args_copy[args_len] = '\0';

	char* token;
	char* rest = args_copy;

	out->arg_count = 0;

	while((token = strtok_r(rest, CP_ARG_SPLIT_CHAR, &rest)) && out->arg_count < CP_MAX_ARG_COUNT) {
		out->arguments[out->arg_count] = strdup(token);

		if(out->arguments[out->arg_count] == NULL) {
			// Obsługa błędu alokacji pamięci
			CP_Free_mem(out);

			return COMMAND_PARSE_ERROR;
		}

		out->arg_count++;
	}

	if(token != NULL) {
		// Zbyt wiele argumentów
		CP_Free_mem(out);

		return COMMAND_PARSE_ERROR;
	}

	return COMMAND_OK;
}

void CP_CMD_execute(CP_Command_t* cmd, uint8_t receiver) {
	if(strcmp(cmd->name, "GETOK") == 0) {
		if(cmd->arg_count == 0) {
			CP_send_status_frame(receiver);
		} else {
			CP_send_error_frame(RFS_COMMAND_ARGUMENT_COUNT_ERROR, receiver);
		}
	} else if(strcmp(cmd->name, "SETINTERVAL") == 0) {
		if(cmd->arg_count == 1) {
			uint32_t interval = 0;
			uint8_t result;

			uint8_t i = 0;

			uint8_t hex_len = strlen(cmd->arguments[0]);

			if(hex_len > 8) {
				CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
				CP_Free_mem(cmd);
				return;
			} else if(hex_len == 1 && cmd->arguments[0][i] == '0') {
				CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
				CP_Free_mem(cmd);
				return;
			}

			if((CP_hex_to_byte(cmd->arguments[0][i], &result)) != HEX_OK) {
				CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
				CP_Free_mem(cmd);
				return;
			} else {
				interval |= result;
			}

			i++;

			for(;i < hex_len; i++) {

				if((CP_hex_to_byte(cmd->arguments[0][i], &result)) != HEX_OK) {
					CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
					CP_Free_mem(cmd);
					return;
				}

				interval <<= 4;
				interval |= result;
			}

			read_interval = interval;
			CP_send_status_frame(receiver);
		} else {
			CP_send_error_frame(RFS_COMMAND_ARGUMENT_COUNT_ERROR, receiver);
		}
	} else if(strcmp(cmd->name, "SETSCREEN") == 0) {
		if(cmd->arg_count == 1) {
			// TODO: Ustawiać flagę statusu wyświetlacza
			switch(cmd->arguments[0][0]) {
				case '0':
					SCREEN_SetStatus(SCREEN_OFF);
					CP_send_status_frame(receiver);
				break;
				case '1':
					SCREEN_SetStatus(SCREEN_ON);
					CP_send_status_frame(receiver);
				break;
				default:
					CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
					break;
			}
		} else {
			CP_send_error_frame(RFS_COMMAND_ARGUMENT_COUNT_ERROR, receiver);
		}
	} else if(strcmp(cmd->name, "SETTEMPUNIT") == 0) {
		if(cmd->arg_count == 1) {
			// TODO: Ustawiać flagę statusu wyświetlacza
			switch(cmd->arguments[0][0]) {
				case '0':
					tempUnit = SCREEN_TempUnit_C;
					SCREEN_Update();
					CP_send_status_frame(receiver);
				break;
				case '1':
					tempUnit = SCREEN_TempUnit_F;
					SCREEN_Update();
					CP_send_status_frame(receiver);
				break;
				case '2':
					tempUnit = SCREEN_TempUnit_K;
					SCREEN_Update();
					CP_send_status_frame(receiver);
					break;
				default:
					CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
					break;
			}
		} else {
			CP_send_error_frame(RFS_COMMAND_ARGUMENT_COUNT_ERROR, receiver);
		}
	} else if(strcmp(cmd->name, "GETINTERVAL") == 0) {
		if(cmd->arg_count == 0) {
			CP_Frame_t frame;
			CP_createFrame_measurement_interval(receiver, &frame);
			CP_send_frame(&frame);
		} else {
			CP_send_error_frame(RFS_COMMAND_ARGUMENT_COUNT_ERROR, receiver);
		}
	} else if(strcmp(cmd->name, "GETOLDINDEX") == 0) {
		if(cmd->arg_count == 0) {
			CP_Frame_t frame;
			CP_createFrame_oldest_index(receiver, &frame);
			CP_send_frame(&frame);
		} else {
			CP_send_error_frame(RFS_COMMAND_ARGUMENT_COUNT_ERROR, receiver);
		}
	} else if(strcmp(cmd->name, "GETDATA") == 0) {
		if(cmd->arg_count == 1) {
			CP_Frame_t frame;
			switch(cmd->arguments[0][0]) {
				case '0':
					if(CP_createFrame_latest_sensor_data(receiver, TEMPERATURE, &frame)) {
						CP_send_frame(&frame);
						break;
					} else {
						CP_send_error_frame(RFS_BUFFER_EMPTY, receiver);
						break;
					}
				case '1':
					if(CP_createFrame_latest_sensor_data(receiver, HUMIDITY, &frame)) {
						CP_send_frame(&frame);
						break;
					} else {
						CP_send_error_frame(RFS_BUFFER_EMPTY, receiver);
						break;
					}
				default:
					CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
					break;
			}
		} else {
			CP_send_error_frame(RFS_COMMAND_ARGUMENT_COUNT_ERROR, receiver);
		}
	} else if(strcmp(cmd->name, "GETARCHIVE") == 0) {
		if(cmd->arg_count == 2) {
			uint16_t index = 0;
			uint8_t result;
			uint8_t i = 0;

			uint8_t hex_len = strlen(cmd->arguments[0]);

			if(hex_len > 3) {
				CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
				CP_Free_mem(cmd);
				return;
			}

			if((CP_hex_to_byte(cmd->arguments[0][i], &result)) != HEX_OK) {
				CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
				CP_Free_mem(cmd);
				return;
			} else {
				index |= result;
			}

			i++;

			for(;i < hex_len; i++) {

				if((CP_hex_to_byte(cmd->arguments[0][i], &result)) != HEX_OK) {
					CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
					CP_Free_mem(cmd);
					return;
				}

				index <<= 4;
				index |= result;
			}

			// Indeks zdekodowany

			switch(cmd->arguments[0][0]) {
				CP_Frame_t frame;
				case '0':
					if(CP_createFrame_archive_sensor_data(receiver, TEMPERATURE, index, &frame)) {
						CP_send_frame(&frame);
						break;
					} else {
						CP_send_error_frame(RFS_INDEX_ERROR, receiver);
						break;
					}
				case '1':
					if(CP_createFrame_archive_sensor_data(receiver, HUMIDITY, index, &frame)) {
						CP_send_frame(&frame);
						break;
					} else {
						CP_send_error_frame(RFS_INDEX_ERROR, receiver);
						break;
					}
				default:
					CP_send_error_frame(RFS_COMMAND_ARGUMENT_INVALID, receiver);
					break;
			}
		} else {
			CP_send_error_frame(RFS_COMMAND_ARGUMENT_COUNT_ERROR, receiver);
		}
	} else {
		CP_send_error_frame(RFS_COMMAND_UNKNOWN, receiver);
		CP_Free_mem(cmd);
		return;
	}

	CP_Free_mem(cmd);
}

void CP_Free_mem(CP_Command_t* cmd) {
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


