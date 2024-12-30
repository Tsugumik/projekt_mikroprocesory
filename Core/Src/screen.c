/*
 * screen.c
 *
 *  Created on: Dec 30, 2024
 *      Author: Błażej Drozd
 */

#include "screen.h"

static SCREEN_Status_t screenStatus = SCREEN_ON;
SCREEN_TempUnits_t tempUnit = SCREEN_TempUnit_C;

static float tempCBuffer = 0;
static float humBuffer = 0;

void SCREEN_CalculateValues(uint32_t* temp20bit, uint32_t* humidity20bit) {
	if(screenStatus == SCREEN_OFF) return;

	tempCBuffer = (*temp20bit / 1048576.0)*200.0 - 50.0;
	humBuffer = *humidity20bit / 10485.76;

	SCREEN_Update();
}

void SCREEN_SetStatus(SCREEN_Status_t status) {
	switch(status) {
		case SCREEN_OFF:
			screenStatus = SCREEN_OFF;
			ssd1306_SetDisplayOn(0);
			break;
		case SCREEN_ON:
			screenStatus = SCREEN_ON;
			SCREEN_Update();
			ssd1306_SetDisplayOn(1);
			break;
	}
}

void SCREEN_Update() {
	char tempStrBuffer[15];
	char humStrBuffer[15];

	float calcTemp;

	switch(tempUnit) {
		case SCREEN_TempUnit_C:
			sprintf(tempStrBuffer, "T: %.2f C", tempCBuffer);
			break;
		case SCREEN_TempUnit_F:
			calcTemp = tempCBuffer * 9.0 / 5.0 + 32.0;
			sprintf(tempStrBuffer, "T: %.2f F", calcTemp);
			break;
		case SCREEN_TempUnit_K:
			calcTemp = tempCBuffer + 273.15;
			sprintf(tempStrBuffer, "T: %.2f K", calcTemp);
			break;
		default:
			sprintf(tempStrBuffer, "T: %.2f C", tempCBuffer);
			break;
	}

	sprintf(humStrBuffer, "W: %.2f %%", humBuffer);

	ssd1306_Fill(Black);
	ssd1306_SetCursor(5, 5);
	ssd1306_WriteString(tempStrBuffer, Font_11x18, White);
	ssd1306_SetCursor(5, 30);
	ssd1306_WriteString(humStrBuffer, Font_11x18, White);
	ssd1306_UpdateScreen();
}
