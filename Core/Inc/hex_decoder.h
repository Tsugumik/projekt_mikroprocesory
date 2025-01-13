/*
 * hex_decoder.h
 *
 *  Created on: Jan 13, 2025
 *      Author: drozd
 */

#ifndef INC_HEX_DECODER_H_
#define INC_HEX_DECODER_H_

#include "stdint.h"
#include "string.h"

typedef enum {
    HEXD_OK,
    HEXD_ERROR,
    HEXD_UNKNOWN_MODE,
    HEXD_OUT_OF_RANGE
} HexDecoder_Status_t;

typedef enum {
	HEXDM_8BIT,
	HEXDM_16BIT,
	HEXDM_32BIT
} HexDecoder_Mode_t;

HexDecoder_Status_t HEX_decode(char*, void*, uint32_t, uint32_t, HexDecoder_Mode_t);
HexDecoder_Status_t HEX_decode_char(char ch, uint8_t* out);


#endif /* INC_HEX_DECODER_H_ */
