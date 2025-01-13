/*
 * hex_decoder.c
 *
 *  Created on: Jan 13, 2025
 *      Author: drozd
 */


#include "hex_decoder.h"

HexDecoder_Status_t HEX_decode(char* input, void* output, uint32_t validate_min, uint32_t validate_max, HexDecoder_Mode_t mode) {
    uint8_t max_chars;
    uint32_t result = 0;
    uint8_t temp;
    size_t hex_len = strlen(input);

    switch (mode) {
        case HEXDM_8BIT:
            max_chars = 2;
            break;
        case HEXDM_16BIT:
            max_chars = 4;
            break;
        case HEXDM_32BIT:
            max_chars = 8;
            break;
        default:
            return HEXD_UNKNOWN_MODE;
    }

    if (hex_len > max_chars) return HEXD_ERROR;
    if (hex_len == 0) return HEXD_ERROR;

    for (size_t i = 0; i < hex_len; i++) {
        if (HEX_decode_char(input[i], &temp) != HEXD_OK) {
            return HEXD_ERROR;
        }
        result = (result << 4) | temp;
    }

    if (result < validate_min || result > validate_max) {
        return HEXD_OUT_OF_RANGE;
    }

    switch (mode) {
        case HEXDM_8BIT:
            *(uint8_t*)output = (uint8_t)result;
            break;
        case HEXDM_16BIT:
            *(uint16_t*)output = (uint16_t)result;
            break;
        case HEXDM_32BIT:
            *(uint32_t*)output = result;
            break;
        default:
            return HEXD_UNKNOWN_MODE;
    }

    return HEXD_OK;
}

HexDecoder_Status_t HEX_decode_char(char ch, uint8_t* out) {
	if(!((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F'))) {
		return HEXD_ERROR;
	}

	*out = (ch >= 'A' ? ch - 'A' + 10 : ch - '0');

	return HEXD_OK;
}
