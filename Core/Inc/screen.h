/*
 * screen.h
 *
 *  Created on: Dec 30, 2024
 *      Author: Błażej Drozd
 */

#ifndef INC_SCREEN_H_
#define INC_SCREEN_H_

#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "stdio.h"

typedef enum {
	SCREEN_TempUnit_C, SCREEN_TempUnit_F, SCREEN_TempUnit_K, SCREEN_Unit_Percent
} SCREEN_Units_t;

typedef enum {
	SCREEN_OFF, SCREEN_ON
} SCREEN_Status_t;

void SCREEN_CalculateValues(uint32_t* temp20bit, uint32_t* humidity20bit);
float SCREEN_CalculateTemp(uint32_t*);
float SCREEN_CalculateHumidity(uint32_t*);
void SCREEN_SetStatus(SCREEN_Status_t status);
void SCREEN_Update();
float SCREEN_ConvertTemp(float, SCREEN_Units_t);

#endif /* INC_SCREEN_H_ */
