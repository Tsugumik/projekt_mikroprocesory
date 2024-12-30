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
#include "crc16_ansi.h"
#include "commands.h"
#include "stdlib.h"
#include "screen.h"

#define CP_START_CHAR 		0x7B
#define CP_END_CHAR 		0x7D
#define CP_ENCODE_CHAR 		0x2A
#define CP_MAX_DATA_LEN		200
#define CP_MIN_FRAME_LEN	12
#define CP_DATA_START_IDX	7
#define CP_STM_REC_ID		0xFF
#define CP_CRC_LEN			4
#define CP_ERROR_FRAME_LEN	3
#define CP_MAX_COMMAND_ARG	10
#define CP_ARG_START_CHAR	0x28
#define CP_ARG_END_CHAR		0x29
#define CP_ARG_SPLIT_CODE	0x2C
#define CP_ARG_SPLIT_CHAR	","

#define CP_START_CODE_CHAR	0x31
#define CP_END_CODE_CHAR	0x32
#define CP_ENCODE_CODE_CHAR	0x33

#define CP_MAX_CMD_LEN		20
#define CP_MAX_ARG_COUNT	20

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
	DR_OK,
	HEX_OK,
	V_OK,
	COMMAND_OK,
	DECODE_OK,
	DECODE_ERROR,
	DR_RECEIVER_DIFF,
	DR_WRONG_DATA_LEN_PROVIDED_IN_FRAME,
	DR_DATA_LEN_TOO_SHORT,
	DR_DATA_LEN_TOO_LONG,
	DR_WRONG_NEXT_CHARACTER,
	DR_DATA_END_WHILE_DECODE,
	DR_FRAME_TOO_SHORT,
	DR_RECEIVER_EQUAL_SENDER,
	HEX_WRONG_CHAR,
	V_CRC_ERROR,
	COMMAND_UNKNOWN,
	COMMAND_EMPTY,
	COMMAND_ARGUMENT_ERROR,
	COMMAND_PARSE_ERROR,
	COMMAND_ARGUMENT_TYPE_ERROR,
	COMMAND_NAME_OK,
	COMMAND_NAME_ERROR
} CP_StatusCode_t;

typedef enum {
	RFS_COMMAND_UNKNOWN,
	RFS_COMMAND_ARGUMENT_COUNT_INVALID
} CP_ReturnFrameStatus_t;

typedef enum {
	CP_DATA_OK,
	CP_STATUS_OK,
	CP_ERROR
} CP_ResponseType_t;

typedef enum {
	SEND_OK,
	GEN_OK,
} CP_TX_StatusCode_t;

typedef struct {
	uint8_t 	sender_id;
	uint8_t 	receiver_id;
	uint8_t 	data_length;
	uint8_t 	data[CP_MAX_DATA_LEN];
	uint16_t	crc;
} CP_Frame_t;

typedef struct {
	char name[CP_MAX_CMD_LEN + 1];
	char* arguments[CP_MAX_ARG_COUNT];
	uint8_t arg_count;
} CP_Command_t;

typedef enum {
	CP_FS_WAIT_FOR_START_CHAR,
	CP_FS_WAIT_FOR_END_CHAR,
	CP_FS_WAIT_FOR_SENDER_BYTE1,
	CP_FS_WAIT_FOR_SENDER_BYTE2,
	CP_FS_WAIT_FOR_RECEIVER_BYTE1,
	CP_FS_WAIT_FOR_RECEIVER_BYTE2,
	CP_FS_WAIT_FOR_DATALEN_BYTE1,
	CP_FS_WAIT_FOR_DATALEN_BYTE2,
	CP_FS_READ_DATA,
	CP_FS_WAIT_FOR_CRC_BYTE1,
	CP_FS_WAIT_FOR_CRC_BYTE2,
	CP_FS_WAIT_FOR_CRC_BYTE3,
	CP_FS_WAIT_FOR_CRC_BYTE4,
	CP_FS_VALIDATE_CRC
} CP_FrameStatus_t;

void 				CP_receive_frame();
CP_StatusCode_t 	CP_hex_to_byte(char, uint8_t*);
CP_StatusCode_t 	CP_hex_to_2bytes(char, uint16_t*);
CP_StatusCode_t 	CP_validate_frame(CP_Frame_t* frame);
CP_StatusCode_t		CP_decode_byte(uint8_t, uint8_t*);

void				CP_send_status_frame(uint8_t);
void				CP_send_error_frame(CP_StatusCode_t, uint8_t);
void				CP_send_data_frame(const char*, uint8_t);

CP_TX_StatusCode_t 	CP_send_frame(CP_Frame_t*);
CP_TX_StatusCode_t	CP_gen_frame(const char*, uint8_t, CP_Frame_t*);
void				CP_byte_to_2hex(uint8_t, uint8_t*);
void 				CP_word_to_hex(uint16_t, uint8_t*);

CP_StatusCode_t		CP_parse_command(CP_Frame_t*, CP_Command_t*);
void 				CP_CMD_execute(CP_Command_t*, uint8_t);

void				CP_Free_mem(CP_Command_t*);



/*
 * Funkcja do testowania działania UARTA
 * do użytku w pętli while w main
 */
void test_received_data();

#endif /* INC_COMMUNICATION_PROTOCOL_H_ */
