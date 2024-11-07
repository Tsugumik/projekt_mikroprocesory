/*
 * communication_protocol.h
 *
 *  Created on: Nov 3, 2024
 *      Author: Błażej Drozd
 */

#ifndef INC_COMMUNICATION_PROTOCOL_H_
#define INC_COMMUNICATION_PROTOCOL_H_

#include "ring_buffer.h"
#include "uart_handler.h"
#include "stdio.h"
#include "ctype.h"

#define CP_START_CHAR 		0x7B
#define CP_END_CHAR 		0x7D
#define CP_ENCODE_CHAR 		0x5C
#define CP_MAX_DATA_LEN		200
#define CP_MIN_DATA_LEN		12

typedef enum {
	WAIT_FOR_START,
	READ_SENDER_ID,
	READ_RECEIVER_ID,
	READ_DATA_LENGTH,
	READ_DATA,
	READ_CRC,
	WAIT_FOR_END
} FrameState;

typedef struct {
	uint8_t 	sender_id;
	uint8_t 	receiver_id;
	uint16_t 	data_length;
	uint8_t 	data[CP_MAX_DATA_LEN];
	uint32_t	crc;
} Frame_t;

void 		CP_encode_data(uint8_t*, uint8_t*, uint8_t, uint8_t*);
void 		CP_decode_data(uint8_t*, uint8_t*, uint8_t, uint8_t*);
void 		CP_receive_frame();
uint8_t 	CP_decode_received_frame(uint8_t*, uint8_t, Frame_t*);
uint8_t		CP_2hex_to_byte(char, char);
uint16_t	CP_hex_to_word(char, char, char, char);
uint8_t 	CP_validate_frame(Frame_t* frame);

/*
 * Funkcja do testowania działania UARTA
 * do użytku w pętli while w main
 */
void test_received_data();

#endif /* INC_COMMUNICATION_PROTOCOL_H_ */
