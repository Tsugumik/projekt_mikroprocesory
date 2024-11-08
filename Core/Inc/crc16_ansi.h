/*
 * crc16_ansi.h
 *
 *  Created on: Nov 7, 2024
 *      Author: drozd
 */

#ifndef INC_CRC16_ANSI_H_
#define INC_CRC16_ANSI_H_

#include "stdint.h"

#define CRC_POLY 0x8005
#define CRC_INIT 0x0000

uint16_t 	crc16_ansi(uint8_t*, uint32_t);
void		crc16_toHexString(uint16_t, char*);


#endif /* INC_CRC16_ANSI_H_ */
