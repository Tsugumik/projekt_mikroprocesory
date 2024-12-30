/*
 * screen.c
 *
 *  Created on: Dec 30, 2024
 *      Author: Błażej Drozd
 */

#include "screen.h"

static SCREEN_Status_t screenStatus = SCREEN_ON;
SCREEN_TempUnits_t tempUnit = SCREEN_TempUnit_C;

void SCREEN_DisplayTempAndHumidity(uint32_t* temp20bit, uint32_t* humidity20bit) {
	if(screenStatus == SCREEN_OFF) return;

	char tempStrBuffer[15];
	char humStrBuffer[15];

	float temperature;

	temperature = (*temp20bit / 1048576.0)*200.0 - 50.0;

	switch(tempUnit) {
		case SCREEN_TempUnit_C:
			sprintf(tempStrBuffer, "T: %.2f C", temperature);
			break;
		case SCREEN_TempUnit_F:
			temperature = temperature * 9.0 / 5.0 + 32.0;
			sprintf(tempStrBuffer, "T: %.2f F", temperature);
			break;
		case SCREEN_TempUnit_K:
			temperature = temperature + 273.15;
			sprintf(tempStrBuffer, "T: %.2f K", temperature);
			break;
		default:
			sprintf(tempStrBuffer, "T: %.2f C", temperature);
			break;
	}

	float humidity = *humidity20bit / 10485.76;
	sprintf(humStrBuffer, "W: %.2f %%", humidity);

	ssd1306_Fill(Black);
	ssd1306_SetCursor(5, 5);
	ssd1306_WriteString(tempStrBuffer, Font_11x18, White);
	ssd1306_SetCursor(5, 30);
	ssd1306_WriteString(humStrBuffer, Font_11x18, White);
	ssd1306_UpdateScreen();

}

void SCREEN_SetStatus(SCREEN_Status_t status) {
	switch(status) {
		case SCREEN_OFF:
			screenStatus = SCREEN_OFF;
			ssd1306_SetDisplayOn(0);
			break;
		case SCREEN_ON:
			screenStatus = SCREEN_ON;
			ssd1306_SetDisplayOn(1);
			break;
	}
}
