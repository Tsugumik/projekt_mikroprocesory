/*
 * crc16_ansi.c
 *
 *  Created on: Nov 7, 2024
 *      Author: drozd
 */
#include "crc16_ansi.h"

uint16_t crc16_ansi(uint8_t* data, uint32_t length) {
	uint16_t crc = CRC_INIT;

	for(uint32_t i=0; i < length; i++) {
		crc ^= (uint16_t)data[i];

		for(uint8_t j = 0; j < 8; j++) {
			if(crc & 0x0001) {
				crc = (crc >> 1) ^ CRC_POLY;
			} else {
				crc >>= 1;
			}
		}
	}

	return crc;
}

void crc16_toHexString(uint16_t crc, char* output) {
	sprintf(output, "%04X", crc);
}
