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
#include "hex_decoder.h"
#include "math.h"

#define CP_START_CHAR 		0x7B
#define CP_END_CHAR 		0x7D
#define CP_ENCODE_CHAR 		0x2A
#define CP_MAX_DATA_LEN		200
#define CP_MIN_FRAME_LEN	12
#define CP_DATA_START_IDX	7

// ID Odbiorcy STM
#define CP_STM_REC_ID		0xFF

#define CP_CRC_LEN			4
#define CP_ERROR_FRAME_LEN	3
#define CP_ARG_START_CHAR	0x28
#define CP_ARG_END_CHAR		0x29
#define CP_ARG_SPLIT_CODE	0x2C
#define CP_ARG_SPLIT_CHAR	","

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
	RFS_COMMAND_ARGUMENT_INVALID,
	RFS_COMMAND_ARGUMENT_COUNT_ERROR,
	RFS_INDEX_ERROR,
	RFS_BUFFER_EMPTY,
	RFS_CMD_PARSE_ERROR
} CP_ReturnFrameStatus_t;


typedef enum {
	SEND_OK,
	GEN_OK,
	GEN_ERROR,
	SEND_ERROR
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
	CP_FS_WAIT_FOR_SENDER_BYTE,
	CP_FS_WAIT_FOR_RECEIVER_BYTE,
	CP_FS_WAIT_FOR_DATALEN_BYTE,
	CP_FS_READ_DATA,
	CP_FS_WAIT_FOR_CRC_BYTE,
	CP_FS_VALIDATE_CRC
} CP_FrameStatus_t;

typedef enum {
	TEMPERATURE,
	HUMIDITY
} CP_ReturnSensorData_t;

void 				CP_receive_frame();
CP_StatusCode_t 	CP_validate_frame(CP_Frame_t* frame);

void				CP_send_status_frame(uint8_t);
void				CP_send_error_frame(CP_ReturnFrameStatus_t, uint8_t);
void				CP_send_data_frame(const char*, uint8_t);

CP_TX_StatusCode_t 	CP_send_frame(CP_Frame_t*);
CP_TX_StatusCode_t	CP_gen_frame(const char*, uint8_t, CP_Frame_t*);
void				CP_byte_to_2hex(uint8_t, uint8_t*);
void 				CP_word_to_hex(uint16_t, uint8_t*);

CP_StatusCode_t		CP_parse_command(CP_Frame_t*, CP_Command_t*);
void 				CP_CMD_execute(CP_Command_t*, uint8_t);

void				CP_Free_mem(CP_Command_t*);

CP_TX_StatusCode_t 	CP_createFrame_measurement_interval(uint8_t, CP_Frame_t*);
CP_TX_StatusCode_t	CP_createFrame_latest_sensor_data(uint8_t, CP_ReturnSensorData_t, CP_Frame_t*);
CP_TX_StatusCode_t	CP_createFrame_archive_sensor_data(uint8_t, CP_ReturnSensorData_t, uint16_t, CP_Frame_t*);
CP_TX_StatusCode_t	CP_createFrame_oldest_index(uint8_t, CP_Frame_t*);
CP_TX_StatusCode_t	CP_createFrame_latest_index(uint8_t, CP_Frame_t*);
CP_TX_StatusCode_t 	CP_createFrame_returnTempUnit(uint8_t, CP_Frame_t*);


void				CP_breakFloat(float num, uint16_t* integer, uint8_t* fractional, uint8_t* isNegative);




/*
 * Funkcja do testowania działania UARTA
 * do użytku w pętli while w main
 */
void test_received_data();

#endif /* INC_COMMUNICATION_PROTOCOL_H_ */
