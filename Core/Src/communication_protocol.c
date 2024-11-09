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
	static uint8_t frame_buffer[CP_MAX_DATA_LEN*2 + CP_MIN_FRAME_LEN];
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

		if(temp_byte == CP_END_CHAR) {
			CP_Frame_t decoded_frame;
			CP_RX_StatusCode_t decode_status;
			char msg[100];
			if((decode_status = CP_decode_received_frame(frame_buffer, frame_index, &decoded_frame)) == DR_OK) {

				CP_RX_StatusCode_t validation_status;

				if((validation_status = CP_validate_frame(&decoded_frame)) == V_OK) {
					// TODO: Ramka jest całkowicie prawidłowa i można przetwarzać przesłane dane
				} else {
					// Odesłanie błędu o błędnej sumie kontrolnej
					return;
				}

				return;
			} else {
				// Odesłanie błędu o błędzie dekodowania ramki

				/*
				 * Jeśli status to DR_RECEIVER_DIFF
				 * nie odpowiadaj na ramkę
				 */

				return;
			}
		}
	}
}

CP_RX_StatusCode_t CP_decode_received_frame(uint8_t* frame_buffer, uint8_t frame_length, CP_Frame_t* output) {
	/*
	 * Jeśli długość wczytanej ramki jest mniejsza
	 * niż jej minimalna długość
	 * zwróć błąd
	 */
	if(frame_length < CP_MIN_FRAME_LEN) return DR_FRAME_TOO_SHORT;

	/*
	 * Przełączenie w tryb czytania danych
	 */
	CP_DecodeState_t decode_state = DS_READ_DATA;

	/*
	 * Ilość wczytanych i zdekodowanych znaków
	 */
	uint16_t data_counter = 0;

	/*
	 * Dane zaczynają się od 7 pozycji
	 */
	uint16_t data_index = CP_DATA_START_IDX;

	/*
	 * Wczytywanie podstawowych informacji z ramki
	 * Jeśli znaleziono w nich nieprawidłowe znaki
	 * należy zwrócić błąd.
	 */
	if(CP_2hex_to_byte((char)frame_buffer[1], (char)frame_buffer[2], &output->sender_id) 	== HEX_WRONG_CHAR) return HEX_WRONG_CHAR;
	if(CP_2hex_to_byte((char)frame_buffer[3], (char)frame_buffer[4], &output->receiver_id) 	== HEX_WRONG_CHAR) return HEX_WRONG_CHAR;
	if(CP_2hex_to_byte((char)frame_buffer[5], (char)frame_buffer[6], &output->data_length) 	== HEX_WRONG_CHAR) return HEX_WRONG_CHAR;


	/*
	 * Jeśli id odbiorcy nie zgadza się z id płytki
	 * należy przestać dekodować ramkę.
	 */
	if(output->receiver_id != CP_STM_REC_ID) return DR_RECEIVER_DIFF;

	/*
	 * Jeśli odbiorca równa się nadawcy
	 * należy zwrócić błąd
	 */
	if(output->receiver_id == output->sender_id) return DR_RECEIVER_EQUAL_SENDER;

	/*
	 * Jeśli zadeklarowano więcej niż 200 znaków w polu danych
	 * należy zwrócić błąd
	 */
	if(output->data_length > 200) return DR_WRONG_DATA_LEN_PROVIDED_IN_FRAME;

	// Teraz można przystąpić do wczytywania danych

	/*
	 * Program przechodzi przez dane i zwiększa data_counter o 1
	 * z każdym zdekodowanym znakiem.
	 *
	 * Znak zakodowany np. sekwencja \1 jest traktowana jako jeden znak
	 * pomimo że zajmuje dwa miejsca w zakodowanej ramce.
	 *
	 * Zadeklarowana długość danych dotyczy danych niekodowanych.
	 */

	/*
	 * Maksymalny indeks do jakiego można czytać dane.
	 * Dalej znajduje się już CRC.
	 */
	uint16_t max_data_index = frame_length - CP_CRC_LEN - 1;

	while(data_counter < output->data_length && data_index < max_data_index) {
		uint8_t temp_byte = frame_buffer[data_index];

		if(decode_state == DS_READ_DATA) {
			/*
			 * Jeśli podczas czytania danych napotkano
			 * znak kodowania
			 * należy go pominąć i przełączyć się
			 * w tryb dekodowania danych
			 */
			if(temp_byte == CP_ENCODE_CHAR) {
				decode_state = DS_DECODE_NEXT_DATA;
				data_index++;
				continue;
			}

			output->data[data_counter] = temp_byte;
			data_index++;
			data_counter++;
			continue;
		} else if(decode_state == DS_DECODE_NEXT_DATA) {
			/*
			 * W trybie dekodowania danych oczekujemy trzech
			 * znaków: 1, 2 lub 3.
			 * Jeśli otrzymamy inny, jest to błędnie
			 * zakodowana ramka.
			 */
			uint8_t decoded_byte;
			switch(temp_byte) {
				case CP_START_CODE_CHAR:
					decoded_byte = CP_START_CHAR;
					break;
				case CP_END_CODE_CHAR:
					decoded_byte = CP_END_CHAR;
					break;
				case CP_ENCODE_CODE_CHAR:
					decoded_byte = CP_ENCODE_CODE_CHAR;
					break;
				default:
					// Zwrócenie błędu o nieprawidłowym znaku kodowania
					return DR_WRONG_NEXT_CHARACTER;

			}

			// Zapisanie zdekodowanego znaku do ramki wyjściowej
			output->data[data_counter] = decoded_byte;
			data_counter++;
			data_index++;
			decode_state = DS_READ_DATA;
			continue;
		}
	}

	// Po zakończeniu przetwarzania danych sprawdź poprawność

	/*
	 * Jeśli zakończono wczytywać dane oczekując znaku do zdekodowania
	 * należy zwrócić błąd
	 */
	if(decode_state == DS_DECODE_NEXT_DATA) return DR_DATA_END_WHILE_DECODE;

	/*
	 * Jeśli licznik danych jest mniejszy od zadeklarowanej ilości
	 * oznacza to, że przesłano mniej danych niż deklarowano
	 * należy zwrócić błąd
	 */
	if(data_counter < output->data_length) return DR_DATA_LEN_TOO_SHORT;

	/*
	 * Nie zczytano wszystkich dostępnych danych
	 * oznacza to, że przesłano więcej danych niż deklarowano
	 * należy zwrócić błąd
	 */
	if(data_index != max_data_index) return DR_DATA_LEN_TOO_LONG;

	// Jeśli dane zostały wczytane bez błędu, można zdekodować CRC

	/*
	 * Jeśli CRC było niepoprawnie zakodowane
	 * należy zwrócić błąd.
	 */
	if(CP_hex_to_word(	frame_buffer[data_index],
						frame_buffer[data_index+1],
						frame_buffer[data_index+2],
						frame_buffer[data_index+3],
						&output->crc) == HEX_WRONG_CHAR) {
		return HEX_WRONG_CHAR;
	}

	// Poprawnie zdekodowano całą ramkę
	return DR_OK;
}

CP_RX_StatusCode_t	CP_2hex_to_byte(char high, char low, uint8_t* output) {
	high = toupper(high);
	low = toupper(low);

	if(!((high >= '0' && high <= '9') || (high >= 'A' && high <= 'F')) ||
			!((low >= '0' && low <= '9') || (low >= 'A' && low <= 'F'))) {
		return HEX_WRONG_CHAR;
	}

	*output = ((high >= 'A' ? high - 'A' + 10 : high - '0') << 4) |
			(low >= 'A' ? low - 'A' + 10 : low - '0');

	return HEX_OK;
}

CP_RX_StatusCode_t CP_hex_to_word(char high1, char low1, char high2, char low2, uint16_t* output) {
	uint8_t byte1;
	uint8_t byte2;

	if(CP_2hex_to_byte(high1, low1, &byte1) == HEX_WRONG_CHAR) return HEX_WRONG_CHAR;
	if(CP_2hex_to_byte(high2, low2, &byte2) == HEX_WRONG_CHAR) return HEX_WRONG_CHAR;

	*output = (byte1 << 8) | byte2;

	return HEX_OK;
}

CP_RX_StatusCode_t CP_validate_frame(CP_Frame_t* frame) {
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
