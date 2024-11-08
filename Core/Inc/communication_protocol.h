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
#define CP_ENCODE_CHAR 		0x2A
#define CP_MAX_DATA_LEN		200
#define CP_MIN_FRAME_LEN	12
#define CP_DATA_START_IDX	7
#define CP_STM_REC_ID		0xFF
#define CP_CRC_LEN			4

#define CP_START_CODE_CHAR	0x31
#define CP_END_CODE_CHAR	0x32
#define CP_ENCODE_CODE_CHAR	0x33

typedef enum {
	WAIT_FOR_START,
	READ_SENDER_ID,
	READ_RECEIVER_ID,
	READ_DATA_LENGTH,
	READ_DATA,
	READ_CRC,
	WAIT_FOR_END
} FrameState;

typedef enum {
	DS_READ_DATA,
	DS_DECODE_NEXT_DATA
} CP_DecodeState_t;

typedef enum {
	DR_OK,
	DR_WRONG_DATA_LEN_PROVIDED_IN_FRAME,
	DR_DATA_LEN_TOO_SHORT,
	DR_DATA_LEN_TOO_LONG,
	DR_WRONG_NEXT_CHARACTER,
	DR_DATA_END_WHILE_DECODE,
	DR_FRAME_TOO_SHORT,
	DR_RECEIVER_EQUAL_SENDER,
	DR_RECEIVER_DIFF,
	HEX_WRONG_CHAR,
	HEX_OK,
	V_OK,
	V_CRC_ERROR
} CP_StatusCode_t;

typedef struct {
	uint8_t 	sender_id;
	uint8_t 	receiver_id;
	uint8_t 	data_length;
	uint8_t 	data[CP_MAX_DATA_LEN];
	uint16_t	crc;
} CP_Frame_t;

void 			CP_encode_data(uint8_t*, uint8_t*, uint8_t, uint8_t*);
void 			CP_decode_data(uint8_t*, uint8_t*, uint8_t, uint8_t*);
void 			CP_receive_frame();
CP_StatusCode_t CP_decode_received_frame(uint8_t*, uint8_t, CP_Frame_t*);
CP_StatusCode_t	CP_2hex_to_byte(char, char, uint8_t*);
CP_StatusCode_t	CP_hex_to_word(char, char, char, char, uint16_t*);
uint8_t 		CP_validate_frame(CP_Frame_t* frame);

/*
 * Funkcja do testowania działania UARTA
 * do użytku w pętli while w main
 */
void test_received_data();

#endif /* INC_COMMUNICATION_PROTOCOL_H_ */
